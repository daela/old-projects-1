#include "df_proximity_monitor.hpp"

const char*df_proximity_monitor::ptListString[6] = { "admin", "allyclan", "allyplayer", "ignoreclan", "ignoreplayer", "ignoreholding" };

df_proximity_monitor::df_proximity_monitor()
: strSessionFile(".session.xml"), bSeenChanged(false)
{ }

bool df_proximity_monitor::Connect()
{
	uLastTime=Timestamp();
	if (objWeb.PrepareSession(strSessionFile,strUser,strPassHash))
	{
		// only clearing the holdings list when a new connection takes place, so we can get information during server down
		mpHoldings.clear();
		strClearUniqueTime=get_current_timestamp_string();
		return true;
	}
	return false;
}
void df_proximity_monitor::ClearUniques()
{
	for (holding_map::iterator it=mpHoldings.begin();it != mpHoldings.end();++it)
	{
		it->second.ClearUnique();
		strClearUniqueTime=get_current_timestamp_string();
	}

}
std::string df_proximity_monitor::GetClearUniqueTime() const
{
	return strClearUniqueTime;
}


void df_proximity_monitor::ReadNodeToSet(df_proximity_monitor::E_LISTWHICH eWhich, TiXmlElement*ptParent)
{
	TiXmlElement*ptElement;
	// bail if no parent
	if (!ptParent)
		return;
	// find the node that holds this list of entries, and bail if it isn't there
	ptElement=XML_GetElement(ptParent,ptListString[eWhich]);
	if (!ptElement)
		return;
	// Get the set for this entry
	string_set&stList=mpSets[eWhich];
	// for each child of the node, add it to the list
	for( TiXmlElement*child = ptElement->FirstChildElement(); child; child = child->NextSiblingElement() )
		stList.insert(string_to_lower_ret(NullFix(child->GetText())));
}
void df_proximity_monitor::AddSetToNode(df_proximity_monitor::E_LISTWHICH eWhich, TiXmlElement*ptParent)
{
	if (!ptParent) return;


	// Create the section element
	TiXmlElement*ptSection=new TiXmlElement(ptListString[eWhich]);
	TiXmlElement*ptElement;
	// Get the set for this entry
	const string_set&stList=mpSets[eWhich];

	// Add an xml entry for each entry of the list
	for (string_set::const_iterator it=stList.begin();it!=stList.end();++it)
	{
		ptElement=new TiXmlElement("name");
		ptElement->LinkEndChild(new TiXmlText(it->c_str()));
		ptSection->LinkEndChild(ptElement);
	}
	ptParent->LinkEndChild(ptSection);
}
bool df_proximity_monitor::LoadConfig(const std::string&strFilename)
{
	TiXmlDocument objDoc;
	TiXmlElement *ptElement;

	bool doSaveBack=false;
	mpSets.clear();

	if (objDoc.LoadFile(strFilename))
	{
		ptElement=XML_GetElement(objDoc.RootElement(),"settings.weburl");
		if (!ptElement)
		{
			#ifdef DEBUG
			std::cerr << "ERROR config file must have a web url" << std::endl;
			#endif
			return false;
		}
		strWebURL=NullFix(ptElement->GetText());

		ptElement=XML_GetElement(objDoc.RootElement(),"settings.user");
		if (!ptElement)
		{
			#ifdef DEBUG
			std::cerr << "ERROR config file must have a user" << std::endl;
			#endif
			return false;
		}
		strUser=NullFix(ptElement->GetText());

		ptElement=XML_GetElement(objDoc.RootElement(),"settings.passwordsha1");
		if (!ptElement)
		{
			ptElement=XML_GetElement(objDoc.RootElement(),"settings.password");
			// if there's no normal password, or it's empty
			if (!ptElement|| (strPassHash=NullFix(ptElement->GetText())).empty())
			{
				#ifdef DEBUG
				std::cerr << "ERROR config file must have a password or a sha1 hash of password" << std::endl;
				#endif
				return false;
			}
			strPassHash=df_web::sha1sum(strPassHash);
			doSaveBack=true;
		}
		else
			strPassHash=string_to_upper_ret(NullFix(ptElement->GetText()));

		ptElement=XML_GetElement(objDoc.RootElement(),"settings.sessionfile");
		if (!ptElement)
		{
			#ifdef DEBUG
			std::cerr << "ERROR config file must provide a name for the session file" << std::endl;
			#endif
			return false;
		}
		strSessionFile=NullFix(ptElement->GetText());

		ptElement=XML_GetElement(objDoc.RootElement(),"settings.seenfile");
		if (ptElement)
			strSeenFile=NullFix(ptElement->GetText());
		else
			strSeenFile="";

		ptElement=XML_GetElement(objDoc.RootElement(),"settings.bankautoclosedelay");

		if (ptElement)
		{
			std::stringstream ss;
			ss << NullFix(ptElement->GetText());
			if (!(ss >> uBankAutoCloseDelay))
				uBankAutoCloseDelay=0;	
		}
		else
			uBankAutoCloseDelay=0;

		// Read in our lists
		ReadNodeToSet(LST_ADMIN,		objDoc.RootElement());
		ReadNodeToSet(LST_ALLYCLAN,		objDoc.RootElement());
		ReadNodeToSet(LST_ALLYPLAYER,		objDoc.RootElement());
		ReadNodeToSet(LST_IGNORECLAN,		objDoc.RootElement());
		ReadNodeToSet(LST_IGNOREPLAYER,		objDoc.RootElement());
		ReadNodeToSet(LST_IGNOREHOLDING,	objDoc.RootElement());

		// Update the Base URL for the bot (YOU ADD THIS)
		objWeb.SetBaseURL(strWebURL);

		// if we converted the password to a sha1sum, lets save the file back out to remove the raw password from the file.
		if (doSaveBack)
			SaveConfig(strFilename);
		return true;
	}
	return false;
}
bool df_proximity_monitor::SaveHoldingPlayers(const std::string&strFilename,const std::string&strStartTime,const df_proximity_monitor::holding_player_map&mpHoldingPlayers)
{
	TiXmlDocument objDoc;
	
	TiXmlElement* ptRoot = new TiXmlElement("rootnode");

	// write out a timestamp to the rootnode
	ptRoot->SetAttribute("begin",strStartTime);
	ptRoot->SetAttribute("end",get_current_timestamp_string());

	TiXmlElement* ptHoldingsSection=new TiXmlElement("holdings");
	unsigned int uAllCount=0;
	unsigned int uHoldingCount=0;
	for (holding_player_map::const_iterator it=mpHoldingPlayers.begin();it != mpHoldingPlayers.end();++it)
	{
		// skip empty holdings
		if (it->second.empty()) continue;
		++uHoldingCount;

		TiXmlElement* ptHolding=new TiXmlElement("holding");
		ptHolding->SetAttribute("name",it->first);
		ptHolding->SetAttribute("count",it->second.size());

		// add to the global size
		uAllCount += it->second.size();

		// elsewhere, this is used for holding name => (map of player name => player object)
		// here, it's used for clan name => ( same ) 
		holding_player_map mpClanPlayers;
		for (player_map::const_iterator itPlayer=it->second.begin();itPlayer!=it->second.end();++itPlayer)
		{
			mpClanPlayers[itPlayer->second.strClan].insert(*itPlayer);
		}
		// Now we have the information by clan, and sorted (As we want)... so start appending!
		for (holding_player_map::const_iterator itClan=mpClanPlayers.begin();itClan!=mpClanPlayers.end();++itClan)
		{
			TiXmlElement* ptClan=new TiXmlElement("clan");
			ptClan->SetAttribute("name",itClan->first);
			ptClan->SetAttribute("count",itClan->second.size());
			for (player_map::const_iterator itPlayer = itClan->second.begin();itPlayer!=itClan->second.end();++itPlayer)
			{
				TiXmlElement* ptPlayer=new TiXmlElement("player");
				ptPlayer->SetAttribute("name",itPlayer->second.strName);
				ptPlayer->SetAttribute("rank",itPlayer->second.strRank);
				ptClan->LinkEndChild(ptPlayer);
			}
			ptHolding->LinkEndChild(ptClan);
		}
		ptHoldingsSection->LinkEndChild(ptHolding);
	}
	ptHoldingsSection->SetAttribute("count",uHoldingCount);
	ptHoldingsSection->SetAttribute("player_count",uAllCount);
	ptRoot->LinkEndChild(ptHoldingsSection);

	// Add it all to the document
	objDoc.LinkEndChild(new TiXmlDeclaration("1.0","",""));
	objDoc.LinkEndChild(ptRoot);
	
	return objDoc.SaveFile(strFilename.c_str());
}
void df_proximity_monitor::SaveListXml(TiXmlElement*ptRoot)
{
	// Add our list sections
	AddSetToNode(LST_ADMIN,ptRoot);
	AddSetToNode(LST_ALLYCLAN,ptRoot);
	AddSetToNode(LST_ALLYPLAYER,ptRoot);
	AddSetToNode(LST_IGNORECLAN,ptRoot);
	AddSetToNode(LST_IGNOREPLAYER,ptRoot);
	AddSetToNode(LST_IGNOREHOLDING,ptRoot);
}
bool df_proximity_monitor::SaveList(const std::string&strFilename)
{
	TiXmlDocument objDoc;
	TiXmlElement* ptRoot = new TiXmlElement("rootnode");

	// Add our list sections
	SaveListXml(ptRoot);

	// Add it all to the document
	objDoc.LinkEndChild(new TiXmlDeclaration("1.0","",""));
	objDoc.LinkEndChild(ptRoot);
	
	return objDoc.SaveFile(strFilename.c_str());
}
	
