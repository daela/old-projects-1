#include "nxNumber.hpp"

nxNumber::nxNumber(const nxNumber&obj)
: nxMemory(*static_cast<const nxMemory*>(&obj))
{
	bLittleEndian = obj.bLittleEndian;
}
nxNumber::nxNumber(const nxULong&uVal,const nxULong &blocksize,const nxEndian &endian)
: nxMemory(blocksize)
{
	switch (endian)
	{
		case nxLITTLE_ENDIAN:
			bLittleEndian=true; break;
		case nxBIG_ENDIAN:
			bLittleEndian=false; break;
		case nxSYSTEM_ENDIAN:
		default:
			bLittleEndian=nxLittleEndian();
	}
	operator=(uVal);
}
nxNumber::nxNumber(const nxMem*ptBuf,const nxULong &blocksize,const nxEndian &endian)
: nxMemory(blocksize,ptBuf)
{
	switch (endian)
	{
		case nxLITTLE_ENDIAN:
			bLittleEndian=true; break;
		case nxBIG_ENDIAN:
			bLittleEndian=false; break;
		case nxSYSTEM_ENDIAN:
		default:
			bLittleEndian=nxLittleEndian();
	}
}
nxNumber&nxNumber::operator=(const nxNumber&uVal)
{
	if (bLittleEndian)
		for (nxULong uI = 0;uI<uBlockSize;++uI)
			ptMemory[uI] = uVal.GetSigVal(uI);
	else
		for (nxULong uI = uBlockSize;uI--;)
			ptMemory[uI] = uVal.GetSigVal(uBlockSize-1-uI);
	return *this;
}
nxNumber&nxNumber::operator=(nxULong uVal)
{
	if (bLittleEndian)
		for (nxULong uI = 0;uI<uBlockSize;++uI,uVal >>= 8)
			ptMemory[uI] = (nxMem)uVal;
	else
		for (nxULong uI = uBlockSize;uI;uVal >>= 8)
			ptMemory[--uI] = (nxMem)uVal;
	return *this;
}
nxMem&nxNumber::GetSig(const nxULong&uIndex)
{
	if (bLittleEndian)
		return ptMemory[uIndex];
	return ptMemory[uBlockSize-1-uIndex];
}
const nxMem nxNumber::GetSigVal(const nxULong&uIndex) const
{
	if (uIndex < uBlockSize)
	{
		if (bLittleEndian)
			return ptMemory[uIndex];
		return ptMemory[uBlockSize-1-uIndex];
	}
	return 0;
}
void nxNumber::copy(const nxNumber& obj)
{
	static_cast<nxMemory*>(this)->copy(*static_cast<const nxMemory*>(&obj));
	bLittleEndian = obj.bLittleEndian;
}
nxNumber::operator nxULong() const
{
	nxULong uVal = 0;
	if (bLittleEndian)
		for (nxULong uI = (sizeof(nxULong)<uBlockSize?sizeof(nxULong):uBlockSize);uI;)
		{
			uVal <<= 8;
			uVal |= ptMemory[--uI];
		}
	else
		for (nxULong uI = (sizeof(nxULong)>uBlockSize?0:uBlockSize-sizeof(nxULong));uI<uBlockSize;++uI)
		{
			uVal <<= 8;
			uVal |= ptMemory[uI];
		}
	return uVal;
}


nxNumber&nxNumber::operator+=(const nxNumber&uVal)
{
	// nacitar's method
	nxUShort uBuf=0;
	for (nxULong i=0;i < uBlockSize;++i)
	{
		nxMem&chDest = GetSig(i);
		chDest = (nxMem)(uBuf += ((nxUShort)chDest) + ((nxUShort)uVal.GetSigVal(i)));
		uBuf>>=8;
	}
	// Ref: Knuth Vol 2 Ch 4.3.1 p 266 Algorithm A.
	//nxMem uCarry = 0;
	//for (nxULong i=0;i < uBlockSize && i < uVal.uBlockSize;++i)
	//{
	//
	//	nxMem&chDest(GetSig(i));
	//	nxMem&chLoc(uVal.GetSig(i));
	//	chDest += uCarry;
	//	uCarry = (chDest < uCarry?1:0);
	//	chDest += chLoc;
	//	uCarry += (chDest < chLoc?1:0);
	//}
	return *this;
}
nxNumber&nxNumber::operator-=(const nxNumber&uVal)
{
	// Ref: Knuth Vol 2 Ch 4.3.1 p 267 Algorithm S.
	nxMem uCarry=0;

	for (nxULong i=0;i<uBlockSize;++i)
	{
		nxMem&chDest(GetSig(i));
		nxMem chLoc(uVal.GetSigVal(i));
		chDest -= uCarry;
		uCarry = (chDest > (0xFFu - uCarry)?1:0);
		chDest -= chLoc;
		uCarry += (chDest > (0xFFu - chLoc)?1:0);
	}
	return *this;
}
nxNumber nxNumber::operator+(const nxNumber&uVal) const
{
	nxNumber objRet(*this);
	return (objRet += uVal);
}
nxNumber nxNumber::operator-(const nxNumber&uVal) const
{
	nxNumber objRet(*this);
	return (objRet -= uVal);
}
nxNumber&nxNumber::operator++()
{
	for (nxULong uI = 0;uI < uBlockSize;++uI)
	{
		nxMem&ch=GetSig(uI);
		if (ch != 0xFFu)
		{
			++ch;
			break;
		}
		else
			ch = 0;
	}
	return *this;
}
nxNumber&nxNumber::operator--()
{
	// Ref: Knuth Vol 2 Ch 4.3.1 p 267 Algorithm S.

	for (nxULong i=0;i<uBlockSize;++i)
	{
		nxMem&chDest(GetSig(i));
		chDest -= 1;
		if (chDest != 0xFFu)
			break;
	}
	return *this;
}
const nxNumber nxNumber::operator++(int)
{
	nxNumber objVal = *this;
	operator++();
	return objVal;
}
const nxNumber nxNumber::operator--(int)
{
	nxNumber objVal = *this;
	operator--();
	return objVal;
}
