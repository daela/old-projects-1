#include "DllHook.h"

namespace nac
{
	HMODULE DllHook::hModuleProcess;
	HMODULE DllHook::hModuleSelf;
	DllHook::dll_map DllHook::mpLibraries;

	// static constructor
	void DllHook::Init(HINSTANCE hInstance)
	{
		hModuleSelf = (HMODULE)hInstance;
		DllHook::hModuleProcess = GetModuleHandle(NULL);
	}




	bool DllHook::WriteMemory(DWORD*ptLocation,DWORD ptValue)
	{
		DWORD flOldProtect, flNewProtect, flDummy;
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(ptLocation, &mbi, sizeof(mbi));
		flNewProtect = mbi.Protect;
		flNewProtect &= ~(PAGE_READONLY | PAGE_EXECUTE_READ);
		flNewProtect |= (PAGE_READWRITE);
		if (VirtualProtect(ptLocation,sizeof(DWORD),flNewProtect,&flOldProtect))
		{
			*ptLocation = ptValue;
			VirtualProtect(ptLocation, sizeof(DWORD), flOldProtect, &flDummy);
			FlushInstructionCache(hModuleProcess,ptLocation,sizeof(DWORD));
			return true;
		}
		return false;
	}

	DllHook::DllHook(const std::string&strLibrary_)
	{
		strLibrary=strLibrary_;
		hModule=NULL;
	}
	DllHook* DllHook::Get(const std::string&strLibrary_)
	{
		std::string strLibrary = strLibrary_;
		string_case<TO_LOWER>(strLibrary);
		// using find + insert instead of just insert to prevent useless calls to the constructor
		dll_map::iterator it = mpLibraries.find(strLibrary);
		if (it == mpLibraries.end())
			it = mpLibraries.insert(dll_pair(strLibrary,DllHook(strLibrary))).first;
		return &it->second;
	}
	bool DllHook::Update(void)
	{
		HMODULE hNew = GetModuleHandleA(strLibrary.c_str());
		if (hNew != hModule)
		{
			//std::cout << "Updating mappings for " << strModuleName << std::endl;
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
				}
			}
			return true;
		}
		return false;
	}
	void DllHook::UpdateLibraries(void)
	{
		for (dll_map::iterator it=mpLibraries.begin();it!=mpLibraries.end();++it)
			it->second.Update();
	}
	bool DllHook::SetRedirect(const std::string&strFunction,const DWORD&dwReplacement)
	{
		if (!dwReplacement) return false;
		func_map::iterator itFunc = mpModuleFunctions.find(strFunction);
		if (itFunc == mpModuleFunctions.end())
		{
			//first one calls second one calls third one calls original...
			mpModuleFunctions.insert(func_pair(strFunction,dwReplacement));
			mpRedirectChain.insert(dword_pair(dwReplacement,dwReplacement));
			stTopChain.insert(dwReplacement);
			return true;
		}
		DWORD dwChain = mpModuleFunctions[strFunction];
		if (dwChain == dwReplacement) return false;
		dword_map::iterator itChain;
		while ((itChain=mpRedirectChain.find(dwChain)) != mpRedirectChain.end())
		{
			dwChain = itChain->second;
			if (stTopChain.find(dwChain) != stTopChain.end())
			{
				itChain->second = dwReplacement;
				mpRedirectChain.insert(dword_pair(dwReplacement,dwChain));
				return true;
			}
			if (dwChain == dwReplacement) return false;
		}
		return false;
	}
	bool DllHook::Install(bool bInstall)
	{
		std::queue<std::string> quModules;
		std::set<std::string> stProcessed;
		UpdateLibraries();
		quModules.push("");
		while (!quModules.empty())
		{
			std::string strModuleName = quModules.front();
			quModules.pop();
			HMODULE hModule;
			if (strModuleName.empty())
				hModule=hModuleProcess;
			else
				hModule=GetModuleHandle(strModuleName.c_str());
			if (!hModule || hModule == hModuleSelf) continue;
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
					dll_map::iterator it = mpLibraries.find(strImportName);
					if (it != mpLibraries.end())
						it->second.UpdateThunk(MakePtr<PIMAGE_THUNK_DATA>(pDosHeader, pImportDesc->FirstThunk),bInstall);
					++pImportDesc;
			}
		}
		return true;
	}
	void DllHook::UpdateThunk(IMAGE_THUNK_DATA*pThunk,bool bInstall) const
	{
		if (hModule)
		{
			const dword_map& mpLookup = (bInstall?mpModuleToRedirect:mpRedirectToModule);
			while (pThunk->u1.Function)
			{
				dword_map::const_iterator it = mpLookup.find(pThunk->u1.Function);
				if (it != mpLookup.end())
					WriteMemory(&(pThunk->u1.Function),it->second);
				++pThunk;
			}
		}
	}
}