bool df_proximity_monitor::SaveConfig(const std::string&strFilename)
{
	TiXmlDocument objDoc;
	TiXmlElement * ptSection, *ptElement, *ptRoot;
	
	ptRoot = new TiXmlElement("rootnode");

	// Settings Section
	ptSection=new TiXmlElement("settings");

	ptElement=new TiXmlElement("weburl");
	ptElement->LinkEndChild(new TiXmlText(strWebURL.c_str()));
	ptSection->LinkEndChild(ptElement);

	ptElement=new TiXmlElement("user");
	ptElement->LinkEndChild(new TiXmlText(strUser.c_str()));
	ptSection->LinkEndChild(ptElement);

	ptElement=new TiXmlElement("passwordsha1");
	ptElement->LinkEndChild(new TiXmlText(strPassHash.c_str()));
	ptSection->LinkEndChild(ptElement);

	ptElement=new TiXmlElement("sessionfile");
	ptElement->LinkEndChild(new TiXmlText(strSessionFile.c_str()));
	ptSection->LinkEndChild(ptElement);

	ptElement=new TiXmlElement("seenfile");
	ptElement->LinkEndChild(new TiXmlText(strSeenFile.c_str()));
	ptSection->LinkEndChild(ptElement);

	ptElement=new TiXmlElement("bankautoclosedelay");
	{
		std::string strDelay;
		std::stringstream ss;
		ss << uBankAutoCloseDelay;
		ss >> strDelay;
		ptElement->LinkEndChild(new TiXmlText(strDelay.c_str()));
	}
	ptSection->LinkEndChild(ptElement);

	ptRoot->LinkEndChild(ptSection);

	// Add our list sections
	SaveListXml(ptRoot);

	// Add it all to the document
	objDoc.LinkEndChild(new TiXmlDeclaration("1.0","",""));
	objDoc.LinkEndChild(ptRoot);
	
	return objDoc.SaveFile(strFilename.c_str());
}
bool df_proximity_monitor::IsAdmin(const std::string&strName)
{
	string_set&stAdmins=mpSets[LST_ADMIN];
	return (stAdmins.find(string_to_lower_ret(strName))!=stAdmins.end());
}
// if you fail to retrieve every page of a multiple-page update, we use whatever data we've gotten so far
// but we don't update our "last time checked" value, so subsequent checks will continue to backtrack.
// this means we use whatever information we've managed to obtain so far, but keep trying to backtrack.

