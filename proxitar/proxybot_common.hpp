#ifndef INCLUDED_PROXYBOT_COMMON_HPP
#define INCLUDED_PROXYBOT_COMMON_HPP

#include <tinyxml/tinyxml.h>
#include <string>
#include <sstream>
#include <iostream>
#include <list>
#include <algorithm>
#include <cctype>
#include <time.h>
#include <iomanip>

std::string NullFix(const char*ptString);

TiXmlElement*XML_GetElement(TiXmlElement*ptNode,const std::string&strElement);

struct Timestamp
{
	Timestamp();
	unsigned int uYear;
	unsigned int uMonth;
	unsigned int uDay;
	unsigned int uHour;
	unsigned int uMinute;
	unsigned int uSecond;
	bool Load(const std::string&strDate,const bool&hasYear=true);
	bool operator<(const Timestamp&rhs) const;
	bool operator==(const Timestamp&rhs) const;
	friend std::ostream&operator<<(std::ostream&os,const Timestamp&rhs); 
};

std::ostream&operator<<(std::ostream&os,const Timestamp&rhs);


void parse_delimited_stream(std::istream&is,std::list<std::string>&lstTokens,const char&delim);
std::string string_to_lower_ret(const std::string&sVal);
std::string& string_to_lower(std::string&sBuffer);

std::string string_to_upper_ret(const std::string&sVal);
std::string& string_to_upper(std::string&sBuffer);

std::string get_current_timestamp_string();
#endif
