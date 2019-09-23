#ifndef __COSTATS_H__
#define __COSTATS_H__

class COStats
{
	public:
		unsigned long uAttack[2];
		unsigned long uDefense;
		unsigned long uAgility;
		unsigned long uDodge;
		unsigned long uMagicAttack;
		unsigned long uMagicDefense;
		unsigned long uMagicDefensePCT;
		unsigned long uHealth;
		unsigned long uMana;
		unsigned long uAccuracy;

		COStats&operator+=(const COStats&rhs);
		COStats operator+(const COStats&rhs) const;
		COStats&operator-=(const COStats&rhs);
		COStats operator-(const COStats&rhs) const;
		COStats OffHand(void) const;
};


#endif
