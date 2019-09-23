#ifndef INCLUDED_NX_COMMON_HPP
#define INCLUDED_NX_COMMON_HPP

#include <nx/os.hpp>
#include <nx/cstdint.hpp>
#include <stdexcept>
#include <string>
#include <sstream>

#ifndef OS_WINDOWS_
	#include <unistd.h> // getpid()
#endif

/// Retrieves the process that runs this command's complete path and file name, and separates the path from the filename
// path includes the trailing "/"
namespace nx
{

	template <class T>
	T min(const T&a,const T&b)
	{
		if (a < b) return a;
		return b;
	}
	template <class T>
	T max(const T&a,const T&b)
	{
		if (b < a) return a;
		return b;
	}
	bool application_path(std::string&strPath,std::string&strFile);
	bool dead_process(const pid_t&pid);
	inline pid_t self_pid(void)
	{
		#ifdef OS_WINDOWS_
		return ::GetCurrentProcessId();
		#else
		return ::getpid();
		#endif
	}

	class endian
	{
		public:
			typedef uint_least<1>::type type;
			typedef uint_least<sizeof_bits<type>::value+1>::type larger_type;
		private:
			endian(); // prevent declaration of the type
			typedef	union {const larger_type uData;const type uMem;} EndianUnion;
			static const EndianUnion objEndianUnion;
		public:
			static const type & little;
			static const type & big;
	};
}


#endif
