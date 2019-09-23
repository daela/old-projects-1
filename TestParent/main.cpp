#include <set>
#include <queue>
#include <map>
#include <string>
#include <windows.h>
#include <iostream>
#include <nac/DllHook.h>
/*

	Update IAT for every module that's loaded
	Update EAT for modules we intercept from
	If newly loaded, update EAT again


	APPLICATION
		- Indexed by original function
		- If found, replace with new
	REMOVAL
		- Indexed by replacement function
		- If found, replace with original

*/
/*
class HookDll
{
	private:
		typedef std::map<DWORD,DWORD> dword_map;
		typedef std::pair<DWORD,DWORD> dword_pair;
		typedef std::map<std::string,DWORD> func_map;

		// List of intercepted libraries, indexed by name for fast searching
		static std::map<std::string,HookDll> mpLibraries;

		static HookDll*GetHook(const std::string&strLibrary);
		std::string	strModuleName;
		HMODULE		hModule;
		func_map	mpModuleFunctions;
		dword_map	mpModuleToRedirect;
		dword_map	mpRedirectToModule;
		static void WriteIAT(DWORD*ptLocation,DWORD ptValue);
		bool ApplyChanges(IMAGE_THUNK_DATA*pThunk,const dword_map&mpLookup) const;
		bool InstallEntry(IMAGE_THUNK_DATA*pThunk) const;
		bool RemoveEntry(IMAGE_THUNK_DATA*pThunk) const;
		template<class T,class A,class B> static T MakePtr( A objA, B objB )
		{
			return (T)((DWORD)(objA)+(DWORD)(objB));
		}
		static bool UpdateInstallation(std::map<std::string,HookDll*>&mpLibraries,bool bInstall);
		bool UpdateMappings(void);

		HookDll(std::string strModuleName_);
	public:

		bool SetRedirect(std::string strFunction,DWORD dwReplacement);

		template<class T> T Real(T pfnFunction) const
		{
			dword_map::const_iterator it = mpRedirectToModule.find((DWORD)pfnFunction);
			if (it != mpRedirectToModule.end())
				return (T)it->second;
			return (T)0;
		}
		static bool Install(std::map<std::string,HookDll*>&mpLibraries);
		static bool Remove(std::map<std::string,HookDll*>&mpLibraries);
};
bool HookDll::Install(std::map<std::string,HookDll*>&mpLibraries)
{
	return UpdateInstallation(mpLibraries,true);
}
bool HookDll::Remove(std::map<std::string,HookDll*>&mpLibraries)
{
	return UpdateInstallation(mpLibraries,false);
}
bool HookDll::UpdateInstallation(std::map<std::string,HookDll*>&mpLibraries,bool bInstall)
{
	std::queue<std::string> quModules;
	std::set<std::string> stProcessed;
	quModules.push("");
	while (!quModules.empty())
	{
		std::string strModuleName = quModules.front();
		//std::cout << "Processing: " << strModuleName << std::endl;
		quModules.pop();
		HMODULE hModule;
		if (strModuleName.empty())
			hModule=GetModuleHandle(NULL);
		else
			hModule=GetModuleHandle(strModuleName.c_str());
		if (!hModule) continue;
		IMAGE_DOS_HEADER *pDosHeader;
		IMAGE_NT_HEADERS *pNTHeader;
		IMAGE_IMPORT_DESCRIPTOR *pImportDesc;
		pDosHeader = (PIMAGE_DOS_HEADER)hModule;
		// Tests to make sure we're looking at a module image (the 'MZ' header)
		if (IsBadReadPtr(pDosHeader, sizeof(IMAGE_DOS_HEADER)) || pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			continue;
		// The MZ header has a pointer to the PE header
		pNTHeader = MakePtr<PIMAGE_NT_HEADERS>(pDosHeader, pDosHeader->e_lfanew);
		// More tests to make sure we're looking at a "PE" image
		if (IsBadReadPtr(pNTHeader, sizeof(IMAGE_NT_HEADERS)) || pNTHeader->Signature != IMAGE_NT_SIGNATURE)
			continue;
		// We now have a valid pointer to the module's PE header.
		// Now get a pointer to its imports section
		pImportDesc = MakePtr<PIMAGE_IMPORT_DESCRIPTOR>(
			pDosHeader,
			pNTHeader->OptionalHeader.DataDirectory
			[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		//Go out if imports table doesn't exist
		if ( pImportDesc <= (PIMAGE_IMPORT_DESCRIPTOR)pNTHeader )
			continue; //pImportDesc will ==pNTHeader.
		while ( pImportDesc->Name ) //Name is a DWORD (RVA, to a DLL name)
		{
				std::string strImportName = MakePtr<char*>(pDosHeader, pImportDesc->Name);
				nac::string_case<nac::TO_LOWER>(strImportName);
				if (stProcessed.find(strImportName) == stProcessed.end())
				{
					quModules.push(strImportName);
					stProcessed.insert(strImportName);
				}
				std::map<std::string,HookDll*>::iterator it = mpLibraries.find(strImportName);
				if (it != mpLibraries.end())
				{
					it->second->UpdateMappings();
					if (bInstall)
						it->second->InstallEntry(MakePtr<PIMAGE_THUNK_DATA>(pDosHeader, pImportDesc->FirstThunk));
					else
						it->second->RemoveEntry(MakePtr<PIMAGE_THUNK_DATA>(pDosHeader, pImportDesc->FirstThunk));
				}
				++pImportDesc;
		}
	}
	return true;
}
HookDll::HookDll(std::string strModuleName_)
{
	hModule = NULL;
	strModuleName = strModuleName_;
	nac::string_case<nac::TO_LOWER>(strModuleName);
}
void HookDll::WriteIAT(DWORD*ptLocation,DWORD ptValue)
{
	DWORD flOldProtect, flNewProtect, flDummy;
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(ptLocation, &mbi, sizeof(mbi));
	flNewProtect = mbi.Protect;
	flNewProtect &= ~(PAGE_READONLY | PAGE_EXECUTE_READ);
	flNewProtect |= (PAGE_READWRITE);
	VirtualProtect(ptLocation, sizeof(DWORD), flNewProtect, &flOldProtect );
	*ptLocation = ptValue;
	VirtualProtect(ptLocation, sizeof(DWORD), flOldProtect, &flDummy);
	FlushInstructionCache(GetModuleHandle(NULL),ptLocation,sizeof(DWORD));
}
bool HookDll::ApplyChanges(IMAGE_THUNK_DATA*pThunk,const dword_map&mpLookup) const
{
	if (!hModule) return false;
	while ( pThunk->u1.Function )
	{
		dword_map::const_iterator it = mpLookup.find(pThunk->u1.Function);
		if (it != mpLookup.end())
			WriteIAT(&(pThunk->u1.Function),it->second);
		++pThunk;
	}
	return true;
}
bool HookDll::InstallEntry(IMAGE_THUNK_DATA*pThunk) const
{
	return ApplyChanges(pThunk,mpModuleToRedirect);
}
bool HookDll::RemoveEntry(IMAGE_THUNK_DATA*pThunk) const
{
	return ApplyChanges(pThunk,mpRedirectToModule);
}
bool HookDll::UpdateMappings(void)
{
	HMODULE hNew = GetModuleHandleA(strModuleName.c_str());
	if (hNew != hModule)
	{
		std::cout << "Updating mappings for " << strModuleName << std::endl;
		hModule = hNew;
		mpModuleToRedirect.clear();
		mpRedirectToModule.clear();
		if (hModule)
		{
			for (func_map::iterator it = mpModuleFunctions.begin();it!=mpModuleFunctions.end();++it)
			{
				DWORD dwProcAddress = (DWORD)GetProcAddress(hModule,it->first.c_str());
				if (dwProcAddress)
				{
					mpModuleToRedirect[dwProcAddress] = it->second;
					mpRedirectToModule[it->second] = dwProcAddress;
				}
				else
				{
					std::cout << "error!!" << std::endl;
				}
			}
		}
		return true;
	}
	return false;
}
bool HookDll::SetRedirect(std::string strFunction,DWORD dwReplacement)
{
	func_map::iterator it = mpModuleFunctions.find(strFunction);
	if (it != mpModuleFunctions.end() || !dwReplacement) return false;
	mpModuleFunctions[strFunction] = dwReplacement;
	return true;
}

HookDll hkUser32("user32.dll");
HookDll hkKernel32("kernel32.dll");

int MyBox(HWND hwnd,LPCSTR text,LPCSTR title,UINT opt)
{
	//std::cout << "REDIRECTED: " << text << std::endl;
	//std::cout << "Real: " << (DWORD)hkUser32.Real(MyBox) << std::endl;
	return hkUser32.Real(MyBox)(NULL,text,"CHANGED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",MB_OK);
}
DWORD dwWEE;
FARPROC WINAPI MyProc(HMODULE hModule,LPCSTR lpProcName)
{
	std::cout << "dwWee: " << dwWEE << std::endl;
	FARPROC pfnRet = hkKernel32.Real(MyProc)(hModule,lpProcName);
	if ((DWORD)pfnRet == dwWEE)
	{
		std::cout << "Returning mybox" << std::endl;
		return (FARPROC)MyBox;
	}
	return pfnRet;
}
//intercept GetProcAddress, LoadLibraryA and LoadLibraryW
*/
nac::DllHook*phk_user32;
int WINAPI MyBox1(HWND hwnd,LPCSTR text,LPCSTR title,UINT opt)
{
	std::cout << "Box1: " << text << " - " << (DWORD)((*phk_user32)(MyBox1)) << std::endl;
	(*phk_user32)(MyBox1)(NULL,text,"Changed!",MB_OK);
	return 0;
}
int WINAPI MyBox2(HWND hwnd,LPCSTR text,LPCSTR title,UINT opt)
{
	std::cout << "Box2: " << text << " - " << (DWORD)((*phk_user32)(MyBox2)) << std::endl;
	((*phk_user32)(MyBox2))(NULL,text,"Changed again!",MB_OK);
	return 0;
}
int WINAPI MyBox3(HWND hwnd,LPCSTR text,LPCSTR title,UINT opt)
{
	std::cout << "Box3: " << text << " - " << (DWORD)((*phk_user32)(MyBox3)) << std::endl;
	((*phk_user32)(MyBox3))(NULL,text,"Changed again!",MB_OK);
	return 0;
}
// Need a built-in way to intercept GetProcess/LoadLibraryA/LoadLibraryW


