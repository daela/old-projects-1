#ifndef INCLUDED_DF_WEB_HPP
#define INCLUDED_DF_WEB_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include <map>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

//#include <cryptopp/sha.h>
//#include <cryptopp/hex.h>
//#include <cryptopp/filters.h>
#include "sha1.h"

#include <tinyxml/tinyxml.h>

#include "proxybot_common.hpp"
struct df_web
{
	enum POLICIES { DF_BANK_POLICY, DF_TOWER_POLICY };
	std::string strBaseURL;
	std::string strSessionKey;
	std::string strClanID;
	df_web(const std::string&base_url="http://app001.us1.darkfallonline.com/sf/fwd");
	bool SetBaseURL(const std::string&base_url);

	template <class T>
	static std::string numeric_string(const T&val)
	{
		std::stringstream ss;
		ss << val;
		return ss.str();
		
	}
	static std::string time_string();
	static std::string sha1sum(const std::string&strData);

	static size_t StringAppendCallback(void *ptr, size_t size, size_t nmemb, void *data);

	static bool PullPage(const std::string&strAddress,std::string&strOutput);
	static bool PullXmlPage(const std::string&strAddress,std::string&strOutput,TiXmlDocument&objDoc);
	static bool DecodeXmlDocument(TiXmlDocument&objDoc);

	bool GetBankPolicy(std::string&strPolicy,unsigned int&uIndex,std::string&strPage,TiXmlDocument&objDoc);
	bool GetTowerPolicy(std::string&strPolicy,unsigned int&uIndex,std::string&strPage,TiXmlDocument&objDoc);
	bool GetPolicy(POLICIES ePolicy,std::string&strPolicy,unsigned int&uIndex,std::string&strPage,TiXmlDocument&objDoc);

	bool SetBankPolicy(const unsigned int uIndex,std::string&strPage,TiXmlDocument&objDoc);
	bool SetTowerPolicy(const unsigned int uIndex,std::string&strPage,TiXmlDocument&objDoc);
	bool SetPolicy(POLICIES ePolicy,const unsigned int uIndex,std::string&strPage,TiXmlDocument&objDoc);

	bool GetProximityPage(const unsigned int&uStartRow,const unsigned int&uPageRows,const unsigned int&uPage,std::string&strPage,TiXmlDocument&objDoc);
	bool Login(const std::string&strUser,const std::string&strPassHash);

	bool GetClanID();
	bool PrepareSession(const std::string&strFilename,const std::string&strUser="",const std::string&strPassHash="");
	bool RecoverSession(const std::string&strFilename);
	bool SaveSession(const std::string&strFilename);
	static bool Decode(const std::string&strInput,std::string&strOut);
};

#endif
