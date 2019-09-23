#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include "COItemType.h"
#include "COItem.h"
#include "COItemAdd.h"

//off-hand doesn't half
//only halved agility, attack, and defense.. doesn't half magic-defense
class COAttributes
{
	typedef std::map<std::string,unsigned long> AttributeMap;
	typedef std::map<std::string,std::string> StringMap;


	AttributeMap mpAttributes;
	StringMap mpStrings;
	inline void HalfAttribute(const std::string&strAttribute)
	{
		SetAttribute(strAttribute,GetAttribute(strAttribute)>>1);
	}
	inline void OutputAttribute(std::ostream&ofs,const std::string&strColor,const std::string&strAttribute,const std::string&strLabel="",const std::string&strPrequel=" ",const std::string&strSequel="",const std::string&strExtraAttribute="") const
	{
		unsigned long uTemp,uTemp2;
		uTemp = GetAttribute(strAttribute);
		uTemp2 = GetAttribute(strExtraAttribute);
		if (uTemp || uTemp2)
		{
			ofs << "<font color=\"#" << strColor << "\">" << (strLabel.empty()?strAttribute:strLabel) << ":" << strPrequel << uTemp << strSequel;
			if (!strExtraAttribute.empty())
				ofs << uTemp2;
			ofs << "</font><br>";
		}
	}
	static inline void BeginOutput(std::ostream&ofs,const std::string&strColor)
	{
		ofs << "<font color=\"#" << strColor << "\">";
	}
	static inline void EndOutput(std::ostream&ofs)
	{
		ofs << "</font></br>";
	}
	public:

		std::string GetString(const std::string&) const;
		void SetString(const std::string&,const std::string&);
		unsigned long GetAttribute(const std::string&) const;
		void SetAttribute(const std::string&,const unsigned long&);
		COAttributes&operator+=(COAttributes);
		COAttributes operator+(const COAttributes&);
		bool ReadItemLine(std::istream&);
		bool ReadComposeLine(std::istream&);
		std::string GetCategory(void) const;
		std::string GetQuality(void) const;
		unsigned long GetComposeID(void) const;
		void OffHand(void);

