#ifndef INCLUDED_NX_TIME_HPP
#define INCLUDED_NX_TIME_HPP

#include <nx/common.hpp>
#include <nx/cstdint.hpp>
namespace nx
{
	void millisleep(const unsigned int&millisecs,const unsigned int&resolution=1);

	class timer
	{
		public:
			enum { NUM_BITS=32 };
			typedef uint_least<NUM_BITS>::type msec_t;
			static msec_t elapsed(const msec_t uStart,const msec_t uEnd);
			static int compare(const msec_t uLeft,const msec_t uRight);
			timer();
			msec_t get() const;
			~timer();
	};
}
#endif

