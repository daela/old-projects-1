#include "df_web.hpp"

df_web::df_web(const std::string&base_url)
: strSessionKey(""), strClanID("")
{
	SetBaseURL(base_url);
}
bool df_web::SetBaseURL(const std::string&base_url)
{
	strBaseURL=base_url;
	// append a question mark if there isn't one already
	if (!strBaseURL.empty() && *strBaseURL.rbegin() != '?')
		strBaseURL += '?';
	return !strBaseURL.empty();
}

std::string df_web::time_string()
{
	return numeric_string(time(NULL));
}
std::string df_web::sha1sum(const std::string&strData)
{
	SHA1 checksum;
	checksum.Input(strData.c_str(),strData.size());

	unsigned int digest[5];
	checksum.Result(digest);

	char alphabet[]="0123456789ABCDEF";
	std::string strOutput;
	for (int i=0;i<5;++i)
	{
		strOutput += alphabet[(digest[i]&0xF0000000u) >> 28];
		strOutput += alphabet[(digest[i]&0x0F000000u) >> 24];
		strOutput += alphabet[(digest[i]&0x00F00000u) >> 20];
		strOutput += alphabet[(digest[i]&0x000F0000u) >> 16];
		strOutput += alphabet[(digest[i]&0x0000F000u) >> 12];
		strOutput += alphabet[(digest[i]&0x00000F00u) >> 8];
		strOutput += alphabet[(digest[i]&0x000000F0u) >> 4];
		strOutput += alphabet[(digest[i]&0x0000000Fu)];
	}

    return strOutput;
}

size_t df_web::StringAppendCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	if (!data) throw std::runtime_error("No string passed to callback data!");

	reinterpret_cast<std::string*>(data)->append((const char*)ptr,size*nmemb);

	return (size_t)(size * nmemb);
}

bool df_web::PullPage(const std::string&strAddress,std::string&strOutput)
{
	CURL *curl_handle;
	CURLcode res;

	strOutput.clear();

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, strAddress.c_str());
	
	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, StringAppendCallback);

	/* data pointer passed to write callback  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &strOutput);

	/* set a 2 minute timeout */
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 20);

	/* for linux we don't want signals */
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);

	/* some servers don't like requests that are made without a user-agent
	   field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)");
	
	/* get it! */
	res = curl_easy_perform(curl_handle);

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);
	if (res == CURLE_OK)
		return true;

	strOutput.clear();
	return false;
}
bool df_web::PullXmlPage(const std::string&strAddress,std::string&strOutput,TiXmlDocument&objDoc)
{
	// clear out the document
	objDoc=TiXmlDocument();
	if (!PullPage(strAddress,strOutput)) return false;
	objDoc.Parse(strOutput.c_str(), 0, TIXML_ENCODING_UTF8);

	TiXmlElement*ptElement=XML_GetElement(objDoc.RootElement(),"ResultNode.Result.Message");
	if (ptElement != NULL)
	{
		#ifdef DEBUG
		std::cerr << "ERROR: Darkfall Web Message = \"" << NullFix(ptElement->GetText()) << "\"" << std::endl;
		#endif
		return false;
	}
	return true;
}

