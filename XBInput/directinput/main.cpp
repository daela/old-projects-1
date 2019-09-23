
#include <windows.h>
#include <iostream>
#include <map>

///http://cs.senecac.on.ca/~weaver/gam666a/joystick.pdf
///Create a thread for DirectInput/XInput polling, with a Sleep(1) (obsiv suggested 16), likely for the same reason
///- Takahashi Meijin is considered to be the fastest button masher in the world, with a record of 16 presses of a button in one second. (62.5ms per press)
///Create a thread for WM_INPUT message processing via message only window.. using GetMessage() to wait for messages
///Create a thread for the actual input processing, make both DI and RI send messages here for input.  GetMessage() here too, so blocking can occur.

// Forward declaration
DWORD WINAPI nxThreadProcToObject(LPVOID lpParameter);

// ThreadProc interface
class nxThreadProc
{
	virtual DWORD WINAPI ThreadProc(void)=0;
	friend DWORD WINAPI nxThreadProcToObject(LPVOID lpParameter);
};

// Intermediate callback used to allow the use of a member function.
DWORD WINAPI nxThreadProcToObject(LPVOID lpParameter)
{
	return reinterpret_cast<nxThreadProc*>(lpParameter)->ThreadProc();
}

// CreateThread wrapper that uses my threadproc interface.  No need to pass data, you have members!
HANDLE nxCreateThread(nxThreadProc&objProc,LPDWORD ptThreadID=NULL,bool bSuspended=false)
{
	return CreateThread(NULL,0,nxThreadProcToObject,&objProc,(bSuspended?CREATE_SUSPENDED:0),ptThreadID);
}

class TestThread : public nxThreadProc
{
	private:
	virtual DWORD WINAPI ThreadProc(void)
	{
		while (1)
		{
			std::cout << ".";
			Sleep(1000);
		}
		return 0;
	}
};


int main(int argc,char* argv[])
{
	/*
	DWORD dwThreadID;
	HANDLE hThread = CreateThread(NULL,0,ThreadProc,NULL,0,&dwThreadID);
	if (hThread == NULL)
		std::cout << "Error!";*/
	TestThread obj;

	//nxThreadDispatcher*pt=&obj;
	//pt->ThreadProc(NULL);
	nxCreateThread(obj);
	while (1)
	{
		std::cout << "!";
		Sleep(1000);
	}
	return 0;
}
