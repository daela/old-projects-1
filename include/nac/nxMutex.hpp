#ifndef __NXMUTEX_HPP__
#define __NXMUTEX_HPP__

#include <errno.h>
#include "type.hpp"

#ifdef NXWINDOWS
class nxMutex
{
	mutable CRITICAL_SECTION C;
#else
#	include <pthread.h>
class nxMutex
{
	mutable pthread_mutex_t M;
#endif

  void operator=(nxMutex &M);
  nxMutex( const nxMutex &M );

  public:

  nxMutex();

  virtual ~nxMutex();

  int Lock() const;

#if !defined(NXWINDOWS) || (_WIN32_WINNT >= 0x0400)
  int TryLock() const;
#endif

  int Unlock() const;
};

#endif // __NXMUTEX_HPP__