bool df_web::DecodeXmlDocument(TiXmlDocument&objDoc)
{
	// retrieve the data field that holds the encoded data
	TiXmlElement* ptElement=XML_GetElement(objDoc.RootElement(),"Data");
	if (ptElement == NULL)
	{
		#ifdef DEBUG
		std::cerr << "ERROR no 'Data' element on encoded document" << std::endl;
		#endif
		return false;
	}
	// decode the proximity data
	std::string strDecoded;
	if (!Decode(NullFix(ptElement->GetText()),strDecoded))
	{
		#ifdef DEBUG
		std::cerr << "ERROR decoding xml document" << std::endl;
		#endif
		return false;
	}
	// load the proximity data into the document
	objDoc=TiXmlDocument();
	objDoc.Parse(strDecoded.c_str(), 0, TIXML_ENCODING_UTF8);
	return true;
}
bool df_web::SetBankPolicy(const unsigned int uIndex,std::string&strPage,TiXmlDocument&objDoc)
{
	return df_web::SetPolicy(DF_BANK_POLICY,uIndex,strPage,objDoc);
}	
bool df_web::SetTowerPolicy(const unsigned int uIndex,std::string&strPage,TiXmlDocument&objDoc)
{
	return df_web::SetPolicy(DF_TOWER_POLICY,uIndex,strPage,objDoc);
}	
bool df_web::SetPolicy(df_web::POLICIES ePolicy,const unsigned int uIndex,std::string&strPage,TiXmlDocument&objDoc)
{
	std::string strRequest;
	std::string strPolicySection;
	std::string strOperationType;
	switch (ePolicy)
	{
		case DF_BANK_POLICY:
			strRequest="9";
			strPolicySection="ClanBank";
			strOperationType="1";
			break;
		case DF_TOWER_POLICY:
			strRequest="36";
			strPolicySection="Guard";
			strOperationType="4";
			break;
		default:
			return false;
	}
	std::string strIndex;
	{
		std::stringstream ss;
		ss << uIndex;
		ss >> strIndex;
	}

	if (!PullXmlPage(
			strBaseURL+"&SessionKey="+strSessionKey+"&ClanID="+strClanID+"&WebGateRequest="+strRequest+"&RequestOwner=QETUO&"+strPolicySection+"Policy="+strIndex+"&OperationType="+strOperationType
			,strPage,objDoc))
	{
		#ifdef DEBUG
		std::cerr << "ERROR retrieving " << strPolicySection << " policy page" << std::endl << strPage << std::endl;
		#endif
		return false;
	}
	return true;
}	
bool df_web::GetBankPolicy(std::string&strPolicy,unsigned int&uIndex,std::string&strPage,TiXmlDocument&objDoc)
{
	return GetPolicy(DF_BANK_POLICY,strPolicy,uIndex,strPage,objDoc);
};
bool df_web::GetTowerPolicy(std::string&strPolicy,unsigned int&uIndex,std::string&strPage,TiXmlDocument&objDoc)
{
	return GetPolicy(DF_TOWER_POLICY,strPolicy,uIndex,strPage,objDoc);
};
bool df_web::GetPolicy(df_web::POLICIES ePolicy,std::string&strPolicy,unsigned int&uIndex,std::string&strPage,TiXmlDocument&objDoc)
{
	std::string strRequest;
	std::string strPolicySection;
	switch (ePolicy)
	{
		case DF_BANK_POLICY:
			strRequest="9";
			strPolicySection="ClanBank";
			break;
		case DF_TOWER_POLICY:
			strRequest="36";
			strPolicySection="Guard";
			break;
		default:
			return false;
	}
	if (!PullXmlPage(
			strBaseURL+"&SessionKey="+strSessionKey+"&ClanID="+strClanID+"&WebGateRequest="+strRequest+"&RequestOwner=QETUO"
			,strPage,objDoc))
	{
		#ifdef DEBUG
		std::cerr << "ERROR retrieving " << strPolicySection << " policy page" << std::endl << strPage << std::endl;
		#endif
		return false;
	}
	if (!DecodeXmlDocument(objDoc))
	{
		#ifdef DEBUG
		std::cerr << "ERROR decoding " << strPolicySection << " policy xml document" << std::endl << strPage << std::endl;
		#endif
		return false;
	}
	TiXmlElement*ptElement=objDoc.RootElement();
	if (ptElement)
	{
		// we have to iterate over children because they have some df12367756473234 node with a 'random' name, so we just check them all
		TiXmlElement*ptSibling=ptElement->FirstChildElement();
		while (ptSibling)
		{
			TiXmlElement*ptCurrentPolicy=XML_GetElement(ptSibling,std::string("ObjectData.Current")+strPolicySection+"Policy");

			if (ptCurrentPolicy)
			{
				TiXmlElement*ptPolicyName=XML_GetElement(ptCurrentPolicy,strPolicySection+"PolicyName");
				TiXmlElement*ptPolicyIndex=XML_GetElement(ptCurrentPolicy,strPolicySection+"PolicyIndex");
				if (ptPolicyName && ptPolicyIndex)
				{
					strPolicy=NullFix(ptPolicyName->GetText());
					std::stringstream ss;
					ss << NullFix(ptPolicyIndex->GetText());
					ss >> uIndex;
					return true;
				}
			}
			ptSibling=ptSibling->NextSiblingElement();
		}
		// if we didn't find it, bail
	}
	return false;
	
};