bool df_proximity_monitor::Check()
{
		unsigned int usedrows=0;
		unsigned int totalrows=20;
		unsigned int page=1;
		std::string strPage;
		TiXmlElement*ptElement;
		bool gotOneTime=false;
		Timestamp uFirstTime;

		// if we've already gotten a message for this person in a particular holding, ignore subsequent messages for that player in that holding
		typedef std::set<std::pair<std::string,std::string> > handled_set;

		handled_set stHandledPlayerForHolding;
		handled_set::value_type objPlayerForHolding;

		TiXmlDocument objDoc;

		df_holding objTempHolding;
		do
		{
			unsigned int pagerows=totalrows - usedrows;
			if (pagerows > 20) pagerows=20;

			if (!objWeb.GetProximityPage(usedrows+1,pagerows,page,strPage,objDoc))
			{
				if (strPage.empty())
				{
					#ifdef DEBUG
					std::cerr << "ERROR empty proximity page received, but we didn't get invalid data.." << std::endl;
					#endif
					return true;
				}
				#ifdef DEBUG
				std::cerr << "ERROR retrieving proximity page" << std::endl << strPage << std::endl;
				#endif

				return false;
			}
			

			ptElement=XML_GetElement(objDoc.RootElement(),"ObjectData.GridTotalRows");
			if (ptElement == NULL)
			{
				#ifdef DEBUG
				std::cerr << "ERROR retrieving total rows" << std::endl;
				#endif
				return false;
			}
			totalrows = atoi(NullFix(ptElement->GetText()).c_str());

			ptElement=XML_GetElement(objDoc.RootElement(),"ObjectData.Events");
			if (ptElement == NULL)
			{
				#ifdef DEBUG
				std::cerr << "ERROR finding Events tag" << std::endl;
				#endif
				return false;
			}
			// if we haven't gotten any timestamp before, just get the first entry's timestamp
			// otherwise, keep going until we find a timestamp < the one we have.
			for( TiXmlElement*child = ptElement->FirstChildElement(); child; child = child->NextSiblingElement() )
			{
				TiXmlElement*ptEventText=XML_GetElement(child,"Text");
				TiXmlElement*ptEventTime=XML_GetElement(child,"TimeData");
				if (ptEventText && ptEventTime)
				{
					Timestamp uTime; uTime.Load(NullFix(ptEventTime->GetText()));
					if (!gotOneTime)
					{
						uFirstTime=uTime;
						gotOneTime=true;
					}
					if (uLastTime==Timestamp())
					{
						uLastTime = uTime;
						return true;
					}
					if (uTime < uLastTime) 
					{
						// exit completely
						uLastTime=uFirstTime;
						return true;
					}
					df_proximity_message objMsg;
					if (objMsg.Parse(NullFix(ptEventText->GetText())))
					{
						// if we haven't processed this person yet this update (as we only want the latest state)
						objPlayerForHolding.first=objMsg.objPlayer.strName;
						objPlayerForHolding.second=objMsg.strHolding;
						// if an insertion was made (meaning, if the name/holding pair wasn't already in the list
						if (stHandledPlayerForHolding.insert(objPlayerForHolding).second)
						{
							// get the holding object and process the message
							objTempHolding.strName=objMsg.strHolding;
							std::map<std::string,df_holding>::iterator it=mpHoldings.insert(std::map<std::string,df_holding>::value_type(objMsg.strHolding,objTempHolding)).first;
		
							it->second.ProcessMessage(objMsg);
							if (ProcessLastSeen(uTime,objMsg))
								bSeenChanged=true;
							//std::cout << "Message: " << objMsg << std::endl;
						}
					}
				}
			}
			++page;
			usedrows += 20;
		} while (usedrows < totalrows);
		if (gotOneTime)
		{
			uLastTime=uFirstTime;
		}
		return true;


}
void df_proximity_monitor::GetNotifications(df_proximity_monitor::notify_map&mpNotify,df_proximity_monitor::holding_player_map*ptHoldingPlayers,const bool getCurrent,const bool getInverse) const
{
	mpNotify.clear();

	df_holding_notification objNotify;

	set_map::const_iterator itMap;
	string_set_ptr_list lstIgnoreClans;
	if ((itMap=mpSets.find(LST_ALLYCLAN))   != mpSets.end()) lstIgnoreClans.push_back(&(itMap->second));
	if ((itMap=mpSets.find(LST_IGNORECLAN)) != mpSets.end()) lstIgnoreClans.push_back(&(itMap->second));

	string_set_ptr_list lstIgnorePlayers;
	if ((itMap=mpSets.find(LST_ALLYPLAYER))   != mpSets.end()) lstIgnorePlayers.push_back(&(itMap->second));
	if ((itMap=mpSets.find(LST_IGNOREPLAYER)) != mpSets.end()) lstIgnorePlayers.push_back(&(itMap->second));

	string_set emptySet;
	const string_set*ptIgnoreHoldings = NULL;
	if ((itMap=mpSets.find(LST_IGNOREHOLDING)) != mpSets.end()) ptIgnoreHoldings = &(itMap->second); 

	for (holding_map::const_iterator it=mpHoldings.begin();it!=mpHoldings.end();++it)
	{
		if (!ptIgnoreHoldings || ptIgnoreHoldings->find(it->first) == ptIgnoreHoldings->end())
		{
			player_map*ptFilteredMap=NULL;
			if (ptHoldingPlayers)
			{
				// store an empty player map in this holding, and get a pointer to it
				ptFilteredMap= &(
					(*ptHoldingPlayers)[it->first] = player_map()
				);
			}
			it->second.GetNotification(objNotify,lstIgnoreClans,lstIgnorePlayers,ptFilteredMap,getCurrent,getInverse);
			mpNotify[it->first] = objNotify;
		}
	}
}
		
