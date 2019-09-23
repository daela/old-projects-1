#ifndef INCLUDED_NX_CSTDINT_HPP
#define INCLUDED_NX_CSTDINT_HPP

#include <nx/os.hpp>

//
// IF YOU GET ERRORS FROM THIS HEADER, YOU NEED TO ADJUST THE TYPEDEFS TO MATCH YOUR SYSTEM!!!
//

namespace nx
{
	// Maximum width integer types

	// On gcc (and maybe other compilers?) long long is always supported
	// but it's use may generate either warnings (with -ansi), or errors
	// (with -pedantic -ansi) unless it's use is prefixed by __extension__

	#ifdef TC_GCC_
	__extension__ typedef unsigned long long uintmax_t;
	__extension__ typedef signed long long intmax_t;
	#else
	typedef unsigned long long uintmax_t;
	typedef signed long long intmax_t;
	#endif
}

// pull in the classes, so we can use them to make our types
// NOTE: this header NEEDS nx::[u]intmax_t !!!
#include <nx/utility.hpp>
#include <nx/detail/cstdint.hpp>
#include <cstddef> // for size_t, ptrdiff_t

#ifndef OS_WINDOWS_
#include <sys/types.h> // pid_t
#endif

namespace nx
{
	typedef uint<8>::type	uint8_t;
	typedef sint<8>::type	int8_t;
	typedef uint<16>::type	uint16_t;
	typedef sint<16>::type	int16_t;
	typedef uint<32>::type	uint32_t;
	typedef sint<32>::type	int32_t;
	typedef uint<64>::type	uint64_t;
	typedef sint<64>::type	int64_t;

	typedef uint_least<8>::type	uint_least8_t;
	typedef uint_least8_t		uint_fast8_t;
	typedef sint_least<8>::type	int_least8_t;
	typedef int_least8_t		int_fast8_t;
	typedef uint_least<16>::type	uint_least16_t;
	typedef uint_least16_t		uint_fast16_t;
	typedef sint_least<16>::type	int_least16_t;
	typedef int_least16_t		int_fast16_t;
	typedef uint_least<32>::type	uint_least32_t;
	typedef uint_least32_t		uint_fast32_t;
	typedef sint_least<32>::type	int_least32_t;
	typedef int_least32_t		int_fast32_t;
	typedef uint_least<64>::type	uint_least64_t;
	typedef uint_least64_t		uint_fast64_t;
	typedef sint_least<64>::type	int_least64_t;
	typedef int_least64_t		int_fast64_t;

	typedef uint_least<sizeof_bits<void*>::value>::type uintptr_t;
	typedef sint_least<sizeof_bits<void*>::value>::type intptr_t;

	typedef std::ptrdiff_t ptrdiff_t;
	typedef std::size_t size_t;

	#ifdef OS_WINDOWS_
	typedef DWORD pid_t;
	#else
	typedef pid_t pid_t;
	#endif
	typedef uint_least<sizeof_bits<pid_t>::value>::type uintpid_t;
}

#endif
