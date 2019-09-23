#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#include "type.hpp"
inline int nxMemCmp(const nxMem * ptrA,const nxMem * ptrB,nxULong uSize)
{
	while (uSize--)
		if (*ptrB < *ptrA) return 1;
		else if (*ptrA++ < *ptrB++) return -1;
	return 0;
}
inline void nxMemCpy(nxMem * ptrDest,const nxMem * ptrSrc,nxULong uSize)
{
	while (uSize--)
		*ptrDest++ = *ptrSrc++;
}
inline void nxMemCpyR(nxMem * ptrDest,const nxMem * ptrSrc,nxULong uSize)
{
	ptrDest += uSize;
	while (uSize--)
		*--ptrDest = *ptrSrc++;
}
inline void nxMemRev(nxMem * ptrLoc,const nxULong &uSize)
{
	nxMem *ptrEnd = ptrLoc;
	ptrEnd += uSize;
	while (ptrLoc < --ptrEnd)
		*ptrEnd ^= *ptrLoc ^= *ptrEnd ^= *ptrLoc++;
}
inline void nxMemSet(nxMem * ptrDest,const nxMem& val,nxULong uCount)
{
	while (uCount--)
		*ptrDest++ = val;
}
// Read a 32-bit little endian value into an integral type.
template<class T> T nxLMemToNum32(nxMemInt32 ptBuf)
{
	return (*ptBuf) | (*++ptBuf << 8) | (*++ptBuf << 16) | (*++ptBuf << 24);
}
// Read a 32-bit big endian value into an integral type.
template<class T> T nxBMemToNum32(nxMemInt32 ptBuf)
{
	return (*ptBuf << 24) | (*++ptBuf << 16) | (*++ptBuf << 8) | (*++ptBuf);
}
// Detect system endianness
inline bool nxLittleEndian(void)
{
	static unsigned short uShort=1;
	return *((nxMem*)(&uShort));
}

#endif // __MEMORY_HPP__
