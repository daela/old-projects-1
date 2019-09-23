#include <windows.h>
#include <string>
#include <nac/Process.h>
#include <nac/ImportAddressTable.h>
#include <nac/strmanip.h>
using namespace std;

std::string strDllFolder;
std::string strDllFile;
bool bItemType;
bool bNoEffects;
bool bKeepBPP;

bool FileExists(std::string strFileName)
{
	WIN32_FIND_DATA obj;
	return (FindFirstFile(strFileName.c_str(),&obj) != INVALID_HANDLE_VALUE);
}
/////////////////////////////////////////////////////////
//
//	Begin IAT Code
//
/////////////////////////////////////////////////////////
nac::ImportAddressTable objIAT;
namespace IAT_Functions
{
	// Stops anti-trojan and stops opening the CO website
	HINSTANCE WINAPI ShellExecuteA(HWND hwnd,LPCSTR lpOperation,LPCSTR lpFile,LPCSTR lpParameters,LPCSTR lpDirectory,INT nShowCmd)
	{
		return (HINSTANCE)33;
	}

	/*LONG WINAPI ChangeDisplaySettingsA(PDEVMODEA lpDevMode,DWORD dwFlags)
	{
		if (bKeepBPP && lpDevMode && lpDevMode->dmFields & DM_BITSPERPEL)
		{
			lpDevMode->dmFields &= ~DM_BITSPERPEL;
		}
		return objIAT.Real(ChangeDisplaySettingsA)(lpDevMode,dwFlags);
	}*/
	HANDLE WINAPI OpenMutexA(DWORD dwDesiredAccess,BOOL bInitialOwner,LPSTR lpName)
	{
		//MessageBoxA(NULL,lpName,"..",MB_OK);
		if (strlen(lpName) == 12 && strncmp(lpName,"TQ_CONQUER",10)==0) return NULL;
		return objIAT.Real(OpenMutexA)(dwDesiredAccess,bInitialOwner,lpName);
	}
	FILE*fopen(const char *filename,const char *mode)
	{
		if (filename)
		{
			if (bItemType && stricmp(filename,"ini/itemtype.dat") == 0)
			{
				string strNewFile = strDllFolder + "itemtype.dat";
				if (FileExists(strNewFile)) //no sense in opening a non-existant file
					return objIAT.Real(fopen)(strNewFile.c_str(),mode);
			}
			if (bNoEffects && stricmp(filename,"ini/3DEffect.ini") == 0)
			{

				return NULL;
			}
		}
		return objIAT.Real(fopen)(filename,mode);
	}
	void Install(void)
	{

		objIAT.AddRedirect("shell32.dll","ShellExecuteA",(DWORD)ShellExecuteA);
		objIAT.AddRedirect("kernel32.dll","OpenMutexA",(DWORD)OpenMutexA);
		//objIAT.AddRedirect("msvcrt.dll","fopen",(DWORD)fopen);

		//if (bKeepBPP)
		//	objIAT.AddRedirect("user32.dll","ChangeDisplaySettingsA",(DWORD)ChangeDisplaySettingsA);

		objIAT.PrepareImports();
		objIAT.Install();
	}
	void Restore(void)
	{
		objIAT.Restore();
	}
}
/////////////////////////////////////////////////////////
//
//	End IAT Code
//
/////////////////////////////////////////////////////////

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
        	MessageBoxA(NULL,"THE STUPID FUCKING DLL IS LOADED!!","nacitar sucks",MB_OK);
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
			/*IAT_Functions::Restore();*/
            break;
        }
    }
    return TRUE;
}
