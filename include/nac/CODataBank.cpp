#include "CODataBank.h"

bool CODataBank::GetItemAttributes(const COEquipmentData&objData,COAttributes&objAttributes,COAttributes&objCompose) const
{
	objCompose = objAttributes = COAttributes();
	CODB_Type_Map::const_iterator ait = mpItemList.find(objData.strType);
	if (ait == mpItemList.end()) return false;
	CODB_Level_Map::const_iterator bit = ait->second.find(objData.uLevel);
	if (bit == ait->second.end()) return false;
	CODB_Quality_Map::const_iterator cit = bit->second.find(objData.strQuality);
	if (cit == bit->second.end()) return false;
	objAttributes = cit->second;

	//if (objData.bOffHand)
	//	objAttributes.OffHand();

	if (objData.uComposition)
	{
		CODB_ComposeID_Map::const_iterator dit = mpCompositionList.find(objAttributes.GetComposeID());
		if (dit == mpCompositionList.end())
		{
			CODB_Compose_Map::const_iterator eit = dit->second.find(objData.uComposition);
			if (eit != dit->second.end())
				objCompose = eit->second;
		}
		//in case it failed, we still need this value
		objCompose.SetAttribute("Composition",objData.uComposition);
	}
	return true;
}

unsigned char CODataBank::ucDecryptionKey[] = {
	0xAD, 0x6B, 0x4F, 0xFB, 0xDD, 0xB8, 0x0E, 0x09, 0x13, 0x33, 0x8F, 0xF5, 0x43, 0x09, 0x15, 0x88,
	0x5D, 0x80, 0xA3, 0x45, 0x2D, 0x42, 0x08, 0x56, 0x80, 0xF8, 0x19, 0xC5, 0x88, 0x1B, 0x3E, 0xEF,
	0x81, 0x07, 0x30, 0x36, 0x95, 0x52, 0x00, 0xF7, 0xFD, 0x5B, 0x5C, 0xBC, 0x6A, 0x26, 0x0E, 0xB2,
	0xA3, 0x67, 0xC5, 0x5D, 0x6F, 0xDC, 0x18, 0x8A, 0xB5, 0xE0, 0xC8, 0x85, 0xE2, 0x3E, 0x45, 0x8D,
	0x8B, 0x43, 0x74, 0x85, 0x54, 0x17, 0xB0, 0xEC, 0x10, 0x4D, 0x0F, 0x0F, 0x29, 0xB8, 0xE6, 0x7D,
	0x42, 0x80, 0x8F, 0xBC, 0x1C, 0x76, 0x69, 0x3A, 0xB6, 0xA5, 0x21, 0x86, 0xB9, 0x29, 0x30, 0xC0,
	0x12, 0x45, 0xA5, 0x4F, 0xE1, 0xAF, 0x25, 0xD1, 0x92, 0x2E, 0x30, 0x58, 0x49, 0x67, 0xA5, 0xD3,
	0x84, 0xF4, 0x89, 0xCA, 0xFC, 0xB7, 0x04, 0x4F, 0xCC, 0x6E, 0xAC, 0x31, 0xD4, 0x87, 0x07, 0x72
};
bool CODataBank::ReadItemFile(const std::string&strFile)
{
	std::ifstream ifs(strFile.c_str(),std::ios::binary);
	mpItemList.clear();
	if (!ifs) return false;
	unsigned char iOffset = 0;;
	std::stringstream ss;
	COAttributes objAttributes;

	for (unsigned char chValue;ifs.read((char*)&chValue,1);iOffset=(iOffset+1)%sizeof(ucDecryptionKey))
	{
		chValue ^= ucDecryptionKey[iOffset];
		chValue = (chValue<<(8-(iOffset%8))) | (chValue>>(iOffset%8));
		if (chValue == '\r')
			continue;
		if (chValue == '\n')
		{
			if (objAttributes.ReadItemLine(ss))
			{
				std::string strType = objAttributes.GetCategory();
				if (!strType.empty())
				{
//					if (strType == "Boots")
//						std::cout << objAttributes.GetAttribute("ItemID") << std::endl;
					//std::cout << "Adding: " << objAttributes.GetQuality() << " " << objAttributes.GetAttribute("LevelReq") << " " << strType << std::endl;
					mpItemList[strType][objAttributes.GetAttribute("LevelReq")][objAttributes.GetQuality()] = objAttributes;

				}
			}
			ss.clear();
			ss.str("");
		}
		else
		{
			ss << chValue;
		}
	}
	ifs.close();
	return true;
}
bool CODataBank::ReadComposeFile(const std::string&strFile)
{
	std::ifstream ifs(strFile.c_str(),std::ios::binary);
	mpCompositionList.clear();
	if (!ifs) return false;
	COAttributes objAttributes;
	while (objAttributes.ReadComposeLine(ifs))
	{
		mpCompositionList[objAttributes.GetAttribute("ComposeID")][objAttributes.GetAttribute("Composition")] = objAttributes;
	}
	ifs.close();
	return true;
}