bool df_web::GetProximityPage(const unsigned int&uStartRow,const unsigned int&uPageRows,const unsigned int&uPage,std::string&strPage,TiXmlDocument&objDoc)
{
	if (!PullXmlPage(	strBaseURL+"&SessionKey="+strSessionKey+"&ClanID="+strClanID+"&WebGateRequest=54&RequestOwner=QETUO"
				+"&odo=true&GridCurrentStartRow="+numeric_string(uStartRow)
				+"&GridCurrentPageRows="+numeric_string(uPageRows)
				+"&GridCurrentPage="+numeric_string(uPage)
				+"&NewsReelFilter=1&TimeFilter=0"
			,strPage,objDoc))
	{
		#ifdef DEBUG
		std::cerr << "ERROR retrieving proximity page" << std::endl << strPage << std::endl;
		#endif
		return false;
	}

	if (!DecodeXmlDocument(objDoc))
	{
		#ifdef DEBUG
		std::cerr << "ERROR decoding proximity xml document" << std::endl << strPage << std::endl;
		#endif
		return false;
	}
	return true;
}
bool df_web::Login(const std::string&strUser,const std::string&strPassHash)
{
	TiXmlDocument objDoc;
	TiXmlElement*ptElement;

	std::string strPage;

	std::string strPasswordRCKHash;

	strSessionKey="";
	// make initial login request
	if (!PullXmlPage(strBaseURL+"&WebGateRequest=1&RequestOwner=EXTBRM&_="+time_string()+"&UserName="+strUser+"&RequestOwner=EXTBRM",strPage,objDoc))
	{
		#ifdef DEBUG
		std::cerr << "ERROR: Making initial login request" << std::endl << strPage << std::endl;
		#endif
		return false;
	}
	// get RCK from the reply
	ptElement=XML_GetElement(objDoc.RootElement(),"RCK");
	if (!ptElement)
	{
		#ifdef DEBUG
		std::cerr << "ERROR: no RCK!" << std::endl << strPage << std::endl;
		#endif
		return false;
	}

	// sha1sum the password followed by the rck
	strPasswordRCKHash = sha1sum(strPassHash+NullFix(ptElement->GetText()));
	
	// make the request for the session key
	if (!PullXmlPage(strBaseURL+"&WebGateRequest=2&RequestOwner=EXTBRM&_="+time_string()+"&UserName="+strUser+"&Password="+strPasswordRCKHash,strPage,objDoc))
	{
		#ifdef DEBUG
		std::cerr << "ERROR: Requesting session key" << std::endl << strPage << std::endl;
		#endif
		return false;
	}

	ptElement=XML_GetElement(objDoc.RootElement(),"SessionKey");
	if (!ptElement)
	{
		#ifdef DEBUG
		std::cerr << "ERROR: no SessionKey!" << std::endl << strPage << std::endl;
		#endif
		return false;
	}
	
	strSessionKey=NullFix(ptElement->GetText());
	return true;		
}

