#include "nxThread.hpp"

#if defined(OS_WINDOWS_)

// Intermediate callback used to allow the use of a member function.
DWORD WINAPI nxThreadProcToObject(LPVOID lpParameter)
{
	return reinterpret_cast<nxThreadProc*>(lpParameter)->ThreadProc();
}

// CreateThread wrapper that uses my threadproc interface.  No need to pass data, you have members!
HANDLE nxCreateThread(nxThreadProc&objProc,LPDWORD ptThreadID,bool bSuspended)
{
	return CreateThread(NULL,0,nxThreadProcToObject,&objProc,(bSuspended?CREATE_SUSPENDED:0),ptThreadID);
}
// CreateThread wrapper that uses my threadproc interface.  No need to pass data, you have members!
HANDLE nxCreateThread(LPTHREAD_START_ROUTINE lpThreadProc,LPVOID lpParameter,LPDWORD ptThreadID,bool bSuspended)
{
	return CreateThread(NULL,0,lpThreadProc,lpParameter,(bSuspended?CREATE_SUSPENDED:0),ptThreadID);
}

#endif
