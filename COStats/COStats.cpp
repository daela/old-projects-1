#include "COStats.h"

COStats&COStats::operator+=(const COStats&rhs)
{
	uAttack[0] += rhs.uAttack[0];
	uAttack[1] += rhs.uAttack[1];
	uDefense += rhs.uDefense;
	uAgility += rhs.uAgility;
	uDodge += rhs.uDodge;
	uMagicAttack += rhs.uMagicAttack;
	uMagicDefense += rhs.uMagicDefense;
	uMagicDefensePCT += rhs.uMagicDefensePCT;
	uHealth += rhs.uHealth;
	uMana += rhs.uMana;
	uAccuracy += rhs.uAccuracy;
	return *this;
}
COStats COStats::operator+(const COStats&rhs) const
{
	COStats objRet = *this;
	return (objRet += rhs);
}
COStats&COStats::operator-=(const COStats&rhs)
{
	uAttack[0] -= rhs.uAttack[0];
	uAttack[1] -= rhs.uAttack[1];
	uDefense -= rhs.uDefense;
	uAgility -= rhs.uAgility;
	uDodge -= rhs.uDodge;
	uMagicAttack -= rhs.uMagicAttack;
	uMagicDefense -= rhs.uMagicDefense;
	uMagicDefensePCT -= rhs.uMagicDefensePCT;
	uHealth -= rhs.uHealth;
	uMana -= rhs.uMana;
	uAccuracy -= rhs.uAccuracy;
	return *this;
}
COStats COStats::operator-(const COStats&rhs) const
{
	COStats objRet = *this;
	return (objRet -= rhs);
}
COStats COStats::OffHand(void) const
{
	COStats objRet = *this;
	objRet.uAttack[0] >>= 1;
	objRet.uAttack[1] >>= 1;
	objRet.uDefense >>= 1;
	objRet.uAgility >>= 1;
	objRet.uDodge >>= 1;
	objRet.uMagicAttack >>= 1;
	objRet.uMagicDefense >>= 1;
	objRet.uHealth >>= 1;
	objRet.uMana >>= 1;
	objRet.uAccuracy >>= 1;
	//uMagicDefensePCT doesn't get halfed!
	return objRet;
}
