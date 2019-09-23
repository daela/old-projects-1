#include <nx/os.hpp>
#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <map>
#include <list>

#include <time.h>

#include "df_proximity_monitor.hpp"
#include "ventrilo_chat.hpp"

#ifndef OS_WINDOWS_
#include "mumble_wrapper.hpp"
#endif

#ifndef OS_WINDOWS_
#include <unistd.h>
#endif


void os_millisleep(const unsigned long&msec)
{
	#if defined(OS_WINDOWS_)
	Sleep(msec);
	#else
	usleep(msec * 1000ul);
	#endif
}
void os_speechdelay(const unsigned long&msec)
{
	//#if defined(OS_WINDOWS_)
	os_millisleep(msec);
	//#endif
}
template <class T> void nowrap_subtract(T&uVal,const T&uAmount)
{
	uVal = (uVal > uAmount?uVal-uAmount:0);
}
std::string stringize_list(const std::string&strPrefix,std::list<std::string>&lstMessage)
{
	if (lstMessage.empty()) return "";

	std::stringstream ss;
	ss << strPrefix;

	std::list<std::string>::iterator itNext;
	for (std::list<std::string>::iterator it=lstMessage.begin();it!=lstMessage.end();it=itNext)
	{
		itNext=it;
		++itNext;
		if (it != lstMessage.begin())
		{
			ss << ", ";
			if (itNext == lstMessage.end())
				ss << "and ";
		}
		ss << *it;
	}
	return ss.str();
}

	
struct ListCommand
{
	enum E_CMD
	{
		CMD_INVALID=0,
		CMD_ALLY_CLAN, CMD_DEALLY_CLAN, CMD_DEALLY_ALL_CLANS,
		CMD_ALLY_PLAYER, CMD_DEALLY_PLAYER, CMD_DEALLY_ALL_PLAYERS,
		CMD_IGNORE_CLAN, CMD_UNIGNORE_CLAN, CMD_UNIGNORE_ALL_CLANS,
		CMD_IGNORE_PLAYER, CMD_UNIGNORE_PLAYER, CMD_UNIGNORE_ALL_PLAYERS,
		CMD_IGNORE_HOLDING, CMD_UNIGNORE_HOLDING, CMD_UNIGNORE_ALL_HOLDINGS,
	};

	E_CMD eCmd;
	df_proximity_monitor::E_LISTWHICH eWhich;
	df_proximity_monitor::E_LISTOP eOp;

