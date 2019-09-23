#include <windows.h>
#include <Tlhelp32.h>
#include <iostream>
#include <string>
#include <sstream>
#include "madCHook.hpp"

MadCodeHook madCodeHook;
bool bItemType, bNoEffects, bMadLoaded;
std::string strAppDir;
DWORD GetParentPID(DWORD dwPID=0)
{
	PROCESSENTRY32 pe32;
	HANDLE hSnapshot;
	DWORD dwRet = 0;
	if (dwPID == 0)
		dwPID = GetCurrentProcessId();
	pe32.dwSize = sizeof(PROCESSENTRY32);
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		if (Process32First(hSnapshot, &pe32))
			while (Process32Next(hSnapshot, &pe32))
				if (pe32.th32ProcessID==dwPID)
				{
					dwRet = pe32.th32ParentProcessID;
					break;
				}
		CloseHandle(hSnapshot);
	}
	return dwRet;
}
bool FileExists(std::string strFileName)
{
	WIN32_FIND_DATA obj;
	return (FindFirstFile(strFileName.c_str(),&obj) != INVALID_HANDLE_VALUE);
}
std::string GetAppDir(HINSTANCE hInst=NULL)
{
	char szName [ MAX_PATH+1 ];
	GetModuleFileName(hInst,szName,MAX_PATH);
	std::string strName = szName;
	return strName.substr(0,strName.find_last_of("\\/")+1);
}

HINSTANCE WINAPI (*orgShellExecuteA)(HWND hwnd,LPCSTR lpOperation,LPCSTR lpFile,LPCSTR lpParameters,LPCSTR lpDirectory,INT nShowCmd);
HINSTANCE WINAPI mchShellExecuteA(HWND hwnd,LPCSTR lpOperation,LPCSTR lpFile,LPCSTR lpParameters,LPCSTR lpDirectory,INT nShowCmd)
{
	return (HINSTANCE)33;
}

LONG WINAPI (*orgChangeDisplaySettingsA)(PDEVMODEA lpDevMode,DWORD dwFlags);
LONG WINAPI mchChangeDisplaySettingsA(PDEVMODEA lpDevMode,DWORD dwFlags)
{
	if (lpDevMode && lpDevMode->dmFields & DM_BITSPERPEL)
	{
		lpDevMode->dmFields &= ~DM_BITSPERPEL;
	}
	return orgChangeDisplaySettingsA(lpDevMode,dwFlags);
}



HANDLE WINAPI (*orgOpenMutexA)(DWORD dwDesiredAccess,BOOL bInitialOwner,LPSTR lpName);
HANDLE WINAPI mchOpenMutexA(DWORD dwDesiredAccess,BOOL bInitialOwner,LPSTR lpName)
{
	if (strlen(lpName) == 12 && strncmp(lpName,"TQ_CONQUER",10)==0) return NULL;
	return orgOpenMutexA(dwDesiredAccess,bInitialOwner,lpName);
}

FILE*(*orgfopen)(const char *filename,const char *mode);
FILE*mchfopen(const char *filename,const char *mode)
{
	if (filename)
	{
		if (bItemType && stricmp(filename,"ini/itemtype.dat") == 0)
		{
			std::string strNewFile = strAppDir + "itemtype.dat";
			if (FileExists(strNewFile)) //no sense in opening a non-existant file
				return orgfopen(strNewFile.c_str(),mode);
		}
		if (bNoEffects && stricmp(filename,"ini/3DEffect.ini") == 0)
		{
			return NULL;
		}
	}
	return orgfopen(filename,mode);
}


// Keep track of current socket handle
SOCKET coSocket;
SOCKET WSAAPI (*orgsocket)(int af,int type,int protocol);
SOCKET WSAAPI mchsocket(int af,int type,int protocol)
{
	SOCKET ret;
	coSocket = ret = orgsocket(af,type,protocol);
	return ret;
}
int WSAAPI (*orgclosesocket)(SOCKET s);
int WSAAPI mchclosesocket(SOCKET s)
{
	if (s == coSocket)
		coSocket = (SOCKET)NULL;
	return orgclosesocket(s);
}

