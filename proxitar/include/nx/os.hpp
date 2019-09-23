#ifndef INCLUDED_NX_OS_HPP
#define INCLUDED_NX_OS_HPP

// TOOLCHAIN MACROS 
#if defined(__GNUC__)
	#define TC_GCC_ 1
	#define NX_ALIGN_TO(bytes) __attribute__((aligned(bytes)))
        #define NX_MAY_ALIAS __attribute__((__may_alias__))
#elif _MSC_VER > 1300 // .net 2002+
	#define TC_VC_ 1
	#define NX_ALIGN_TO(bytes) __declspec( align( bytes ) )
        // NX_MAY_ALIAS - NOT DEFINING IT FOR NON-GCC, NOT EVEN EMPTILY.  We want a compilation error, or for you to #ifdef it!
#endif

// Detect C++0X; 1 for full support, 0 for experimental (but defined)
#if __cplusplus > 199711L 
	#define TC_CPP0X_ 1
#elif defined(__GXX_EXPERIMENTAL_CXX0X__)
	#define TC_CPP0X_ 0
#endif

// OS MACROS
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	#define OS_WINDOWS_ 1
#elif defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__) || defined(LINUX)
	#define OS_LINUX_ 1
#elif defined(__APPLE__)
	#define OS_MAC_ 1
#elif defined (__SVR4) && defined (__sun)
	#define OS_SOLARIS_ 1
#else
	#define OS_OTHER_ 1
#endif

// OS SPECIFIC INITIALIZATION
#if defined(OS_WINDOWS_)
	#define WINVER 0x0500
	#define _WIN32_WINNT 0x0501
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
	#ifndef PATH_MAX
		#define PATH_MAX MAX_PATH
	#endif
#else
	#include <limits.h>
	#define MAX_PATH PATH_MAX
#endif

#endif