bool df_web::GetClanID()
{
	std::string strPage;

	strClanID="";
	// request the page that contains the clan id	
	if (!PullPage(strBaseURL+"&WebGateRequest=1&RequestOwner=QETUO&SessionKey="+strSessionKey,strPage))
	{
		#ifdef DEBUG
		std::cerr << "ERROR: Retrieving clan id" << std::endl << strPage << std::endl;
		#endif
		return false;
	}

	// search the page data for a line containing the clan id
	std::stringstream ss;
	ss << strPage;
	
	std::string strLine;
	std::string::size_type pos,pos2;
	// var ClanID = '12345'
	while (std::getline(ss,strLine))
	{
		pos=strLine.find_first_not_of(" \t",0);
		if (pos != std::string::npos && strLine.size()-pos > 10 && strLine.substr(pos,10) == "var ClanID")
		{
			pos = strLine.find('\'',pos+10);
			if (pos != std::string::npos)
			{
				++pos;
				pos2 = strLine.find('\'',pos);
				if (pos2 == std::string::npos)
					pos2=strLine.size();

				// we got it, store it and return
				strClanID=strLine.substr(pos,pos2-pos);
				return true;
			}
		}
	}
	// we  never found the line, fail
	return false;
}
bool df_web::PrepareSession(const std::string&strFilename,const std::string&strUser,const std::string&strPassHash)
{
	// recover any session we can from the xml file
	if (RecoverSession(strFilename))
		return true;

	// we couldn't recover the session; if we have no credentials, bail
	if (strUser.empty() || strPassHash.empty()) return false;

	bool isDone=false;
	// make two attempts to log in with the provided credentials
	for (unsigned int i=0;!isDone && i<2;++i)
	{
		// isDone is true if we logged in and got a clan id
		isDone=(Login(strUser,strPassHash) && GetClanID());
	}
	// if we suceeded, save our information
	if (isDone)
	{
		SaveSession(strFilename);
	}
	return isDone;
}
// restores the session key and clan id from a xml file
bool df_web::RecoverSession(const std::string&strFilename)
{
	// load in the xml file, or retrieve it from the web
	TiXmlDocument objDoc;
	TiXmlElement*ptKey,*ptClanID;

	if (!strFilename.empty() && objDoc.LoadFile(strFilename.c_str()))
	{
		ptKey=XML_GetElement(objDoc.RootElement(),"Globals.SessionKey");
		ptClanID=XML_GetElement(objDoc.RootElement(),"Globals.ClanID");
		if (ptKey && ptClanID)
		{
			// We have an old session key and clan id, let's try to retrieve the clan ID to make sure
			// we don't assume some erroneous session (and we haven't changed clans), and if we both
			// can acquire the clan id and it matches, then we have recovered the session.
			strSessionKey = NullFix(ptKey->GetText());
			std::string strOldClanID = NullFix(ptClanID->GetText());
			if (GetClanID() && strClanID == strOldClanID)
				return true;
		}
	}
	strSessionKey="";
	strClanID="";
	return false;
}
// saves the session key and clan id to a xml file
bool df_web::SaveSession(const std::string&strFilename)
{
	TiXmlDocument objDoc;
	TiXmlElement * ptGlobals, *ptElement, *ptRoot;
	
	ptGlobals=new TiXmlElement("Globals");

	ptElement=new TiXmlElement("SessionKey");
	ptElement->LinkEndChild(new TiXmlText(strSessionKey.c_str()));
	ptGlobals->LinkEndChild(ptElement);

	ptElement=new TiXmlElement("ClanID");
	ptElement->LinkEndChild(new TiXmlText(strClanID.c_str()));
	ptGlobals->LinkEndChild(ptElement);

	ptRoot = new TiXmlElement("RootNode");
	ptRoot->LinkEndChild(ptGlobals);

	objDoc.LinkEndChild(new TiXmlDeclaration("1.0","",""));
	objDoc.LinkEndChild(ptRoot);
	
	return objDoc.SaveFile(strFilename.c_str());
}

