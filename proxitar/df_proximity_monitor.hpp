#ifndef INCLUDED_DF_PROXIMITY_MONITOR_HPP
#define INCLUDED_DF_PROXIMITY_MONITOR_HPP

#include <string>
#include <set>
#include <map>

#include "df_web.hpp"
#include "df_player.hpp"
#include "df_holding.hpp"
#include "proxybot_common.hpp"

struct df_seen
{
	Timestamp objTime;
	std::string strHolding;
};
class df_proximity_monitor
{
	public:
		typedef std::map<std::string,df_holding> holding_map;
	private:
		df_web objWeb;
		std::string strWebURL;
		std::string strUser;
		std::string strPassHash;
		std::string strSessionFile;
		std::string strSeenFile;
		unsigned int uBankAutoCloseDelay;

		holding_map mpHoldings;	

		// not saved to monitor.xml
		std::string strClearUniqueTime;

	public:
		typedef std::pair<std::string,Timestamp> holding_timestamp_pair;
		typedef std::map<std::string,holding_timestamp_pair> lastseen_map;
	private:
		lastseen_map mpLastSeenPlayer;
		lastseen_map mpLastSeenClan;
		bool bSeenChanged;
		Timestamp uLastTime;

	public:
		enum E_LISTWHICH { LST_ADMIN=0, LST_ALLYCLAN, LST_ALLYPLAYER, LST_IGNORECLAN, LST_IGNOREPLAYER, LST_IGNOREHOLDING, LST_COUNT };
		enum E_LISTOP { OP_ADD, OP_DEL, OP_CLEAR };
		static const char*ptListString[6];

		typedef df_holding::string_set string_set;
		typedef df_holding::string_set_ptr_list string_set_ptr_list;
		typedef df_holding::player_map player_map;

		typedef std::map<E_LISTWHICH,string_set> set_map;
		typedef std::map<std::string,df_holding_notification> notify_map;

		typedef std::map<std::string,player_map> holding_player_map; 

		// a list of sets
		set_map mpSets;

		df_proximity_monitor();
		bool Connect();
		std::string GetClearUniqueTime() const;
		bool LoadConfig(const std::string&strFilename);
		bool SaveConfig(const std::string&strFilename);
		bool SaveList(const std::string&strFilename);
		void SaveListXml(TiXmlElement*ptRoot);

		void ClearUniques();

		static bool SaveHoldingPlayers(const std::string&strFilename,const std::string&strStartTime,const holding_player_map&mpHoldingPlayers);

		bool IsAdmin(const std::string&strName);
		bool Check();
		void GetNotifications(notify_map&mpNotify,holding_player_map*ptHoldingPlayers=NULL,const bool getCurrent=true,const bool getInverse=false) const;

		void ManageList(E_LISTWHICH eWhich,E_LISTOP eOp,const std::string&strValue);
		bool SeenPlayer(const std::string&strName,holding_timestamp_pair&obj);
		bool SeenClan(const std::string&strClan,holding_timestamp_pair&obj);
		bool SaveSeen();
		bool LoadSeen();
		bool HasSeenChanged();
		bool GetBankPolicy(std::string&strPolicy,unsigned int&uIndex);
		bool SetBankPolicy(const unsigned int uIndex);
		bool GetTowerPolicy(std::string&strPolicy,unsigned int&uIndex);
		bool SetTowerPolicy(const unsigned int uIndex);
		
		unsigned int BankAutoCloseDelay() const;
		void SetBankAutoCloseDelay(const unsigned int uDelay);

	private:
		bool ProcessLastSeen(const Timestamp&objTime,const df_proximity_message&objMessage);
		void AddSetToNode(E_LISTWHICH eWhich, TiXmlElement*ptParent);
		void ReadNodeToSet(E_LISTWHICH eWhich, TiXmlElement*ptParent);
		void AddSeenToSection(const lastseen_map&mpSeen, TiXmlElement*ptParent);
		void ReadSectionToSeen(lastseen_map&mpSeen, TiXmlElement*ptParent);
};

#endif