void df_proximity_monitor::ManageList(df_proximity_monitor::E_LISTWHICH eWhich,df_proximity_monitor::E_LISTOP eOp,const std::string&strValue)
{
	if (eWhich < LST_COUNT)
	{
		string_set&stList=mpSets[eWhich];
		switch (eOp)
		{
			case OP_ADD:
				stList.insert(strValue); break;
			case OP_DEL:
			{
				string_set::iterator it=stList.find(strValue);
				if (it!=stList.end())
					stList.erase(it);
				break;
			}
			case OP_CLEAR:
				stList.clear(); break;
		}
	}
}
bool df_proximity_monitor::ProcessLastSeen(const Timestamp&objTime,const df_proximity_message&objMessage)
{
	// arbitrary block
	{
		holding_timestamp_pair&objPair=mpLastSeenPlayer[objMessage.objPlayer.strName];
		if (objPair.second < objTime)
		{
			mpLastSeenPlayer[objMessage.objPlayer.strName] = holding_timestamp_pair(objMessage.strHolding,objTime);	
			return true;
		}
	}

	// we already ensure player names only send the most recent messages, but we dont for clan names... so we handle that here
	// arbitrary block
	{
		holding_timestamp_pair&objPair=mpLastSeenClan[objMessage.objPlayer.strClan];
		if (objPair.second < objTime)
		{
			objPair = holding_timestamp_pair(objMessage.strHolding,objTime);	
			return true;
		}
	}
	return false;
}

