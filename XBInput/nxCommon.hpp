#ifndef NXCOMMON_HPP_INCLUDED
#define NXCOMMON_HPP_INCLUDED

#include <stdexcept>
#include <iostream>

#define WINVER 0x0500
#define _WIN32_WINNT 0x0501
#include <windows.h>

// Error routines
#define NX_STRINGIFY(x) #x
#define NX_TOSTRING(x) NX_STRINGIFY(x)
// Not the most efficient macro ever written, but it only occurs if there's an error anyway.
#define NX_NOPATHERR(x,e) (std::string(x).substr(std::string(x).find_last_of("\\/")+1) + (e))
#define NX_ERROR(e) (NX_NOPATHERR(__FILE__ "(" NX_TOSTRING(__LINE__) "): ",(e)))
#define THROWERROR(e) (throw std::runtime_error(NX_ERROR(e)))

typedef unsigned long nxInputId;
typedef unsigned char nxDeviceId;
typedef unsigned char nxExtendedId;
enum nxInputType { NX_KEYBOARD=0, NX_MOUSE, NX_XINPUT, NX_DINPUT };

// Call at start of program with the hInstance
// Retrieve the HINSTANCE by passing NULL (or no parameter) at a later time
inline HINSTANCE nxInstanceHandle(HINSTANCE hInst=NULL)
{
	static HINSTANCE hInstance = NULL;
	if (!hInst) return hInstance;
	hInstance = hInst;
	return hInstance;
}

inline void DebugMessage(const std::string&str)
{
	#ifdef DEBUG
	std::cout << str << std::endl;
	#else
	MessageBox(NULL,str.c_str(),"Debug Message",MB_OK);
	#endif
}


#endif // NXCOMMON_HPP_INCLUDED
