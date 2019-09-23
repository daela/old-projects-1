#include "strmanip.h"

namespace nac
{
	template <>
	void string_case<TO_LOWER>(std::string &s, std::locale const & l )
	{
	  for(std::string::iterator e= s.begin(); e!= s.end(); ++e)
		*e= std::tolower(*e, l);
	}

	template <>
	void string_case<TO_UPPER>(std::string &s, std::locale const & l )
	{
	  for(std::string::iterator e= s.begin(); e!= s.end(); ++e)
		*e= std::toupper(*e, l);
	}

	void split_path(const std::string&strFilePath,std::string&strPath,std::string&strFile)
	{
		std::string::size_type pos = strFilePath.find_last_of("\\/");
		if (pos != std::string::npos)
		{
			strPath = strFilePath.substr(0,pos+1);
			strFile = strFilePath.substr(pos+1);
		}
		else
		{
			strPath = "";
			strFile = strFilePath;
		}
	}
}
