#include "proxybot_common.hpp"

std::string NullFix(const char*ptString)
{
	if (!ptString)
		return "";
	return ptString;
}
TiXmlElement*XML_GetElement(TiXmlElement*ptNode,const std::string&strElement)
{
	std::string::size_type uStart, uPos;
	
	for (uStart=0;uStart != std::string::npos;uStart=uPos+1)
	{
		uPos=strElement.find('.',uStart);
		if (!ptNode) break;
		ptNode=ptNode->FirstChildElement(strElement.substr(uStart,uPos-uStart).c_str());
		if (uPos == std::string::npos) break;
	}
	return ptNode;
}
Timestamp::Timestamp()
{ uYear=uMonth=uDay=uHour=uMinute=uSecond=0; }

bool Timestamp::Load(const std::string&strDate,const bool&hasYear)
{
	char ch;
	std::stringstream ss;
	ss << strDate;
	// mumble "[11:37:47 PM]"
	if (strDate[0] == '[')
	{
		// mumble
		std::string strSuffix;
		ss >> ch >> uHour >> ch >> uMinute >> ch >> uSecond >> ch >> strSuffix;
		if (strSuffix[0] == 'P') // PM
		{
			uHour += 12;
		}
	}
	// ventrilo "2010-03-28 21:02:47"
	else
	{
		if (hasYear)
			ss >> uYear >> ch;
		ss >> uMonth >> ch >> uDay >> uHour >> ch >> uMinute >> ch >> uSecond;
	}
	if (ss) return true;
	return false;
}
bool Timestamp::operator==(const Timestamp&rhs) const
{
	return (
		uYear == rhs.uYear &&
		uMonth == rhs.uMonth &&
		uDay == rhs.uDay &&
		uHour == rhs.uHour &&
		uMinute == rhs.uMinute &&
		uSecond == rhs.uSecond
	);
}
	
bool Timestamp::operator<(const Timestamp&rhs) const
{
	if (uYear < rhs.uYear) return true;
	if (rhs.uYear < uYear) return false;
	if (uMonth < rhs.uMonth) return true;
	if (rhs.uMonth < uMonth) return false;
	if (uDay < rhs.uDay) return true;
	if (rhs.uDay < uDay) return false;
	if (uHour < rhs.uHour) return true;
	if (rhs.uHour < uHour) return false;
	if (uMinute < rhs.uMinute) return true;
	if (rhs.uMinute < uMinute) return false;
	return (uSecond<rhs.uSecond);
}

std::ostream&operator<<(std::ostream&os,const Timestamp&rhs)
{
	os << rhs.uYear << "-" << rhs.uMonth << "-" << rhs.uDay << " " << rhs.uHour << ":" << rhs.uMinute << ":" << rhs.uSecond;
	return os;
}

void parse_delimited_stream(std::istream&is,std::list<std::string>&lstTokens,const char&delim)
{
	std::string strToken;
	while (std::getline(is,strToken,delim))
	{
		std::string::size_type uPos=strToken.find_first_not_of(" \t");
		if (uPos)
			strToken=strToken.substr(uPos);
		uPos=strToken.find_last_not_of(" \t");
		if (uPos != std::string::npos)
			strToken=strToken.substr(0,uPos+1);
		lstTokens.push_back(strToken);	
	}
}
std::string string_to_lower_ret(const std::string&sVal)
{
	std::string sBuffer=sVal;
	string_to_lower(sBuffer);
	return sBuffer;
}
std::string& string_to_lower(std::string&sBuffer)
{
	std::transform( sBuffer.begin(), sBuffer.end(), sBuffer.begin(), ::tolower );
	return sBuffer;
}

std::string string_to_upper_ret(const std::string&sVal)
{
	std::string sBuffer=sVal;
	string_to_upper(sBuffer);
	return sBuffer;
}
std::string& string_to_upper(std::string&sBuffer)
{
	std::transform( sBuffer.begin(), sBuffer.end(), sBuffer.begin(), ::toupper );
	return sBuffer;
}

std::string get_current_timestamp_string()
{
	struct tm *current;
	time_t now;

	time(&now);
	current = localtime(&now);

	std::stringstream ss;
	ss	<< std::setw(2) << std::setfill('0') << (current->tm_mon+1) << '-'
		<< std::setw(2) << std::setfill('0') << current->tm_mday << '-'
		<< std::setw(4) << std::setfill('0') << (1900+current->tm_year) << " "
		<< std::setw(2) << std::setfill('0') << current->tm_hour << ':'
		<< std::setw(2) << std::setfill('0') << current->tm_min << ':'
		<< std::setw(2) << std::setfill('0') << current->tm_sec;
	return ss.str();
}
