#include "df_holding.hpp"
#include <algorithm>
#include <cctype>

std::string df_holding_notification::GetMessageString() const
{
	std::stringstream ss;
	ss << strHolding; 

	// WARNING: HoldingName has 4 enemies (from ClanName|, mostly ClanName)
	// WARNING: HoldingName has enemy Joe Blow from ClanName

	if (uCount == 0)
	{
		ss << " is clear";
	}
	else
	{
		ss << " has ";
		if (uCount == 1)
		{
			if (!strOnlyPerson.empty())
				ss << "enemy " << strOnlyPerson << " ";
			else
				ss << "1 enemy ";
		}
		else
		{
			ss << uCount << " enemies " << (bOtherClans?"mostly ": "");
		}
		ss << "from " << (strPrimaryClan.empty()?"unclanned":strPrimaryClan);
	}
	return ss.str();	
}
void df_holding_notification::Clear(const std::string&strNewHolding)
{
	if (!strNewHolding.empty())
		strHolding=strNewHolding;
	uCount=0;
	strPrimaryClan=strOnlyPerson="";
	bOtherClans=false;
}

df_holding::df_holding(const std::string&strHoldingName)
{
	Clear(strHoldingName);
}
void df_holding::Clear(const std::string&strNewHolding)
{
	if (!strNewHolding.empty())
		strName=strNewHolding;
	mpPresent.clear();
	ClearUnique();
}

void df_holding::ClearUnique()
{
	mpUnique.clear();
}
bool df_holding::ProcessMessage(const df_proximity_message&objMsg)
{
	if (objMsg.strHolding != strName) return false;
	player_map::iterator itPresent = mpPresent.find(objMsg.objPlayer.strName);

	// if he's already in system, this is either an exit (and he needs to go), or we somehow
	// got an entrance from someone we think is already inside.. and we need to make him "exit" anyway.
	if (itPresent != mpPresent.end())
	{
		// remove the player
		mpPresent.erase(itPresent);
	}
	if (objMsg.isEnter)
	{
		// store the player and his object
		mpPresent.insert(player_map::value_type(objMsg.objPlayer.strName,objMsg.objPlayer));

	}
	// store this player's updated information in the unique list, even if it's an exit
	mpUnique[objMsg.objPlayer.strName] = objMsg.objPlayer;
	return true;
}
void df_holding::GetNotification(
	df_holding_notification&objNotify,
	const df_holding::string_set_ptr_list&lstIgnoreClanSets,
	const df_holding::string_set_ptr_list&lstIgnorePlayerSets,
	df_holding::player_map*ptFilteredMap,
	const bool getCurrent,
	const bool getInverse) const
{
	objNotify.Clear(strName);

	// Which map do we car about?
	const df_holding::player_map & mpPlayers = (getCurrent?mpPresent:mpUnique);

	typedef std::map<std::string,unsigned int> clan_map;
	clan_map mpClanCount;
	bool bSkip;
	string_set_ptr_list::const_iterator itSetList;

	// clear the filtered destination map, if requested
	if (ptFilteredMap) ptFilteredMap->clear();

	for (player_map::const_iterator it=mpPlayers.begin();it!=mpPlayers.end();++it)
	{
		bSkip=getInverse; // naturally, this is false

		// see if this clan is in one of the lists that we need to ignore
		for (itSetList=lstIgnoreClanSets.begin();itSetList!=lstIgnoreClanSets.end();++itSetList)
		{
			const string_set*ptSet=*itSetList;
			if (ptSet->find(it->second.strClan) != ptSet->end())
			{
				bSkip=!getInverse;
				break;
			}
		}
		if (bSkip) continue;

		// see if this player is in one of the lists we need to ignore
		for (itSetList=lstIgnorePlayerSets.begin();itSetList!=lstIgnorePlayerSets.end();++itSetList)
		{
			const string_set*ptSet=*itSetList;
			if (ptSet->find(it->second.strName) != ptSet->end())
			{
				bSkip=!getInverse;
				break;
			}
		}
		if (bSkip) continue;
		// if we got here, we aren't ignoring this person

		// Add this person to the filtered destination map, if requested
		if (ptFilteredMap)
			ptFilteredMap->insert(*it);

		// if this is the first character we've processed, set him as the only person
		if (objNotify.uCount==0)
			objNotify.strOnlyPerson=it->second.strName;
		// increment the total enemy count 
		++objNotify.uCount;
		// retrieve the clan's count, initializing it to 0 if it doesn't exist
		clan_map::iterator itCount=mpClanCount.insert(clan_map::value_type(it->second.strClan,0)).first;
		// increment the count by 1
		++itCount->second;
	}
	
	if (mpClanCount.size()>1)
		objNotify.bOtherClans=true;

	clan_map::const_iterator itMax=mpClanCount.begin();
	for (clan_map::const_iterator it=itMax;it!=mpClanCount.end();++it)
	{
		if (it->second > itMax->second)
			itMax=it;
	}
	if (itMax != mpClanCount.end())
		objNotify.strPrimaryClan=itMax->first;
	string_to_lower(objNotify.strPrimaryClan);
	string_to_lower(objNotify.strOnlyPerson);
}