/*
	If a DLL is Loaded, and it was previously not loaded, reapply the IAT changes

*/
int main()
{
	//will be the dll hInstance after testing
	nac::DllHook::Init(GetModuleHandle(NULL)+1);
	phk_user32 = nac::DllHook::Get("user32.dll");
	/*dwWEE = (DWORD)GetProcAddress(GetModuleHandleA("user32.dll"),"MessageBoxA");
	hkUser32.SetRedirect("MessageBoxA",(DWORD)MyBox);k
	hkKernel32.SetRedirect("GetProcAddress",(DWORD)MyProc);
*/
	std::cout << "MyBox1: " << (DWORD)MyBox1 << std::endl;
	std::cout << "MyBox2: " << (DWORD)MyBox2 << std::endl;
	std::cout << "MsBoxA: " << (DWORD)GetProcAddress(GetModuleHandleA("user32.dll"),"MessageBoxA") << std::endl;
	std::cout << phk_user32->SetRedirect("MessageBoxA",(DWORD)&MyBox1) << std::endl;
	std::cout << phk_user32->SetRedirect("MessageBoxA",(DWORD)&MyBox2) << std::endl;
	std::cout << phk_user32->SetRedirect("MessageBoxA",(DWORD)&MyBox3) << std::endl;
	nac::DllHook::Install();
	MessageBoxA(NULL,"Hello #1","Test Title",MB_OK);
	nac::DllHook::Install(false);
	MessageBoxA(NULL,"Hello #2","Test Title",MB_OK);
    return 0;
}
