#ifndef __CODATABANK_H__
#define __CODATABANK_H__

#include <map>
#include <string>
#include <sstream>
#include "COAttributes.h"
#include "COEquipmentData.h"

class CODataBank
{
	static unsigned char ucDecryptionKey[];

	typedef std::map<std::string,COAttributes> CODB_Quality_Map;
	typedef std::map<unsigned long,CODB_Quality_Map> CODB_Level_Map;
	typedef std::map<std::string,CODB_Level_Map> CODB_Type_Map;
	typedef std::map<unsigned long,COAttributes> CODB_Compose_Map;
	typedef std::map<unsigned long,CODB_Compose_Map> CODB_ComposeID_Map;
	CODB_Type_Map mpItemList;
	CODB_ComposeID_Map mpCompositionList;

	public:
		bool ReadItemFile(const std::string&);
		bool ReadComposeFile(const std::string&);

		bool GetItemAttributes(const COEquipmentData&,COAttributes&,COAttributes&) const;
};

#endif
