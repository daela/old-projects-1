#ifndef NXCOMMON_HPP_INCLUDED
#define NXCOMMON_HPP_INCLUDED

/// OS MACROS
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

/// OS SPECIFIC INITIALIZATION
#if defined(OS_WINDOWS_)
    #define WINVER 0x0500
    #define _WIN32_WINNT 0x0501
	#include <windows.h>
	#define PATH_MAX MAX_PATH
#else
	#include <limits.h>
	#define MAX_PATH PATH_MAX
#endif

#include <stdexcept>
#include <string>
#include <boost/config.hpp>				// for BOOST_STRINGIZE
#include <boost/current_function.hpp>	// for BOOST_CURRENT_FUNCTION
#include <boost/interprocess/sync/interprocess_recursive_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/cstdint.hpp>

typedef boost::interprocess::interprocess_recursive_mutex nxRecursiveMutex;
typedef boost::interprocess::scoped_lock<nxRecursiveMutex> nxScopedRecursiveLock;





/// Detailed throw, which includes the file, line, pretty function name, and an error string.
// std::string::npos is defined as "Maximum value for size_t", so, if there's no path on __FILE__, max value + 1 is 0, so, it gets the whole thing.  Just what we wanted.
#define nxThrow(x) (throw std::runtime_error(std::string(__FILE__).substr(std::string(__FILE__).find_last_of("/\\")+1) + '(' + BOOST_STRINGIZE(__LINE__) + ") in '" + std::string(BOOST_CURRENT_FUNCTION) + "': " + (x) ))

/// Retrieves the process that runs this command's complete path and file name, and separates the path from the filename
// path includes the trailing "/"
bool nxApplicationPath(std::string&strPath,std::string&strFile);



typedef boost::uint_least32_t nxInputId;
typedef boost::uint_least32_t nxInputMessage;
typedef boost::uint8_t nxDeviceId;
typedef boost::uint8_t nxExtendedId;

enum nxInputType { NX_KEYBOARD=0, NX_MOUSE, NX_XINPUT, NX_DINPUT };

// Call at start of program with the hInstance
// Retrieve the HINSTANCE by passing NULL (or no parameter) at a later time
#if defined(OS_WINDOWS_)
HINSTANCE nxInstanceHandle(HINSTANCE hInst=NULL);
#endif



#endif // NXCOMMON_HPP_INCLUDED
