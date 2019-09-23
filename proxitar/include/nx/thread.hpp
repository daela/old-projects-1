#ifndef INCLUDED_NX_THREAD_HPP
#define INCLUDED_NX_THREAD_HPP

#include <nx/os.hpp>

#ifndef OS_WINDOWS_
#include <pthread.h>
#endif

namespace nx
{

	class thread
	{
		public:

		#ifdef OS_WINDOWS_
		typedef HANDLE handle;
		#else
		typedef pthread_t handle;
		#endif

		virtual ~thread();

		// driver prototype
		virtual int main(void*ptData)=0;

		// spawn a thread
		bool spawn(handle&hThread,void*ptData=NULL);

		// join with a thread
		static int join(handle hThread);
	};
}

#endif
