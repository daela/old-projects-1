#include <windows.h>
#include <Tlhelp32.h>
#include <win32/madCHook.hpp>
#include <iostream>
#include <string>
#include <sstream>
MadCodeHook madCodeHook;
bool bItemType, bNoEffects;

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

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {

        case DLL_PROCESS_ATTACH:
        {

			std::string strDirectory = GetAppDir(hinstDLL);
			std::string strMadCHook = strDirectory + "madCHook.dll";
			std::string strIpcName;
			{
				std::stringstream ss;
				ss << "NAC_IPC_" << GetParentPID();
				strIpcName = ss.str();
			}
			MessageBoxA(NULL,strIpcName.c_str(),"nacitar sucks",MB_OK);
			if (madCodeHook.Init(strMadCHook))
			{
				std::cout << "Yay!" << std::endl;
			}
			bool result = false;
			madCodeHook.SendIpcMessage(strIpcName.c_str(), (LPVOID)"i", 1, (LPVOID)&result, sizeof(bool),INFINITE,FALSE);
			MessageBoxA(NULL,(result?"TRUE":"FALSE"),"DLL",MB_OK);
			madCodeHook.SendIpcMessage(strIpcName.c_str(), (LPVOID)"q", 1, (LPVOID)&result, sizeof(bool),INFINITE,FALSE);
			MessageBoxA(NULL,(result?"TRUE":"FALSE"),"DLL",MB_OK);
			madCodeHook.HookAPI("shell32.dll","ShellExecuteA",(PVOID)mchShellExecuteA,(PVOID*)&orgShellExecuteA,0);
        	/*char szName [ MAX_PATH+1 ];
        	std::vector<unsigned char> vcBuffer;

			GetModuleFileName(hinstDLL,szName,MAX_PATH);
			nac::split_path(szName,strDllFolder,strDllFile);
			nac::Process::GetParentMessage(vcBuffer);

			bItemType = bNoEffects = bKeepBPP = false;
			for (std::vector<unsigned char>::iterator it = vcBuffer.begin();it!=vcBuffer.end();++it)
			{
				switch (*it)
				{
					case 'i': bItemType = true; break;
					case 'e': bNoEffects = true; break;
					case 'b': bKeepBPP = true; break;
				}
			}
			objIAT.AddIgnore(strDllFile);
			IAT_Functions::Install();*/
            break;
        }
        case DLL_PROCESS_DETACH:
        {
        	MessageBoxA(NULL,"IF YOU ARE READING THIS WE ARE FUCKED!","nacitar sucks",MB_OK);
        	madCodeHook.UnhookAPI((PVOID*)&orgShellExecuteA);
			/*IAT_Functions::Restore();*/
            break;
        }
    }
    return TRUE;
}
