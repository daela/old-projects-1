
#include "df_proximity_message.hpp"

std::string df_proximity_message::StripColor(const std::string&strMessage)
{
	std::string::size_type pos=0 - 1,pos2;
	std::string strOutput;
	do
	{
		++pos;
		pos2=strMessage.find("^C",pos);
		if (pos2 == std::string::npos) pos2=strMessage.size();

		strOutput.append(strMessage.begin()+pos,strMessage.begin()+pos2);

		pos=strMessage.find('^',pos2+2);
	} while (pos != std::string::npos);
	return strOutput;
}

bool df_proximity_message::Parse(const std::string&strColoredMessage)
{
	std::string::size_type uStatePos=std::string::npos;
	std::string::size_type uStateSize=0;
	std::string::size_type uPos;

	const std::string strMessage=StripColor(strColoredMessage);
	const std::string ptStateStrings[4] = { "has left our city of", "left our city of", "has been spotted in our city of", "entered our city of" };
	// find the position of the state string
	for (unsigned int i=0;i<4;++i)
	{
		uPos=strMessage.rfind(ptStateStrings[i]);
		// if we haven't found a string, or our position is later in the string than the one we found AND handle special case of "has left" vs "left"
		if (uPos != std::string::npos && (uStatePos == std::string::npos || uPos > uStatePos))
		{
			uStatePos=uPos;
			uStateSize=ptStateStrings[i].size();
			// if we found it for the first one, skip the second
			if (i==0) ++i;
			
			// if it's message 1, it's a leave message (as index 0 is made into 1, too), else it's an enter message
			isEnter=!(i == 1);
		}	
	}
	if (uStatePos == std::string::npos || uStatePos < 2) return false;

	// find the trailing period
	uPos = strMessage.rfind('.');
	if (uPos == std::string::npos) return false;

	// grab the city from between
	strHolding = strMessage.substr(uStatePos+uStateSize+1,uPos-uStatePos-uStateSize-1);

	// now, find the rank text, if present
	uPos=strMessage.find(' ');
	if (uPos == std::string::npos) return false;
	
	objPlayer.strRank = strMessage.substr(0,uPos);

	const std::string ptRankStrings[10] = { "Recruit", "Private", "Corporal", "Sergeant", "Lieutenant", "Captain", "Major", "Colonel", "General", "SupremeGeneral" }; 
	bool bFound=false;
	for (unsigned int i=0;i<10;++i)
	{
		if (ptRankStrings[i] == objPlayer.strRank)
		{
			bFound=true;
			break;
		}
	}
	// if the first word isn't a rank, then this person is clanless for sure
	if (!bFound)
	{
		objPlayer.strRank="";
		objPlayer.strClan="";
		uPos=uStatePos-1;
	}
	else
	{
		// find the clan string
		const std::string ptClanStrings[2] = { "from the Clan of the", "from our clan the" };
		std::string::size_type uClanDivPos=std::string::npos;
		std::string::size_type uClanDivSize=0;
		for (unsigned int i=0;i<2;++i)
		{
			uPos=strMessage.rfind(ptClanStrings[i]);
			// if we haven't found a string, or our position is later in the string than the one we found AND handle special case of "has left" vs "left"
			if (uPos != std::string::npos && (uClanDivPos == std::string::npos || uPos > uClanDivPos))
			{
				uClanDivPos=uPos;
				uClanDivSize=ptClanStrings[i].size();
			}	
		}
		if (uClanDivPos == std::string::npos || uClanDivPos < 2) return false;
		objPlayer.strClan=strMessage.substr(uClanDivPos+uClanDivSize+1,uStatePos-uClanDivPos-uClanDivSize-2);
		uPos=uClanDivPos-1;
	}
	
	// reusing uStatePos for start of name position
	if (objPlayer.strRank.empty())
		uStatePos=0;
	else
		uStatePos=objPlayer.strRank.size()+1;

	// if clan text was present, remove the rank from the name
	objPlayer.strName = strMessage.substr(uStatePos,uPos-uStatePos);
		

	// names can't have commas; this prevents "A Mount, ridden by" and any other potential messages of that sort.

	if (objPlayer.strName.find(',') != std::string::npos) return false;
	string_to_lower(objPlayer.strName);
	string_to_lower(objPlayer.strClan);
	string_to_lower(objPlayer.strRank);
	string_to_lower(strHolding);

	return true;
}

std::ostream&operator<<(std::ostream&os,const df_proximity_message&rhs)
{
	os
		<< "Name:\t" << rhs.objPlayer.strName << std::endl
		<< "Rank:\t" << rhs.objPlayer.strRank << std::endl
		<< "Clan:\t" << rhs.objPlayer.strClan << std::endl
		<< "Enter:\t" << rhs.isEnter << std::endl
		<< "City:\t" << rhs.strHolding;
	return os;
}

