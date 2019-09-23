#include <nx/common.hpp>
#include <nx/time.hpp>
#include <nx/numeric.hpp>

#ifndef OS_WINDOWS_
#include <time.h> // clock_gettime
#include <unistd.h> // symbols telling me if clock_gettime is available
#include <sys/time.h> // gettimeofday
#endif
#include <cerrno>
namespace nx
{
	void millisleep(const unsigned int&millisecs,const unsigned int&resolution)
	{
		if (millisecs)
		{
			#ifdef OS_WINDOWS_
			// If our event object was created successfully
			HANDLE hEvent = CreateEvent( NULL, true, FALSE, NULL );
			if (hEvent != NULL)
			{
				// Clear the event's signalled state
				ResetEvent(hEvent);
				// Create a multimedia timer with 1ms precision that will signal the event upon expiration
				UINT uTimer=timeSetEvent( millisecs, resolution, (LPTIMECALLBACK)hEvent, 0, TIME_ONESHOT | TIME_CALLBACK_EVENT_SET );
				if (uTimer)
				{
					// Wait for the event to be signalled, and wait at most the number of milliseconds specified
					// The timer will wake us up early, though, as it has higher precision than WaitForSingleObject
					WaitForSingleObject( hEvent, millisecs );
					// Kill the timer event just in case something crazy happened
					timeKillEvent(uTimer);
					// Cleanup
					CloseHandle(hEvent);
					return;
				}
				// Cleanup
				CloseHandle(hEvent);
			}
			// Fall back to the normal ~15ms accuracy sleep.
			Sleep(millisecs);
			#else
			struct timespec tspec;
			// Get the number of seconds
			tspec.tv_sec = millisecs/1000ul;
			// Subtract the seconds off of the milliseconds, and then convert to nanoseconds
			tspec.tv_nsec = (millisecs-(tspec.tv_sec*1000ul))*1000000ul;
			// Loop until the time elapses, calling again for the remainder if interrupted by a signal.
			while(nanosleep(&tspec,&tspec)==-1 && errno == EINTR) ; //LOOP IS EMPTY

			#endif
		}
	}

	timer::timer()
	{
		#ifdef OS_WINDOWS_
		timeBeginPeriod(1);
		#endif
	}
	
	timer::msec_t timer::get() const
	{
		#ifdef OS_WINDOWS_
		return timeGetTime();
		#else
			#if _POSIX_TIMERS > 0 && defined(_POSIX_MONOTONIC_CLOCK)
			// use CLOCK_MONOTONIC if it's available
				struct timespec tspec;
				if (clock_gettime(CLOCK_MONOTONIC,&tspec) != 0)
					throw std::runtime_error("clock_gettime failed!");
				return static_cast<msec_t>(tspec.tv_sec)*1000 + static_cast<msec_t>(tspec.tv_nsec/1000000);
			#else
			// fallback on gettimeofday
				struct timeval tval;
				gettimeofday(&tval,NULL);
				return static_cast<msec_t>(tval.tv_sec)*1000 + static_cast<msec_t>(tval.tv_usec/1000);
			#endif
		#endif
	}
	timer::msec_t timer::elapsed(const timer::msec_t uStart,const timer::msec_t uEnd)
	{
		return (uEnd - uStart) & numeric::fill_bits<timer::NUM_BITS>::value;
	}
	int timer::compare(const timer::msec_t uLeft,const timer::msec_t uRight)
	{
		// only a 1 in the high bit.
		static const msec_t uMidPoint = (numeric::fill_bits<timer::NUM_BITS>::value >> 1)+1;
		// <
		if (elapsed(uLeft+1,uRight) < uMidPoint)
			return -1;
		// >
		if (elapsed(uRight+1,uLeft) < uMidPoint)
			return 1;
		// ==
		return 0;
	}
	timer::~timer()
	{
		#ifdef OS_WINDOWS_
		timeEndPeriod(1);
		#endif
	}

}
