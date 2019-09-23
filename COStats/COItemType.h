#ifndef __COITEMTYPE_H__
#define __COITEMTYPE_H__
#include <map>
#include <vector>
#include <sstream>
#include <string>
#include "COItem.h"
class COItemType
{
	static unsigned char ucDecryptionKey[];
	typedef std::map<std::string,COItem> COItem_Quality_Map;
	typedef std::map<unsigned long,COItem_Quality_Map> COItem_Level_Map;
	typedef std::map<std::string,COItem_Level_Map> COItem_Type_Map;
	COItem_Type_Map mpItemList;
	public:
		bool Read(std::istream&);
		bool GetTypeList(std::vector<std::string>&vcTypes) const;
		bool GetLevelList(std::string strType,std::vector<unsigned long>&vcLevels) const;
		bool GetQualityList(std::string strType, unsigned long uLevel,std::vector<std::string>&vcQualities) const;
		bool GetItem(std::string strType,unsigned long uLevel,std::string strQuality,COItem&obj) const;
};

#endif
