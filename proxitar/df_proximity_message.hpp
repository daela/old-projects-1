#ifndef INCLUDED_PROXIMITY_MESSAGE_HPP
#define INCLUDED_PROXIMITY_MESSAGE_HPP

#include <string>
#include <iostream>
#include "df_player.hpp"
#include "proxybot_common.hpp"

struct df_proximity_message
{
	std::string strHolding;
	bool isEnter;
	df_player objPlayer;

	bool Parse(const std::string&strMessage);
	static std::string StripColor(const std::string&strMessage);
	friend std::ostream&operator<<(std::ostream&os,const df_proximity_message&rhs);
};

std::ostream&operator<<(std::ostream&os,const df_proximity_message&rhs);

#endif