// Keyboard interaction
BOOL WINAPI (*orgGetKeyboardState)(PBYTE lpKeyState);
BOOL WINAPI mchGetKeyboardState(PBYTE lpKeyState)
{
	BOOL ret = orgGetKeyboardState(lpKeyState);
	DWORD pid;
	// Get current window's PID
	GetWindowThreadProcessId(GetForegroundWindow(),&pid);
	// If we're in the right window
	if (pid == GetCurrentProcessId())
	{
		//Ctrl-T disconnect
		if ((lpKeyState[VK_CONTROL] & 0x80) && (lpKeyState[0x54] & 0x80))
		{
			if (coSocket)
				mchclosesocket(coSocket);
		}
	}
	return ret;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {

        case DLL_PROCESS_ATTACH:
        {
			coSocket = (SOCKET)NULL;
			strAppDir = GetAppDir(hinstDLL);
			std::string strMadCHook = strAppDir + "madCHook.dll";
			bItemType = bNoEffects = bMadLoaded = false;
			if (!FileExists(strMadCHook))
			{
				MessageBoxA(NULL,"You must have madCHook.dll in the same folder as nConquer.\r\nConquer will open normally.","DLL Missing",MB_OK);
				return FALSE;
			}
			std::string strIpcName;
			{
				std::stringstream ss;
				ss << "NAC_IPC_" << GetParentPID();
				strIpcName = ss.str();
			}
			if (!madCodeHook.Init(strMadCHook))
			{
				MessageBoxA(NULL,"madCHook.dll was located, but couldn't be loaded.\r\nConquer will open normally.","DLL Load Error",MB_OK);
				return FALSE;
			}
			bMadLoaded = true;

			madCodeHook.SendIpcMessage(strIpcName.c_str(), (LPVOID)"i", 1, (LPVOID)&bItemType, sizeof(bool),INFINITE,FALSE);
			madCodeHook.SendIpcMessage(strIpcName.c_str(), (LPVOID)"e", 1, (LPVOID)&bNoEffects, sizeof(bool),INFINITE,FALSE);
			{
				bool result = false;
				madCodeHook.SendIpcMessage(strIpcName.c_str(), (LPVOID)"q", 1, (LPVOID)&result, sizeof(bool),INFINITE,FALSE);
			}
			madCodeHook.HookAPI("shell32.dll","ShellExecuteA",(PVOID)mchShellExecuteA,(PVOID*)&orgShellExecuteA,0);
			madCodeHook.HookAPI("user32.dll","ChangeDisplaySettingsA",(PVOID)mchChangeDisplaySettingsA,(PVOID*)&orgChangeDisplaySettingsA,0);
			if (bItemType || bNoEffects)
				madCodeHook.HookAPI("msvcrt.dll","fopen",(PVOID)mchfopen,(PVOID*)&orgfopen,0);
			madCodeHook.HookAPI("kernel32.dll","OpenMutexA",(PVOID)mchOpenMutexA,(PVOID*)&orgOpenMutexA,0);
			madCodeHook.HookAPI("ws2_32.dll","socket",(PVOID)mchsocket,(PVOID*)&orgsocket,0);
			madCodeHook.HookAPI("ws2_32.dll","closesocket",(PVOID)mchclosesocket,(PVOID*)&orgclosesocket,0);
			madCodeHook.HookAPI("user32.dll","GetKeyboardState",(PVOID)mchGetKeyboardState,(PVOID*)&orgGetKeyboardState,0);


            break;
        }
        case DLL_PROCESS_DETACH:
        {
        	if (bMadLoaded)
        	{
				madCodeHook.UnhookAPI((PVOID*)&orgShellExecuteA);
				madCodeHook.UnhookAPI((PVOID*)&orgChangeDisplaySettingsA);
				madCodeHook.UnhookAPI((PVOID*)&orgfopen);
				madCodeHook.UnhookAPI((PVOID*)&orgOpenMutexA);
				madCodeHook.UnhookAPI((PVOID*)&orgsocket);
				madCodeHook.UnhookAPI((PVOID*)&orgclosesocket);
				madCodeHook.UnhookAPI((PVOID*)&orgGetKeyboardState);

				bMadLoaded = false;
        	}
            break;
        }
    }
    return TRUE;
}