	ListCommand()
	{ }
	ListCommand(E_CMD cmd,df_proximity_monitor::E_LISTWHICH which,df_proximity_monitor::E_LISTOP op)
	: eCmd(cmd), eWhich(which), eOp(op)
	{ }
};
int main(int argc, char *argv[])
{
	// initialize libcurl
	curl_global_init(CURL_GLOBAL_ALL);


	#if 0
	{
		std::string strInput,strOutput;
		TiXmlDocument objDoc;
		strInput="";
		df_web::Decode(strInput,strOutput);
		std::cout << strOutput;
		return 0;
	}
	#endif
	const std::string strMonitorConfig="monitor.xml";

	df_proximity_monitor objMonitor;

	typedef df_proximity_monitor::notify_map notify_map;

	typedef std::map<std::string,std::string> lastmsg_map;
	lastmsg_map mpLastMessage;
	notify_map mpNotifications;

	std::string strMessage;

	if (!objMonitor.LoadConfig(strMonitorConfig))
	{
		std::cerr << "Your monitor configuration is invalid!" << std::endl;
		return 1;
	}
	std::cout << "LoadSeen: " << objMonitor.LoadSeen() << std::endl;

	ventrilo_chat objChat;
	std::cout << "Find Chat Window: " << objChat.Find() << std::endl;

	Timestamp objLastChatTime, objMostRecentTime;
	const Timestamp objEmptyTimestamp;

	typedef std::list<std::string> msg_list;
	msg_list lstNotice, lstWarning;
	bool bWantStatus;

	bool isBankOpen=false;
	time_t timeBankOpen=time_t();

	std::string strBankPolicy;
	unsigned int uBankPolicyIndex;

	ventrilo_message objMessage;
	#ifndef OS_WINDOWS_
	mumble_wrapper objMumble;
	nx::thread::handle hMumble;
	objMumble.spawn(hMumble);
	os_millisleep(10000); // 10 seconds to join
	#endif
	typedef std::map<std::string,ListCommand> listcommand_map;
	listcommand_map mpListCommand;
	
	mpListCommand["ally_clan"]        = ListCommand(ListCommand::CMD_ALLY_CLAN,        df_proximity_monitor::LST_ALLYCLAN, df_proximity_monitor::OP_ADD  );
	mpListCommand["deally_clan"]      = ListCommand(ListCommand::CMD_DEALLY_CLAN,      df_proximity_monitor::LST_ALLYCLAN, df_proximity_monitor::OP_DEL  );
	mpListCommand["deally_all_clans"] = ListCommand(ListCommand::CMD_DEALLY_ALL_CLANS, df_proximity_monitor::LST_ALLYCLAN, df_proximity_monitor::OP_CLEAR);

	mpListCommand["ally_player"]        = ListCommand(ListCommand::CMD_ALLY_PLAYER,        df_proximity_monitor::LST_ALLYPLAYER, df_proximity_monitor::OP_ADD  );
	mpListCommand["deally_player"]      = ListCommand(ListCommand::CMD_DEALLY_PLAYER,      df_proximity_monitor::LST_ALLYPLAYER, df_proximity_monitor::OP_DEL  );
	mpListCommand["deally_all_players"] = ListCommand(ListCommand::CMD_DEALLY_ALL_PLAYERS, df_proximity_monitor::LST_ALLYPLAYER, df_proximity_monitor::OP_CLEAR);

	mpListCommand["ignore_clan"]        = ListCommand(ListCommand::CMD_IGNORE_CLAN,        df_proximity_monitor::LST_IGNORECLAN, df_proximity_monitor::OP_ADD  );
	mpListCommand["unignore_clan"]      = ListCommand(ListCommand::CMD_UNIGNORE_CLAN,      df_proximity_monitor::LST_IGNORECLAN, df_proximity_monitor::OP_DEL  );
	mpListCommand["unignore_all_clans"] = ListCommand(ListCommand::CMD_UNIGNORE_ALL_CLANS, df_proximity_monitor::LST_IGNORECLAN, df_proximity_monitor::OP_CLEAR);

	mpListCommand["ignore_player"]        = ListCommand(ListCommand::CMD_IGNORE_PLAYER,        df_proximity_monitor::LST_IGNOREPLAYER, df_proximity_monitor::OP_ADD  );
	mpListCommand["unignore_player"]      = ListCommand(ListCommand::CMD_UNIGNORE_PLAYER,      df_proximity_monitor::LST_IGNOREPLAYER, df_proximity_monitor::OP_DEL  );
	mpListCommand["unignore_all_players"] = ListCommand(ListCommand::CMD_UNIGNORE_ALL_PLAYERS, df_proximity_monitor::LST_IGNOREPLAYER, df_proximity_monitor::OP_CLEAR);

	mpListCommand["ignore_holding"]        = ListCommand(ListCommand::CMD_IGNORE_HOLDING,        df_proximity_monitor::LST_IGNOREHOLDING, df_proximity_monitor::OP_ADD  );
	mpListCommand["unignore_holding"]      = ListCommand(ListCommand::CMD_UNIGNORE_HOLDING,      df_proximity_monitor::LST_IGNOREHOLDING, df_proximity_monitor::OP_DEL  );
	mpListCommand["unignore_all_holdings"] = ListCommand(ListCommand::CMD_UNIGNORE_ALL_HOLDINGS, df_proximity_monitor::LST_IGNOREHOLDING, df_proximity_monitor::OP_CLEAR);

	objChat.Send("proxitar is connecting");
	os_speechdelay(4000);
	for (;;)
	{
		while (!objMonitor.Connect())
		{
			std::cerr << "Failed to connect, waiting a minute to retry...";
			os_millisleep(60000);
		}
		
		objChat.Send("SUCCESS, proxitar is connected");
		
		for (unsigned int uFailCount=0;uFailCount<10;)
		{
			bWantStatus=false;
			if (!objMonitor.Check())
			{
				// it failed, increment our fail count and sleep for 2 seconds
				++uFailCount;
				os_millisleep(2000);
				continue;
			}

			if (objMonitor.BankAutoCloseDelay())
			{
				if (objMonitor.GetBankPolicy(strBankPolicy,uBankPolicyIndex))
				{
					// TODO: maybe make this use something other than the number, if the need arises
					if (uBankPolicyIndex != 2)
					{
						if (isBankOpen)
						{
							time_t timeNow = time(NULL);
							if (static_cast<unsigned int>(timeNow - timeBankOpen) > objMonitor.BankAutoCloseDelay())
							{
								std::cout << "Closing the bank..." << std::endl;
								if (objMonitor.SetBankPolicy(2))
								{
									isBankOpen=false;
									objChat.Send("POLICY: the bank has been closed.");
									os_speechdelay(2500);
								}
							}
						}
						else
						{
							isBankOpen=true;
							timeBankOpen = time(NULL);
							std::cout << "The bank will be closed in " << objMonitor.BankAutoCloseDelay() << " second(s)." << std::endl;
						}
					}
					else
					{
						isBankOpen=false;
					}
				}
			}
			else
			{
				isBankOpen=false;
			}

			#define NX_PROXYBOT_FAST_FREQUENCY 4000
			#define NX_PROXYBOT_SLOW_FREQUENCY 10000
			unsigned int uFrequency=NX_PROXYBOT_SLOW_FREQUENCY;

			uFailCount=0;

			lstNotice.clear();
			lstWarning.clear();

			// do any other commands
			std::vector<ventrilo_message> vcChatMessages;
			if (objChat.GetText())
			{
				ventrilo_chat::bufsize_t uPos=-1;
				std::string strLine;
				for (unsigned int i=0;objChat.GetLastLine(uPos,strLine);++i)
				{
					if (!objMessage.Load(strLine) || objMessage.objTime == objEmptyTimestamp) break;

					if (i == 0)
					{
						// if we don't have a time yet (first time we ran this), set it and bail.
						if (objLastChatTime == objEmptyTimestamp)
						{
							objMostRecentTime = objMessage.objTime;
							break;
						}
						// set the latest time we read this update
						objMostRecentTime = objMessage.objTime;
					}
					// if this isn't a new message, bail
					if (!(objLastChatTime < objMessage.objTime))
						break;
					// we have a new, parsed message... check for commands!

					// TODO: push objMessage into a list, have this list also get loaded by any "event" messages (for mumble stuff), parse like usual
					// TODO: change timing to be fast if all holdings are empty, and slow if anyone is in a holding
					vcChatMessages.push_back(objMessage);
				}
				// update our last processed time	
				objLastChatTime=objMostRecentTime;
			}
			#ifndef OS_WINDOWS_	
			while (objMumble.objMessageQueue.try_pop(objMessage))
			{
				std::cout << "MUMBLE: " << objMessage << std::endl;
				vcChatMessages.push_back(objMessage);
			}
			#endif
			if (!vcChatMessages.empty())
			{
				for (std::vector<ventrilo_message>::iterator itMessage=vcChatMessages.begin();itMessage!=vcChatMessages.end();++itMessage)
				{
					objMessage=*itMessage;
					std::string strTemp, strCmd;
					std::stringstream ss;
					ss << objMessage.strMessage;
					// read in the bot prefix and command, make the prefix lowercase, and compare it
					bool isPlayer;
					if (ss >> strTemp >> strCmd && string_to_lower(strTemp) == "bot:")
					{
						string_to_lower(strCmd);
						listcommand_map::iterator itCmd=mpListCommand.find(strCmd);
						if (itCmd != mpListCommand.end() && objMonitor.IsAdmin(objMessage.strName))
						{
							switch (itCmd->second.eCmd)
							{
								// "all" commands don't have lists to process
								case ListCommand::CMD_DEALLY_ALL_CLANS:
								case ListCommand::CMD_DEALLY_ALL_PLAYERS:
								case ListCommand::CMD_UNIGNORE_ALL_CLANS:
								case ListCommand::CMD_UNIGNORE_ALL_PLAYERS:
								case ListCommand::CMD_UNIGNORE_ALL_HOLDINGS:
									objMonitor.ManageList(itCmd->second.eWhich,itCmd->second.eOp,"");
									break;
								// non-"all" commands process lists of parameters and pass them individually.
								default:
								{
									std::list<std::string> lstTokens;
									parse_delimited_stream(ss,lstTokens,',');
									std::set<std::string>::iterator itSet;				
									if (lstTokens.empty()) break;
									for (std::list<std::string>::const_iterator it=lstTokens.begin();it!=lstTokens.end();++it)
									{
										strTemp=string_to_lower_ret(*it);
										objMonitor.ManageList(itCmd->second.eWhich,itCmd->second.eOp,strTemp);
										if (itCmd->second.eCmd == ListCommand::CMD_IGNORE_HOLDING)
										{
											// now we wont get the notification, but when we unignore it later we
											// want the "previous state" to be considered "clear".
											lastmsg_map::iterator itNotify=mpLastMessage.find(strTemp);
											if (itNotify != mpLastMessage.end()) mpLastMessage.erase(itNotify);
										}
									}
								}
							}
							// save changes to our configuration
							objMonitor.SaveConfig(strMonitorConfig);
							// notify that the deed has been done
							objChat.Send("action performed");
							os_speechdelay(1500);
						}
						else if (strCmd == "status")
						{
							bWantStatus=true;
						}
						else if (strCmd == "sound")
						{
							// not attempting to cache this list
							std::ifstream ifs("sounds.cfg");
							if (ifs)
							{
								std::string strRequested;
								ss.get(); // remove whitespace
								std::getline(ss,strRequested);	

								if (strRequested.find('.') == std::string::npos)
								{
									// if no dot was found, we'll assume no extension was provided, so we'll add one
									strRequested += ".wav";
								}

								// look for an entry for this sound
								std::string strLine;
								while (std::getline(ifs,strLine))
								{
									std::stringstream ss(strLine);
									std::string strVolume;
									std::string strFile;
									if (ss >> strVolume >> strFile)
									{
										if (strFile == strRequested)
										{
											// call our sound playing script
											system((std::string("./scripts/play_sound.sh ")+strVolume+" sounds/"+strRequested).c_str());
											break;
										}
									}
								}
							}
						}
						else if (strCmd == "say" && objMonitor.IsAdmin(objMessage.strName))
						{
							std::string strMessage;
							std::getline(ss,strMessage);	
							objChat.Send(strMessage);
							os_speechdelay(2500);
						}
						else if (strCmd == "clear_unique" && objMonitor.IsAdmin(objMessage.strName))
						{
							objMonitor.ClearUniques();
							objChat.Send("SETTING: list of unique players has been cleared");
							os_speechdelay(4000);
						}
						else if (strCmd == "unique_allies" || strCmd == "unique_enemies" || strCmd == "current_allies" || strCmd == "current_enemies" || strCmd == "list")
						{
							bool isResult=false;
							if (strCmd != "list")
							{
								df_proximity_monitor::holding_player_map mpHoldingPlayers;
								// indexes into the string are fun!	
								bool getCurrent=(strCmd[0] == 'c');
								bool getInverse=(strCmd[7+getCurrent] == 'a');

								objMonitor.GetNotifications(mpNotifications,&mpHoldingPlayers,getCurrent,getInverse);
								isResult = df_proximity_monitor::SaveHoldingPlayers("result.xml",objMonitor.GetClearUniqueTime(),mpHoldingPlayers);
							}
							else
							{
								isResult = objMonitor.SaveList("result.xml");
							}
							if (isResult)
							{
								objChat.Send("RESULT: available at the link in my comment");
								system("./scripts/on_result_page.sh result.xml");
								os_speechdelay(4000);
							}
							else
							{
								objChat.Send("ERROR: problem writing results");
								os_speechdelay(4000);
							}
						}
						else if (strCmd == "tower")
						{
							bool isIndexPresent=true;
							std::string strIndex;
							unsigned int uIndex;
							bool isIndexGood=true;
							if (!(ss >> strIndex))
								isIndexPresent=false;
							string_to_lower(strIndex);
							if (strIndex == "off")
							{
								uIndex = 2;
							}
							else if (strIndex == "on")
							{
								uIndex = 0;
							}
							else if (strIndex == "enemy")
							{
								uIndex = 1;
							}
							else
							{
								std::stringstream ss;
								ss << strIndex;
								if (!(ss >> uIndex && uIndex < 3))
								{
									isIndexGood=false;
								}
							}
							
							if (isIndexGood)
							{
								std::cout << "Setting tower policy to: " << uIndex << std::endl;
								objMonitor.SetTowerPolicy(uIndex);
								// TODO: stop using BANK's temp variables, use my own... these names suck
								if (objMonitor.GetTowerPolicy(strBankPolicy,uBankPolicyIndex))
								{
									std::string strPolicyIndex;
									{
										std::stringstream ss;
										ss << uBankPolicyIndex;
										ss >> strPolicyIndex;
									}
									objChat.Send(std::string("POLICY: tower now set to, ") + strPolicyIndex + ", " + strBankPolicy);
									os_speechdelay(6000);
								}
								else
								{
									objChat.Send("ERROR: tower policy could not be retrieved.");
									os_speechdelay(3000);
								}
							}
							else // no valid index, so we just give status
							{
								if (isIndexPresent)
								{
									objChat.Send("ERROR: valid tower settings are, on, off, or enemy");
									os_speechdelay(5000);
								}
								else
								{
									if (objMonitor.GetTowerPolicy(strBankPolicy,uBankPolicyIndex))
									{
										std::string strPolicyIndex;
										{
											std::stringstream ss;
											ss << uBankPolicyIndex;
											ss >> strPolicyIndex;
										}
										objChat.Send(std::string("POLICY: tower currently set to ") + strPolicyIndex + ", " + strBankPolicy);
										os_speechdelay(6000);
									}
									else
									{
										objChat.Send("ERROR: tower policy could not be retrieved.");
										os_speechdelay(3000);
									}
								}
							}

						}
						else if (strCmd == "bank")
						{
							bool isIndexPresent=true;
							std::string strIndex;
							unsigned int uIndex;
							bool isIndexGood=true;
							if (!(ss >> strIndex))
								isIndexPresent=false;
							string_to_lower(strIndex);
							if (strIndex == "close" || strIndex == "ally")
							{
								uIndex = 2;
							}
							else if (strIndex == "open" || strIndex == "all")
							{
								uIndex = 0;
							}
							else
							{
								std::stringstream ss;
								ss << strIndex;
								if (!(ss >> uIndex && uIndex < 4))
								{
									isIndexGood=false;
								}
							}
							
							if (isIndexGood)
							{
								std::cout << "Setting bank policy to: " << uIndex << std::endl;
								objMonitor.SetBankPolicy(uIndex);
								if (objMonitor.GetBankPolicy(strBankPolicy,uBankPolicyIndex))
								{
									std::string strPolicyIndex;
									{
										std::stringstream ss;
										ss << uBankPolicyIndex;
										ss >> strPolicyIndex;
									}
									objChat.Send(std::string("POLICY: bank now set to, ") + strPolicyIndex + ", " + strBankPolicy);
									os_speechdelay(6000);
								}
								else
								{
									objChat.Send("ERROR: bank policy could not be retrieved.");
									os_speechdelay(3000);
								}
							}
							else // no valid index, so we just give status
							{
								if (strIndex == "auto" && objMonitor.IsAdmin(objMessage.strName))
								{
									unsigned int uDelay;
									if (!(ss >> uDelay))
									{
										uDelay=300;
									}
									objMonitor.SetBankAutoCloseDelay(uDelay);
									objMonitor.SaveConfig(strMonitorConfig);
									if (uDelay)
									{
										std::string strDelay;
										std::stringstream ss;
										ss << uDelay;
										ss >> strDelay;
										objChat.Send(std::string("POLICY: the bank will now automatically close when open more than ")+ strDelay + " seconds");
										os_speechdelay(8000);
									}
									else
									{
										objChat.Send("POLICY: the bank will not be closed automatically, close it yourself.");
										os_speechdelay(8000);
									}
								}
								else
								{
									if (isIndexPresent)
									{
										objChat.Send("ERROR: valid bank settings are, open or close");
										os_speechdelay(3500);
									}
									else
									{
										if (objMonitor.GetBankPolicy(strBankPolicy,uBankPolicyIndex))
										{
											std::string strPolicyIndex;
											{
												std::stringstream ss;
												ss << uBankPolicyIndex;
												ss >> strPolicyIndex;
											}
											objChat.Send(std::string("POLICY: bank currently set to ") + strPolicyIndex + ", " + strBankPolicy);
											os_speechdelay(6000);
										}
										else
										{
											objChat.Send("ERROR: bank policy could not be retrieved.");
											os_speechdelay(3000);
										}
									}
								}
							}
						}
							
						else if ((isPlayer=(strCmd == "seen_player")) || strCmd == "seen_clan")
						{
							//SEEN: nacitar sevaht remains unseen, Robert Goulet was in Mehatil on 
							std::list<std::string> lstTokens;
							parse_delimited_stream(ss,lstTokens,',');

							if (!lstTokens.empty())
							{
								std::stringstream ss;
								ss << "SEEN: ";
								std::list<std::string>::const_iterator itNext;
								df_proximity_monitor::holding_timestamp_pair objSeenPair;
								for (std::list<std::string>::const_iterator it=lstTokens.begin();it!=lstTokens.end();it=itNext)
								{
									itNext=it;
									++itNext;
									strTemp=string_to_lower_ret(*it);
									if (it != lstTokens.begin())
									{
										ss << ", ";
										if (itNext==lstTokens.end())
											ss << "and ";
									}
									ss << strTemp;
									if ((isPlayer && objMonitor.SeenPlayer(strTemp,objSeenPair)) || (!isPlayer && objMonitor.SeenClan(strTemp,objSeenPair)))
										ss << " was in " << objSeenPair.first << " on " << objSeenPair.second;
									else
										ss << " remains unseen";
								}
								objChat.Send(ss.str());
								os_speechdelay(3000*lstTokens.size());
							}
						}
						else
						{
							// TODO: handle command spam
							//objChat.Send("ERROR: invalid command");
							//os_speechdelay(2000);
						}
					}
					//std::cout << "Line: " << objMessage << std::endl;
				}
			}
			objMonitor.GetNotifications(mpNotifications);
			unsigned int uEnemies=0;
			for (notify_map::iterator it=mpNotifications.begin();it!=mpNotifications.end();++it)
			{
				lastmsg_map::iterator itLast=mpLastMessage.find(it->first);
				if (itLast==mpLastMessage.end())
				{
					// if we didn't find a previous message, change the count to 0 to generate a clear message
					unsigned int uTemp=it->second.uCount;
					it->second.uCount=0;
					// save the iterator upon insertion
					itLast=mpLastMessage.insert(lastmsg_map::value_type(it->first,it->second.GetMessageString())).first;
					// restore the original count
					it->second.uCount=uTemp;
				}
				// no matter what, increment our all-holdings-enemy-count
				uEnemies += it->second.uCount;
				std::string strMessage=it->second.GetMessageString();
				if (bWantStatus || itLast->second != strMessage)
				{
					itLast->second=strMessage;
					if (it->second.uCount == 0)
					{
						// if this is a normal request (meaning, it's different than before)
						// or if it doesn't match... output the notice
						// REASON: we don't want "is clear" spam on a status update, unless it's new
						if (!bWantStatus || itLast->second != strMessage)
							lstNotice.push_back(strMessage);
					}
					else	
						lstWarning.push_back(strMessage);
				}
			}


			//#ifdef OS_WINDOWS_
			#define NX_PROXYBOT_WARNING_PREFIX "WARNING: "
			#define NX_PROXYBOT_NOTICE_PREFIX "NOTICE: "
			//#else
			//#define NX_PROXYBOT_WARNING_PREFIX ""
			//#define NX_PROXYBOT_NOTICE_PREFIX ""
			//#endif
			
			strMessage=stringize_list(NX_PROXYBOT_WARNING_PREFIX,lstWarning);
			if (!strMessage.empty())
			{
				std::string strStupidClanName="wwwwwwwwwwwwwwwwwwwwwwwww";
				std::string::size_type pos=strMessage.find(strStupidClanName);
				if (pos != std::string::npos)
				{
					strMessage.replace(pos,strStupidClanName.size(),"25 W");
				}	
				objChat.Send(strMessage);
				unsigned int uDelay=4000*lstWarning.size();
				nowrap_subtract(uFrequency,uDelay);
				os_speechdelay(uDelay);
			}
			// if no holdings have any enemies at all, and we either requested the status, or we have at least one new clear message
			if (!uEnemies && (bWantStatus || !lstNotice.empty()))
			{
				strMessage=NX_PROXYBOT_NOTICE_PREFIX "all clear";
				uFrequency=NX_PROXYBOT_FAST_FREQUENCY;
			}
			else
			{
				strMessage=stringize_list(NX_PROXYBOT_NOTICE_PREFIX,lstNotice);
				uFrequency=NX_PROXYBOT_SLOW_FREQUENCY;
			}

			if (!strMessage.empty())
			{
				objChat.Send(strMessage);
				unsigned int uDelay=4000;

				// if we didn't make an all clear message, multiply it out by the number of notifications
				if (uEnemies) uDelay *= lstNotice.size();

				nowrap_subtract(uFrequency,uDelay);
				os_speechdelay(uDelay);
			}
			std::cout << "= " << time(NULL) << std::endl;
			if (objMonitor.HasSeenChanged())
				std::cout << "Seen: " << objMonitor.SaveSeen() << std::endl;
			os_millisleep(uFrequency);
		}
		objChat.Send("ERROR: proxitar has disconnected, attempting reconnection...");
		os_speechdelay(5000);
	}
	// cleanup libcurl
	curl_global_cleanup();
	return 0;	
}
