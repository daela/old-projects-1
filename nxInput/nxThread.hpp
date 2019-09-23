#ifndef NXTHREAD_HPP_INCLUDED
#define NXTHREAD_HPP_INCLUDED

#include "nxCommon.hpp"

#if defined(OS_WINDOWS_)

// // Intermediate callback used to allow the use of a member function.
DWORD WINAPI nxThreadProcToObject(LPVOID lpParameter);

// ThreadProc interface
class nxThreadProc
{
	virtual DWORD WINAPI ThreadProc(void)=0;
	friend DWORD WINAPI nxThreadProcToObject(LPVOID lpParameter);
};

// CreateThread wrapper that uses my threadproc interface.  No need to pass data, you have members!
HANDLE nxCreateThread(nxThreadProc&objProc,LPDWORD ptThreadID=NULL,bool bSuspended=false);
HANDLE nxCreateThread(LPTHREAD_START_ROUTINE lpThreadProc,LPVOID lpParameter=NULL,LPDWORD ptThreadID=NULL,bool bSuspended=false);
#endif

#endif // NXTHREAD_HPP_INCLUDED