bool df_proximity_monitor::SeenPlayer(const std::string&strName,holding_timestamp_pair&obj)
{
	lastseen_map::iterator it=mpLastSeenPlayer.find(strName);
	if (it == mpLastSeenPlayer.end()) return false;
	obj=it->second;
	return true;
}
bool df_proximity_monitor::SeenClan(const std::string&strClan,holding_timestamp_pair&obj)
{
	lastseen_map::iterator it=mpLastSeenClan.find(strClan);
	if (it == mpLastSeenClan.end()) return false;
	obj=it->second;
	return true;
}
void df_proximity_monitor::AddSeenToSection(const lastseen_map&mpSeen, TiXmlElement*ptSection)
{
	if (!ptSection) return;
	TiXmlElement *ptElement;
	for (lastseen_map::const_iterator it=mpSeen.begin();it!=mpSeen.end();++it)
	{
		ptElement=new TiXmlElement("entry");
		ptElement->SetAttribute("name",it->first);
		ptElement->SetAttribute("holding",it->second.first);
		{
			std::stringstream ss;
			ss << it->second.second;

			ptElement->SetAttribute("time",ss.str());
		}
		ptSection->LinkEndChild(ptElement);
	}
}
void df_proximity_monitor::ReadSectionToSeen(lastseen_map&mpSeen, TiXmlElement*ptSection)
{
	if (!ptSection) return;
	// for each child of the node, add it to the list
	const char*ptTime, *ptHolding, *ptName;
	for( TiXmlElement*child = ptSection->FirstChildElement(); child; child = child->NextSiblingElement() )
	{
		ptTime=child->Attribute("time");
		ptHolding=child->Attribute("holding");
		ptName=child->Attribute("name");
		if (ptTime && ptHolding && ptName)
		{
			Timestamp obj;
			obj.Load(ptTime);
			mpSeen[ptName] = holding_timestamp_pair(ptHolding,obj);
		}
	}
}