		static void Output(std::ostream&,COAttributes&,COAttributes&);
		void OutputItem(std::ostream&) const;
};
void COAttributes::Output(std::ostream&ofs,COAttributes&objAttributes,COAttributes&objCompose)
{
	unsigned long uTemp,uTemp2;
	//std::string strTemp;
	BeginOutput(ofs,"ebe57f");
	ofs << objAttributes.GetQuality() << objAttributes.GetString("Name");
	if (uTemp = objCompose.GetAttribute("Composition"))
		ofs << "(+" << uTemp << ")";
	//if (!(strTemp = objAttributes.GetString("Type")).empty())
	//	ofs << "(" << strTemp << ")";
	EndOutput(ofs);

	//stats (needs prof!)
	objAttributes.OutputAttribute(ofs,"ffffff","LevelReq","Level");
	objAttributes.OutputAttribute(ofs,"ffffff","StrengthReq","Strength");
	objAttributes.OutputAttribute(ofs,"ffffff","AgilityReq","Agility");
	objAttributes.OutputAttribute(ofs,"ffffff","Attack_Min","Attack"," ","-","Attack_Max");
	objAttributes.OutputAttribute(ofs,"ffffff","Agility",""," +");
	objAttributes.OutputAttribute(ofs,"8ccefd","MagicAttack","Magic Atk"," +");
	objAttributes.OutputAttribute(ofs,"ffffff","Defense");
	objAttributes.OutputAttribute(ofs,"8ccefd","MagicDefensePCT","Magic Def"," +","%");

	//composition
	objCompose.OutputAttribute(ofs,"c501e7","Defense","Ph-Defense","+");
	objCompose.OutputAttribute(ofs,"c501e7","MagicDefense","M-Defense","+");
	objCompose.OutputAttribute(ofs,"c501e7","Accuracy","","+");
	uTemp = objCompose.GetAttribute("Attack_Min");
	uTemp2 = objCompose.GetAttribute("Attack_Max");
	if (uTemp || uTemp2)
	{
		BeginOutput(ofs,"c501e7");
		ofs << "Ph-Damage:+" << uTemp;
		if (uTemp != uTemp2)
			ofs << "-" << uTemp2;
		EndOutput(ofs);
	}



}
void COAttributes::OffHand(void)
{
	HalfAttribute("Attack_Min");
	HalfAttribute("Attack_Max");
	HalfAttribute("Defense");
	HalfAttribute("Agility");
	HalfAttribute("Dodge");
	HalfAttribute("MagicAttack");
	HalfAttribute("MagicDefense");
	HalfAttribute("Health");
	HalfAttribute("Mana");
	HalfAttribute("Accuracy");
}
unsigned long COAttributes::GetComposeID(void) const
{
	unsigned long uItemID = GetAttribute("ItemID");
	std::string strItemID;
	{
		std::stringstream ss;
		ss << uItemID;
		strItemID=ss.str();
	}
	if (strItemID.size() >= 6)
	{
		if ((uItemID >= 900000 && uItemID <= 900999) || (uItemID >= 111303 && uItemID <= 118999) || (uItemID >= 130203 && uItemID <= 139999)) //Shields, Helms, Armors
		{
			strItemID[strItemID.size()-3] = '0';
			strItemID[strItemID.size()-1] = '0';
		}

		else if ((uItemID >= 410000 && uItemID <= 490400) || (uItemID >= 510000 && uItemID <= 580400)) //1/Handers, 2/Handers
		{
			strItemID[0] = strItemID[1] = strItemID[2] = '5';
		}
		else if (!((uItemID >= 150000 && uItemID <= 160250) || (uItemID >= 500000 && uItemID <= 500400) || (uItemID >= 120003 && uItemID <= 121249))) //BackSwords, Bows, Necklaces/Bags
		{
			return 0;
		}
		strItemID[strItemID.size()-1] = '0';
		std::stringstream ss;
		ss << strItemID;
		unsigned long uRet;
		ss >> uRet;
		return uRet;
	}
	return 0;
}
std::string COAttributes::GetQuality(void) const
{
	switch (GetAttribute("ItemID")%10)
	{
		case 0: return "Fixed";
		case 3: return "Normal_1";
		case 4: return "Normal_2";
		case 5: return "Normal_3";
		case 6: return "Refined";
		case 7: return "Unique";
		case 8: return "Elite";
		case 9: return "Super";
	}
	return "";
}
std::string COAttributes::GetCategory(void) const
{
	std::string strTypeDesc = GetString("Type");
	if (strTypeDesc == "Warrior`sHelmet") return "Helmet";
    if (strTypeDesc == "Warrior`sArmor") return "Armor";
    if (strTypeDesc == "Shield") return "Shield";
    if (strTypeDesc == "Trojan`sCoronet") return "Coronet";
    if (strTypeDesc == "Trojan`sArmor") return "Mail";
    if (strTypeDesc == "Taoist`sCap") return "Cap";
    if (strTypeDesc == "Taoist`sRobe") return "Robe";
    if (strTypeDesc == "Taoist`sBag") return "Bag";
    if (strTypeDesc == "Taoist`sBracelet") return "Bracelet";
    if (strTypeDesc == "Archer`sCoat") return "Coat";
    if (strTypeDesc == "Archer`sHat") return "Hat";

    if (strTypeDesc == "Blade") return "Blade";
    if (strTypeDesc == "Taoist`sBackSword") return "Backsword";
    if (strTypeDesc == "Hook") return "Hook";
    if (strTypeDesc == "Whip") return "Whip";
    if (strTypeDesc == "Axe") return "Axe";
    if (strTypeDesc == "Hammer") return "Hammer";
    if (strTypeDesc == "Club") return "Club";
    if (strTypeDesc == "Scepter") return "Scepter";
    if (strTypeDesc == "Dagger") return "Dagger";
    if (strTypeDesc == "Archer`sBow") return "Bow";
    if (strTypeDesc == "Sword") return "Sword";

    if (strTypeDesc == "Glaive") return "Glaive";
    if (strTypeDesc == "PoleAxe") return "PoleAxe";
    if (strTypeDesc == "Longhammer") return "LongHammer";
    if (strTypeDesc == "Spear") return "Spear";
    if (strTypeDesc == "Wand") return "Wand";
    if (strTypeDesc == "Halbert") return "Halbert";
    if (strTypeDesc == "Coat") return "Dress";
    if (strTypeDesc == "Dress") return "Dress";
    if (strTypeDesc == "Necklace") return "Necklace";
    if (strTypeDesc == "Ring") return "Ring";
	if (strTypeDesc == "HeavyRing") return "HeavyRing";
    if (strTypeDesc == "Boots" || strTypeDesc == "Boot") return "Boots";
    if (strTypeDesc == "Earring" || strTypeDesc == "Earrings") return "Earrings";
    return "";
}
bool COAttributes::ReadItemLine(std::istream&ifs)
{
	unsigned long uIgnore;
	mpAttributes.clear();
	mpStrings.clear();
	ifs >> mpAttributes["ItemID"] >> mpStrings["Name"] >> mpAttributes["ClassID"] >> mpAttributes["ProficiencyReq"] >> mpAttributes["LevelReq"] >> uIgnore
		>> mpAttributes["StrengthReq"] >> mpAttributes["AgilityReq"] >> uIgnore >> uIgnore >> mpAttributes["Tradable"] >> uIgnore
		>> mpAttributes["Price"] >> uIgnore >> mpAttributes["Attack_Max"] >> mpAttributes["Attack_Min"] >> mpAttributes["Defense"]
		>> mpAttributes["Agility"] >> mpAttributes["Dodge"] >> mpAttributes["Health"] >> mpAttributes["Mana"] >> uIgnore
		>> uIgnore >> uIgnore >> uIgnore >> uIgnore >> uIgnore >> uIgnore
		>> uIgnore >> mpAttributes["MagicAttack"] >> mpAttributes["MagicDefensePCT"] >> mpAttributes["Range"] >> uIgnore
		>> uIgnore >> uIgnore >> uIgnore >> uIgnore >> mpStrings["Type"] >> mpStrings["Description"];
	if (ifs)
		return true;
	return false;
}
bool COAttributes::ReadComposeLine(std::istream&ifs)
{
	mpAttributes.clear();
	mpStrings.clear(); //attack values could be reversed, but no composes differ on min/max thus far
	ifs >> mpAttributes["ComposeID"] >> mpAttributes["Composition"] >> mpAttributes["Health"] >> mpAttributes["Attack_Min"] >> mpAttributes["Attack_Max"]
		>> mpAttributes["Defense"] >> mpAttributes["MagicAttack"] >> mpAttributes["MagicDefense"] >> mpAttributes["Accuracy"] >> mpAttributes["Dodge"];
	if (ifs)
		return true;
	return false;
}
std::string COAttributes::GetString(const std::string&strAttribute) const
{
	StringMap::const_iterator it = mpStrings.find(strAttribute);
	if (it != mpStrings.end()) return it->second;
	return std::string("");
}
void COAttributes::SetString(const std::string&strAttribute,const std::string&strValue)
{
	if (!strValue.empty())
		mpStrings[strAttribute] = strValue;
	else
	{
		StringMap::iterator it = mpStrings.find(strAttribute);
		if (it != mpStrings.end()) mpStrings.erase(it);
	}
}
unsigned long COAttributes::GetAttribute(const std::string&strAttribute) const
{
	AttributeMap::const_iterator it = mpAttributes.find(strAttribute);
	if (it != mpAttributes.end()) return it->second;
	return 0L;
}
void COAttributes::SetAttribute(const std::string&strAttribute,const unsigned long&uValue)
{
	if (uValue)
		mpAttributes[strAttribute] = uValue;
	else
	{
		AttributeMap::iterator it = mpAttributes.find(strAttribute);
		if (it != mpAttributes.end()) mpAttributes.erase(it);
	}
}
COAttributes COAttributes::operator+(const COAttributes&rhs)
{
	COAttributes objRet = *this;
	objRet += rhs;
	return objRet;
}
COAttributes&COAttributes::operator+=(COAttributes rhs)
{
	//mpStrings.clear();
	for (AttributeMap::iterator it = mpAttributes.begin();it!=mpAttributes.end();++it)
	{
		AttributeMap::iterator nit = rhs.mpAttributes.find(it->first);
		if (nit != rhs.mpAttributes.end())
		{
			it->second += nit->second;
			rhs.mpAttributes.erase(nit);
		}
	}
	//Copy the rest
	std::copy(rhs.mpAttributes.begin(), rhs.mpAttributes.end(), std::inserter( mpAttributes, mpAttributes.begin() ) );
	return *this;
}

