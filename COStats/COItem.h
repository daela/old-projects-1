#ifndef __COITEM_H__
#define __COITEM_H__

#include <string>
#include <fstream>
#include <sstream>
#include "COStats.h"
class COItem
{
	private:
		unsigned long uIgnored[18];
	public:
		unsigned long uLevelReq;
		unsigned long uItemID;
		std::string strItemName;
		unsigned long uClassID;
		unsigned long uProficiencyReq;
		unsigned long uStrengthReq;
		unsigned long uAgilityReq;
		unsigned long uTradable;
		unsigned long uPrice;
		unsigned long uRange;
		COStats objStats;

		std::string strTypeDesc;
		std::string strDesc;

		std::string GetQuality(void) const;
		std::string GetType(void) const;
		unsigned long GetComposeID(void) const;
		friend std::istream&operator>>(std::istream&,COItem&);
};

#endif