bool df_proximity_monitor::SaveSeen()
{
	if (strSeenFile.empty()) return false;
	bSeenChanged=false;
	TiXmlDocument objDoc;
	TiXmlElement * ptSection,*ptRoot;
	
	ptRoot = new TiXmlElement("rootnode");

	// Add an xml entry for each entry of the list
	ptSection=new TiXmlElement("players");
	AddSeenToSection(mpLastSeenPlayer,ptSection);
	ptRoot->LinkEndChild(ptSection);

	ptSection=new TiXmlElement("clans");
	AddSeenToSection(mpLastSeenClan,ptSection);
	ptRoot->LinkEndChild(ptSection);

	// Add it all to the document
	objDoc.LinkEndChild(new TiXmlDeclaration("1.0","",""));
	objDoc.LinkEndChild(ptRoot);
	
	return objDoc.SaveFile(strSeenFile.c_str());
}
bool df_proximity_monitor::LoadSeen()
{
	if (strSeenFile.empty()) return false;
	bSeenChanged=false;
	TiXmlDocument objDoc;
	TiXmlElement *ptSection;
	mpLastSeenPlayer.clear();
	mpLastSeenClan.clear();
	if (objDoc.LoadFile(strSeenFile.c_str()))
	{
		// find the node that holds this list of entries, and bail if it isn't there
		ptSection=XML_GetElement(objDoc.RootElement(),"players");
		ReadSectionToSeen(mpLastSeenPlayer,ptSection);
		ptSection=XML_GetElement(objDoc.RootElement(),"clans");
		ReadSectionToSeen(mpLastSeenClan,ptSection);
	}
	return true;
}
bool df_proximity_monitor::HasSeenChanged()
{
	return bSeenChanged;
}
bool df_proximity_monitor::GetBankPolicy(std::string&strPolicy,unsigned int&uIndex)
{
	TiXmlDocument objDoc;
	std::string strPage;
	return objWeb.GetBankPolicy(strPolicy,uIndex,strPage,objDoc);
}
bool df_proximity_monitor::SetBankPolicy(const unsigned int uIndex)
{
	TiXmlDocument objDoc;
	std::string strPage;
	return objWeb.SetBankPolicy(uIndex,strPage,objDoc);
}
bool df_proximity_monitor::GetTowerPolicy(std::string&strPolicy,unsigned int&uIndex)
{
	TiXmlDocument objDoc;
	std::string strPage;
	return objWeb.GetTowerPolicy(strPolicy,uIndex,strPage,objDoc);
}
bool df_proximity_monitor::SetTowerPolicy(const unsigned int uIndex)
{
	TiXmlDocument objDoc;
	std::string strPage;
	return objWeb.SetTowerPolicy(uIndex,strPage,objDoc);
}
unsigned int df_proximity_monitor::BankAutoCloseDelay() const
{
	return uBankAutoCloseDelay;
}
void df_proximity_monitor::SetBankAutoCloseDelay(const unsigned int uDelay)
{
	uBankAutoCloseDelay=uDelay;
}