class COEquipmentData
{
	public:
		COEquipmentData(std::string strType_="",unsigned long uLevel_=0,std::string strQuality_="",unsigned long uComposition_="",bool bOffHand_=false);
		std::string strType;
		unsigned long uLevel;
		std::string strQuality;
		unsigned long uComposition;
		bool bOffHand;
};
COEquipmentData::COEquipmentData(std::string strType_,unsigned long uLevel_,std::string strQuality_,unsigned long uComposition_,bool bOffHand_)
: strType(strType_), uLevel(uLevel_), strQuality(strQuality_), uComposition(uComposition_), bOffHand(bOffHand_)
{ }
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

	if (objData.bOffHand)
		objAttributes.OffHand();

	if (objData.uComposition)
	{
		CODB_ComposeID_Map::const_iterator dit = mpCompositionList.find(objAttributes.GetComposeID());
		if (dit == mpCompositionList.end()) return false;
		CODB_Compose_Map::const_iterator eit = dit->second.find(objData.uComposition);
		if (eit == dit->second.end()) return false;
		objCompose = eit->second;
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


int main(void)
{
	/*std::ifstream ifsType("itemtype.dat",std::ios::binary);
	std::ifstream ifsAdd("ItemAdd.ini",std::ios::binary);
	if (!ifsType || !ifsAdd) return 1;
	COEquipment objEquipment;
	COItemType objItemType;
	COItemAdd objItemAdd;
	COStats objStats;
	objItemType.Read(ifsType);
	objItemAdd.Read(ifsAdd);


	objEquipment.vcItems.push_back(COEquipmentData("Helmet",120,"Super",12));
	objEquipment.vcItems.push_back(COEquipmentData("Necklace",130,"Super",7));
	objEquipment.vcItems.push_back(COEquipmentData("HeavyRing",127,"Super",8));
	objEquipment.vcItems.push_back(COEquipmentData("Blade",130,"Super",7));
	objEquipment.vcItems.push_back(COEquipmentData("Armor",120,"Super",12));
	objEquipment.vcItems.push_back(COEquipmentData("Shield",110,"Super",12,true));
	objEquipment.vcItems.push_back(COEquipmentData("Boots",129,"Super",7));
	//objEquipment.vcItems.push_back(COEquipmentData("Gourd",15,"Super",7));
	if (!objEquipment.Calculate(objItemType,objItemAdd,objStats))
	{
		std::cout << "Error :(" << std::endl;
		return false;
	}
	std::cout << "Defense: " << objStats.uDefense << std::endl;
	ifsType.close();*/
	CODataBank objDB;
	objDB.ReadItemFile("itemtype.dat");
	objDB.ReadComposeFile("ItemAdd.ini");
	COAttributes objAttributes, objCompose;
	std::cout << "Worked: " << objDB.GetItemAttributes(COEquipmentData("Shield",110,"Super",9),objAttributes,objCompose) << std::endl;
	std::ofstream ofs("test.html");
	ofs << "<body bgcolor=\"#000000\">";
	COAttributes::Output(ofs,objAttributes,objCompose);
	ofs << "</body>";
	ofs.close();

	return 0;
};
