#include "COItemAdd.h"
bool COItemAdd::GetComposeData(unsigned long uComposeID,unsigned long uComposition,COStats&objStats) const
{
	COStats_ID_Map::const_iterator nit = mpComposeList.find(uComposeID);
	if (nit == mpComposeList.end()) return false;
	COStats_Compose_Map::const_iterator mit = nit->second.find(uComposition);
	if (mit == nit->second.end()) return false;
	objStats = mit->second;
	return true;
}
bool COItemAdd::Read(std::istream&ifs)
{
	COStats objStats;
	unsigned long uComposeID;
	unsigned long uComposition;
	objStats.uAgility = 0;
	objStats.uMana = 0;
	objStats.uMagicDefensePCT = 0;
	while (ifs >> uComposeID >> uComposition >> objStats.uHealth >> objStats.uAttack[0] >> objStats.uAttack[1]
		    >> objStats.uDefense >> objStats.uMagicAttack >> objStats.uMagicDefense >> objStats.uAccuracy
			>> objStats.uDodge)
	{
		mpComposeList[uComposeID][uComposition] = objStats;
	}
	return true;
}
