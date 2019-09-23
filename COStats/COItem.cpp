#include "COItem.h"


std::string COItem::GetType(void) const
{
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
std::string COItem::GetQuality(void) const
{
	switch (uItemID%10)
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
std::istream&operator>>(std::istream&ifs,COItem&obj)
{
	obj.objStats.uAccuracy = 0;
	obj.objStats.uMagicDefense = 0;
	return (ifs >> obj.uItemID >> obj.strItemName >> obj.uClassID >> obj.uProficiencyReq >> obj.uLevelReq >> obj.uIgnored[0]
			    >> obj.uStrengthReq >> obj.uAgilityReq >> obj.uIgnored[1] >> obj.uIgnored[2] >> obj.uTradable >> obj.uIgnored[3]
				>> obj.uPrice >> obj.uIgnored[4] >> obj.objStats.uAttack[1] >> obj.objStats.uAttack[0] >> obj.objStats.uDefense
				>> obj.objStats.uAgility >> obj.objStats.uDodge	>> obj.objStats.uHealth >> obj.objStats.uMana >> obj.uIgnored[5]
				>> obj.uIgnored[6] >> obj.uIgnored[7] >> obj.uIgnored[8] >> obj.uIgnored[9] >> obj.uIgnored[10] >> obj.uIgnored[11]
				>> obj.uIgnored[12] >> obj.objStats.uMagicAttack >> obj.objStats.uMagicDefensePCT >> obj.uRange >> obj.uIgnored[13]
				>> obj.uIgnored[14] >> obj.uIgnored[15] >> obj.uIgnored[16] >> obj.uIgnored[17] >> obj.strTypeDesc >> obj.strDesc);
}
