#ifndef __COITEMADD_H__
#define __COITEMADD_H__
#include <map>
#include <fstream>
#include <string>
#include "COStats.h"
class COItemAdd
{
	//compose id -> composition
	typedef std::map<unsigned long,COStats> COStats_Compose_Map;
	typedef std::map<unsigned long,COStats_Compose_Map> COStats_ID_Map;
	COStats_ID_Map mpComposeList;

	public:
		bool Read(std::istream&ifs);
		bool GetComposeData(unsigned long uComposeID,unsigned long uComposition,COStats&objStats) const;
};

#endif
