#ifndef __NXNUMBER_HPP__
#define __NXNUMBER_HPP__
#include <nx/nxMemory.hpp>
#include <nx/type.hpp>
#include <nx/memory.hpp>

class nxNumber : public nxMemory
{
	private:
		bool bLittleEndian;
	public:
		enum nxEndian { nxBIG_ENDIAN,nxLITTLE_ENDIAN,nxSYSTEM_ENDIAN };
		// nxULong constructor
		nxNumber(const nxULong&uVal,const nxULong &blocksize=sizeof(nxUInt),const nxEndian &endian = nxSYSTEM_ENDIAN);
		// memory constructor
		nxNumber(const nxMem*ptBuf=nxNULL,const nxULong &blocksize=sizeof(nxUInt),const nxEndian &endian = nxSYSTEM_ENDIAN);
		// copy constructor
		nxNumber(const nxNumber&obj);

		// assignment from other object
		nxNumber&operator=(const nxNumber&uVal);
		// assignment from nxULong
		nxNumber&operator=(nxULong uVal);

		// make this object a copy of another
		void copy(const nxNumber& obj);

		// cast to nxULong
		operator nxULong() const;

		// get significant digit
		nxMem&GetSig(const nxULong&uIndex);
		// get significant digit value
		const nxMem GetSigVal(const nxULong&uIndex) const;

		// math routines
		nxNumber&operator+=(const nxNumber&uVal);
		nxNumber&operator-=(const nxNumber&uVal);
		nxNumber operator+(const nxNumber&uVal) const;
		nxNumber operator-(const nxNumber&uVal) const;
		nxNumber&operator++();
		nxNumber&operator--();
		const nxNumber operator++(int);
		const nxNumber operator--(int);
};

#endif //__NXNUMBER_HPP__