//
// The format is pretty simple.
//
// - There's a comma delimited list of numbers, and numbers preceded by an underscore.
// - If there is no underscore, this number represents the ascii code of a single character
// - If there is an underscore, this number refers to a previous entry in this comma delimited list
//   - Assuming N is a number, _N means
//     - Look at the Nth entry in this list, and get whatever text that entry would produce.
//     - Also, get the first character of whatever text entry (N+1) would produce and add that as well.
//   - Keep in mind
//     - Both entry N and entry (N+1) could also be an underscore preceded entry.
//     - You use _all_ of entry N, but only the first char of entry (N+1)
//     - Entry N could be the current location minus one, and in that case you output entry N and then
//       repeat the first character of entry N (because that's also now the first character of entry N+1)!
//
bool df_web::Decode(const std::string&strInput,std::string&strOut)
{
	// clear the output string
	strOut.clear();
	// reserve the same amount of space as the input string to potentially speed things up;
	strOut.reserve(strInput.size());

	// shorthand
	typedef std::string::size_type size_type;

	// field delimiter
	const char delimiter=',';

	// a mapping of entry indexes to the total number of characters > 1 used by entries up to this point
	std::map<size_type,size_type> mpExtras;	

	// count of number of fields output so far
	size_type uFieldCount=0;

	// Our states obviously.
	enum State { READY, BACKREF, ASCII };

	// Initial values
	State eState=READY; // the state
	size_type uVal=0; // the currently read integer value
	bool isData=false; // whether or not we've found a digit so far
	bool isTrailingWhitespace=false; // whether or not we've found whitespace 

	bool isIgnoringSizeLine=false; // whether or not we're ignoring the hex 'size'(?) line injected into large pages of encoded data
	// notice we also include strInput.size(); we handle '\0' in this loop.
	for (size_type uPos=0;uPos<=strInput.size();++uPos)
	{
		// alias the char for shorthand purposes
		const char& ch = strInput[uPos];

		// if we're in the ready state
		if (eState == READY)
		{
			if (std::isspace(ch))
			{
				// skip whitespace
				continue;
			}
			else if (ch == '_')
			{
				// if it started with a _ then we have a back reference.
				// now we're ready to read in our entry offset
				eState=BACKREF;
				continue;
			}
			else if (!std::isdigit(ch))
			{
				// if it didn't start with a digit, it's INVALID
				return false;
			}

			// it was a digit, so we're ready to read in our ascii value
			eState=ASCII;
		}

		// For the bank policy page, and maybe others, there's data inserted after the numeric data but before the delimiter in one part of the data
		// I'm not sure what this data says; I'm guessing the web interface has a 4K buffer size, and it gets 4k of data then is told how much more
		// data there is (the data is _close_ to the size hex representation of the rest of the encoded data [not decoded]).  Anyway, we don't care,
		// we wrote this character-by-character, so we'll just skip it.

		// LOGIC: after reading a newline, ignore input until we hit a delimiter
		if (!isIgnoringSizeLine)
		{
			if (ch=='\r' || ch == '\n')
			{
				isIgnoringSizeLine=true;
			}

		}
		if (isIgnoringSizeLine)
		{
			if (ch != delimiter && ch != '\0')
				continue;
			isIgnoringSizeLine=false;
		}
			
		// if we're here, then we're reading in the value (ascii or offset)	
		if (std::isdigit(ch))
		{
			// if it's a digit, make sure there wasn't whitespace read in previously
			if (isTrailingWhitespace)
			{
				// no whitespace between digits!
				return false;
			}
			// store this digit in our integer
			uVal = uVal*10+(ch - '0');

			// specify that we have read in integer data
			isData=true;
		}
		else if (ch==delimiter || ch=='\0')
		{
			// if we've reached end of string, or a delimiter

			if (!isData)
			{
				// we do not support empty fields, there must be data!
				return false;
			}
		
			if (eState == BACKREF)
			{
				///////
				// if it was a backref, we have to include all the text produced by entry N (where N == uVal)
				// as well as the first character of entry N+1 (which could be the entry we're outputting now)

				// uVal is the value read.
				// if we had no extra characters output by prior backrefs, we'd expect this to also be the offset
				// for the data, because normal entries output ONE character per ONE entry.
				///////

				if (uVal >= uFieldCount)
					// we can only back-reference past fields, not the current one.
					return false;

				// the number of extras we're aware of existing previous to field N
				size_type uExtras=0;

				// find either an entry for field N, or if it doesn't exist the first one after it that's listed 
				std::map<size_type,size_type>::iterator it=mpExtras.lower_bound(uVal);
				
				// if we actually found the value, then the offset we are referring to was, itself, a backref
				if (it != mpExtras.end() && it->first == uVal)
				{
					// add the total number of extras that were output by this other backref
					// note that now we have the number of extras up to AND INCLUDING entry N
					uExtras += it->second;
				}
				// if we're not at the first element (if empty, that can be considered a != end() check as well)
				if (it != mpExtras.begin())
				{
					// go to the closest previous value
					--it;

					// because if there were no extras ever we could consider uVal as being equivalent to the offset
					// into the string for the field's data, we use uVal as an offset and increment it by
					// the number of extras that precede it so that it refers to the correct location.
					uVal += it->second;

					// if we found entry N in the prior if statement (thereby changing uExtras from 0)
					if (uExtras)
						// we added the total number of extras up to and including entry N
						// but we only want entry N's extra count, not the count "up to" it
						// so remove the previous field's count leaving us with the difference
						uExtras -= it->second;
					
				}
				// output the first character
				// doing this first ensures referencing entry N-1 from entry N works
				// because otherwise when it goes to get the one extra character from
				// the next entry, there won't be one
				strOut += strOut[uVal];
				
				// now output the rest of the characters + one extra
				strOut += strOut.substr(uVal+1,uExtras+1);

				// store how many extra characters were output in total up to this point
				// again leveraging the assumption that for all normal output
				// that because we only output one character, field index == output offset
				mpExtras[uFieldCount]=strOut.size()-uFieldCount-1;
			}
			else if (eState == ASCII)
			{
				// if we read in an ascii value rather than a backref, simply output the char
				strOut += (char)uVal;
			}
		
			// we've processed a field, so increment our field count and reset our loop variables
			++uFieldCount;
			uVal=0;
			isData=false;
			isTrailingWhitespace=false;
			eState = READY;

			// if we considered the end-of-string as a 'delimiter', then we're done; break out of the loop
			if (ch == '\0') break;
		}
		else
		{
			// if we've received data, consider spaces as trailing
			// without isData && being here, "_ \t\r\n123" would falsely be considered a valid backref
			if (isData && std::isspace(ch))
				isTrailingWhitespace=true;
			else
			{
				// invalid character (or, if whitespace, invalid spot for it) 
				return false;
			}
		}
	}
	// if we've made it out of the loop, we've successfully decoded the string
	return true;
}
