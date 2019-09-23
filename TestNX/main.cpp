#include <nx/common.hpp>
#include <nx/endian.hpp>
#include <nx/memory.hpp>
#include <nx/number.hpp>
#include <iostream>
#include <sstream>
/*
	Missing *, /, *=, /=
	Rotate left, rotate right
	Print decimal.
	Missing <, <=, >, >=, ==, !=




*/

namespace nx
{
	template<class T> class nxBigNum
	{
		public:
		T*ptMemory;

		const nxUChar uMod;
		const nxUChar uSize;
		const nxMem bEndian;

		nxBigNum(const nxULong&size=sizeof(T),const nxMem&endian=nxEndian::Native)
		: uMod(size%sizeof(T)),
		  uSize((size+uMod)/sizeof(T)),
		  bEndian(endian)
		{
			ptMemory = new T[uSize];
		}
		~nxBigNum()
		{
			delete[] ptMemory;
		}
		inline void ZeroHigh(void) { if (uSize) ptMemory[uSize-1] &= (((T)~0u) >> (uMod*8u)); }
		inline nxULong Size(void) const {
			if (uMod)
				if (uSize)
					return ((nxULong)uSize-1)*sizeof(T)+uMod;
				else
					return uMod;
			else
				return ((nxULong)uSize)*sizeof(T);
		}

		nxBigNum& operator=(const nxULong&rhs)
		{
			nxULong uVal = rhs;
			// If you get a warning for right shift count being greater than the width of the type
			// that will only happen if nxULong is the same size as T, and it's not an error.
			// I have addressed the problem with the if. IGNORE THE WARNING!
			if (sizeof(T) < sizeof(nxULong))
				for (nxUChar uI = 0;uI<uSize;++uI,uVal >>= (sizeof(T)*8u))
					ptMemory[uI] = (T)uVal;
			else
				for (nxUChar uI = 0;uI<uSize;++uI,uVal=0)
					ptMemory[uI] = (T)uVal;
			return *this;
		}
		T Digit(const nxUChar&uIndex) const
		{
			return (uIndex < uSize?ptMemory[uIndex]:0u);
		}
		nxBigNum& operator+=(const nxBigNum&rhs)
		{
			bool bCarry = false;
			for (nxUChar uI = 0;uI<uSize;++uI)
			{
				T & uL	= ptMemory[uI];
				const T uR	= rhs.Digit(uI);
				if (bCarry)
				{
					uL += uR + 1;
					if (uL > uR)
						bCarry = false;
				}
				else
				{
					uL += uR;
					if (uL < uR)
						bCarry = true;
				}
			}
			ZeroHigh();
			return *this;
		}
		nxBigNum& operator<<=(const nxULong&uVal)
		{
			if (uVal)
			{
				if ((uVal/8u/sizeof(T)) > uSize)
				{
					for (nxMem uI=0;uI<uSize;++uI)
						ptMemory[uI] = 0u;
				}
				else
				{
					nxULong uDiff = uVal%(sizeof(T)*8);	// Bits in excess of a whole byte
					nxUChar uI = uSize-1;
					// Shift over the current byte, or it with the high bits of the next lower byte
					for (nxMem uJ=uI-(uVal/8u/sizeof(T));uJ;--uI,--uJ)
						ptMemory[uI] = (ptMemory[uJ] << uDiff) | (ptMemory[uJ-1] >> (sizeof(T)-uDiff));
					// There's no next-lower byte, so, simplify
					ptMemory[uI] = (ptMemory[0] << uDiff);
					// Zero the bytes that were entirely shifted in from zeroes
					//while (uI)
					//	ptMemory[uI] = 0u;
					ZeroHigh();
				}
			}
			return *this;
		}
		nxBigNum& rotateLeft(const nxULong&uVal)
		{
			// uVal = 9
			// uDiff = 1
			// uBlocks = uVal/8u/sizeof(T) = 1
			//
			// 12345678 9ABCDEFG HIJKLMNO shifted right... (from right to left, swap bytes until the done.
			// HIJKLMNO 12345678 9ABCDEFG

			// 12345678 9ABCDEFG HIJKLMNO shifted left... (from left to right, swap bytes until the done.
			// 9ABCDEFG HIJKLMNO 12345678

			//
			nxULong uDiff = uVal % (sizeof(T)*8);
			nxULong uBytes = (uVal/8u) % uSize;



		}
		//COPY AND PASTE JOB, THIS ISN'T RIGHT
		nxBigNum& operator>>=(const nxULong&uVal)
		{
			if (uVal)
			{
				if ((uVal/sizeof(T)) > uSize)
				{
					for (nxMem uI=0;uI<uSize;++uI)
						ptMemory[uI] = 0u;
				}
				else
				{
					nxULong uDiff = uVal%(sizeof(T)*8);	// Bits in excess of a whole byte
					nxUChar uI = 0;
					nxMem uMax = uSize-1;

					// uVal/sizeof(T) is how many whole blocks are shifted
					for (nxUChar uJ = uI + (uVal/sizeof(T));uJ < uMax;++uI,++uJ)
					// Shift over the current byte, or it with the low bits of the next higher byte
					for (nxMem uJ=uI+(uVal/sizeof(T));uJ < uMax;++uI,++uJ)
						ptMemory[uI] = (ptMemory[uJ] >> uDiff) | (ptMemory[uJ+1] << (sizeof(T)-uDiff));
					// There's no higher byte, so, simplify
					ptMemory[uI] = (ptMemory[uMax] >> uDiff);
					// Zero the bytes that were entirely shifted in from zeroes
					while (uI < uMax)
						ptMemory[++uI] = 0u;
				}
			}
			return *this;
		}

		friend std::ostream&operator<<(std::ostream&os,const nxBigNum&rhs)
		{
			if (rhs.uSize)
			{
				nxUChar uI, uJ, uMod = (rhs.uMod?rhs.uMod:sizeof(T));
				if (rhs.bEndian == nxEndian::Little)
				{
					for (uI = 0;uI < (rhs.uSize-1);++uI)
						for (uJ=0;uJ < sizeof(T);++uJ)
							os.put(rhs.ptMemory[uI] >> (8*uJ));
					for (uJ=0;uJ < uMod;++uJ)
						os.put(rhs.ptMemory[uI] >> (8*uJ));
				}
				else
				{
					for (uI = rhs.uSize-1,uJ=uMod;uJ--;)
						os.put(rhs.ptMemory[uI] >> (8*uJ));
					while (uI--)
						for (uJ=sizeof(T);uJ--;)
							os.put(rhs.ptMemory[uI] >> (8*uJ));
				}
			}
			return os;
		}
		friend std::istream&operator>>(std::istream&is,nxBigNum&rhs)
		{
			if (rhs.uSize)
			{
				nxUChar uI, uJ, uMod = (rhs.uMod?rhs.uMod:sizeof(T));
				if (rhs.bEndian == nxEndian::Little)
				{
					for (rhs.ptMemory[uI]=0,uI=0;uI < rhs.uSize-1;++uI)
						for (uJ = 0;uJ < sizeof(T);++uJ)
							rhs.ptMemory[uI] |= ((T)((nxUChar)is.get())) << (8*uJ);
					for (rhs.ptMemory[uI]=0,uJ=0;uJ < uMod;++uJ)
						rhs.ptMemory[uI] |= ((T)((nxUChar)is.get())) << (8*uJ);
				}
				else
				{
					for (uI=rhs.uSize-1,rhs.ptMemory[uI] = 0,uJ=uMod;uJ--;)
						rhs.ptMemory[uI] |= ((T)((nxUChar)is.get())) << (8*uJ);
					while (uI--)
						for (rhs.ptMemory[uI] = 0,uJ=sizeof(T);uJ--;)
							rhs.ptMemory[uI] |= ((T)((nxUChar)is.get())) << (8*uJ);
				}
			}
			return is;
		}



	};
}
int main(void)
{
	nx::nxBigNum<unsigned long> blargh(4,nx::nxEndian::Little);
	nx::nxBigNum<unsigned long> blah(4,nx::nxEndian::Little);
	std::stringstream ss;
	blargh = 0x44434241;
	std::cout << "Blargh: " << blargh << std::endl;
	blargh <<= 10;
	blargh <<= 4;
	blargh <<= 2;

	std::cout << "Blargh: " << blargh << std::endl;
	return 0;
}
