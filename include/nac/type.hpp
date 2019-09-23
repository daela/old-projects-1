#ifndef __TYPE_HPP__
#define __TYPE_HPP__

#if defined(WIN32) || defined(WIN64) || defined (WINDOWS)
#define NXWINDOWS
#include "win32.hpp"
#endif
// 8-bit type for use when referring to a memory buffer
typedef unsigned char nxMem;
// 32-bit memory buffer
typedef nxMem nxMemInt32[4];
// type that is >= 32 bits
typedef int nxInt32;

typedef int nxInt;
typedef unsigned int nxUInt;

// type that is >= 32 bits unsigned
typedef unsigned int nxUInt32;
// type for large int sizes, unsigned
typedef unsigned long nxULong;
// type for large int sizes
typedef unsigned long nxLong;

typedef short nxShort;
typedef unsigned short nxUShort;

#define nxNULL ((nxMem*)0u)
#endif // __TYPE_HPP__
