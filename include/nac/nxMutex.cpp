#include "nxMutex.hpp"

void nxMutex::operator=(nxMutex &M) {}

nxMutex::nxMutex( const nxMutex &M ) {}

#ifdef NXWINDOWS

	nxMutex::nxMutex()
	{ InitializeCriticalSection(&C); }

	nxMutex::~nxMutex()
	{ DeleteCriticalSection(&C); }

	int nxMutex::Lock() const
	{ EnterCriticalSection(&C); return 0; }

	#if (_WIN32_WINNT >= 0x0400)
	int nxMutex::TryLock() const
	{ return (TryEnterCriticalSection(&C)?0:EBUSY); }
	#endif

	int nxMutex::Unlock() const
	{ LeaveCriticalSection(&C); return 0; }

#else

	nxMutex::nxMutex()
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&M,&attr);
		pthread_mutexattr_destroy(&attr);
	}

	nxMutex::~nxMutex()
	{ pthread_mutex_unlock(&M); pthread_mutex_destroy(&M); }

	inline int nxMutex::Lock() const
	{ return pthread_mutex_lock(&M); }

	inline int nxMutex::TryLock() const
	{ return pthread_mutex_trylock(&M); }

	inline int nxMutex::Unlock() const
	{ return pthread_mutex_unlock(&M); }

#endif
