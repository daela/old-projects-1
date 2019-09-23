#ifndef INCLUDED_DF_HOLDING_HPP
#define INCLUDED_DF_HOLDING_HPP
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <list>

#include "df_player.hpp"
#include "df_proximity_message.hpp"
#include "proxybot_common.hpp"
struct df_holding_notification
{
	std::string strHolding;
	unsigned int uCount;
	std::string strPrimaryClan;
	std::string strOnlyPerson;
	bool bOtherClans;
	std::string GetMessageString() const;
	void Clear(const std::string&strNewHolding="");
};
// TODO: GetNotification needs to accept ignore lists!
struct df_holding
{
	typedef std::set<std::string> string_set;
	typedef std::list<const string_set*> string_set_ptr_list;
	df_holding(const std::string&strHoldingName="");
	std::string strName;

	typedef std::map<std::string,df_player> player_map;
	player_map mpPresent;

	player_map mpUnique;


	void Clear(const std::string&strNewHolding="");
	void ClearUnique();

	bool ProcessMessage(const df_proximity_message&objMsg);

	void GetNotification(
		df_holding_notification&objNotify,
		const string_set_ptr_list&lstIgnoreClanSets,
		const string_set_ptr_list&lstIgnorePlayerSets,
		player_map*ptFilteredMap=NULL,
		const bool getCurrent=true,
		const bool getInverse=false) const;
};

#endif
