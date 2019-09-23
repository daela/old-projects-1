#ifndef __CRITICALSECTION_H__
#define __CRITICALSECTION_H__

#include <windows.h>

namespace nac
{
	class CriticalSection
	{
		private:
			CRITICAL_SECTION cSec;
		public:
			CriticalSection();
			~CriticalSection();
			void Enter();
			void Leave();
	};
}

#endif
