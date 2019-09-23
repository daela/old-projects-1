#include <nx/os.hpp>
#include <nx/thread.hpp>
#include <nx/cstdint.hpp>
#include <utility>
namespace nx
{

	namespace detail
	{
		typedef std::pair<thread*,void*> thread_app_proc_data_t;

		// proc to launch application main()
		#ifdef OS_WINDOWS_
		DWORD WINAPI winthread_app_proc( LPVOID ptr ) 
		#else
		void *pthread_app_proc( void *ptr )
		#endif
		{
			// pair from parameter
			thread_app_proc_data_t * ptPassedPair = reinterpret_cast<thread_app_proc_data_t*>(ptr);
			// local copy
			thread_app_proc_data_t objPair((thread*)NULL,(void*)NULL);
			if (ptPassedPair)
			{
				objPair= *ptPassedPair;
				// free original
				delete ptPassedPair;
			}
			
			if (objPair.first)
			{
				return 
					#ifdef OS_WINDOWS_
					(DWORD)
					#else
					(void*)
					#endif
					(objPair.first->main(objPair.second));
			}
			return 0;
		}

	}

	// virtual destructor
	thread::~thread() { }

	// spawn a thread
	bool thread::spawn(handle&hThread,void*ptData)
	{
		detail::thread_app_proc_data_t* ptProcData = new detail::thread_app_proc_data_t;
		ptProcData->first = this;
		ptProcData->second = (void*) ptData;
		#if OS_WINDOWS_
		hThread=CreateThread( NULL, 0, detail::winthread_app_proc, ptProcData, 0, NULL);  
		if (hThread == NULL)
		{
			delete ptProcData;
			return false;
		}
		return true;
		#else
		if (pthread_create(&hThread, NULL, detail::pthread_app_proc, (void*)ptProcData) != 0)
		{
			delete ptProcData;
			return false;
		}
		return true;
		#endif
	}


	// join a thread
	int thread::join(handle hThread)
	{
		int iRet;
		#ifdef OS_WINDOWS_
		DWORD ret;
		WaitForSingleObject(hThread,INFINITE);
		if (GetExitCodeThread(hThread,&ret) == 0)
		{
			CloseHandle(hThread);
			// TODO: error
			return 0;
		}
		CloseHandle(hThread);
		iRet=ret;
		#else
		void* ret;
		if (pthread_join(hThread,&ret) != 0)
		{
			// TODO: error
			return 0;
		}
		iRet=(int)reinterpret_cast<uintptr_t>(ret);
		#endif
		return iRet;
	}

}

