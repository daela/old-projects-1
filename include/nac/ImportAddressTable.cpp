#include "ImportAddressTable.h"
namespace nac
{
	ImportAddressTable::IAT_Function::IAT_Function(std::string strLibrary_,std::string strFunction_)
	: strLibrary(strLibrary_), strFunction(strFunction_)
	{ }

	bool ImportAddressTable::IAT_Function::operator<(const IAT_Function&obj) const
	{
		if (strLibrary < obj.strLibrary) return true;
		if (obj.strLibrary < strLibrary) return false;
		return (strFunction < obj.strFunction);
	}

	ImportAddressTable::IAT_FuncData::IAT_FuncData(DWORD pfnOriginal_,DWORD pfnRedirect_)
	: pfnOriginal(pfnOriginal_), pfnRedirect(pfnRedirect_)
	{
			ptFuncInfo=NULL;
	}

	void ImportAddressTable::IAT_FuncData::Install()
	{
		for (std::set<DWORD*>::iterator it=stImport.begin();it!=stImport.end();++it)
		{
			WriteIAT(*it,pfnRedirect);
		}
	}

	void ImportAddressTable::IAT_FuncData::Restore()
	{
		for (std::set<DWORD*>::iterator it=stImport.begin();it!=stImport.end();++it)
		{
			WriteIAT(*it,pfnOriginal);
		}
	}
	template<class T,class A,class B> T ImportAddressTable::MakePtr( A objA, B objB )
	{
		return (T)((DWORD)(objA)+(DWORD)(objB));
	}



	void ImportAddressTable::WriteIAT(DWORD*ptr,DWORD val)
	{
		DWORD flOldProtect, flNewProtect, flDummy;
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(ptr, &mbi, sizeof(mbi));
		flNewProtect = mbi.Protect;
		flNewProtect &= ~(PAGE_READONLY | PAGE_EXECUTE_READ);
		flNewProtect |= (PAGE_READWRITE);
		VirtualProtect(ptr, sizeof(PVOID), flNewProtect, &flOldProtect );
		*ptr = val;
		VirtualProtect(ptr, sizeof(PVOID), flOldProtect, &flDummy);
		FlushInstructionCache(GetModuleHandle(NULL),ptr,sizeof(PVOID));
	}

	bool ImportAddressTable::LoadImports(HMODULE hModule)
	{
		IMAGE_DOS_HEADER *pDosHeader;
		IMAGE_NT_HEADERS *pNTHeader;
		IMAGE_IMPORT_DESCRIPTOR *pImportDesc;
		pDosHeader = (PIMAGE_DOS_HEADER)hModule;

		// Tests to make sure we're looking at a module image (the 'MZ' header)
		if ( IsBadReadPtr(pDosHeader, sizeof(IMAGE_DOS_HEADER)) )
			return false;
		if ( pDosHeader->e_magic != IMAGE_DOS_SIGNATURE )
			return false;

		// The MZ header has a pointer to the PE header
		pNTHeader = MakePtr<PIMAGE_NT_HEADERS>(pDosHeader, pDosHeader->e_lfanew);
		// More tests to make sure we're looking at a "PE" image
		if ( IsBadReadPtr(pNTHeader, sizeof(IMAGE_NT_HEADERS)) )
			return false;
		if ( pNTHeader->Signature != IMAGE_NT_SIGNATURE )
			return false;

		// We now have a valid pointer to the module's PE header.
		// Now get a pointer to its imports section
		pImportDesc = MakePtr<PIMAGE_IMPORT_DESCRIPTOR>(
			pDosHeader,
			pNTHeader->OptionalHeader.DataDirectory
			[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		//Go out if imports table doesn't exist
		if ( pImportDesc <= (PIMAGE_IMPORT_DESCRIPTOR)pNTHeader )
			return false; //pImportDesc will ==pNTHeader.

		while ( pImportDesc->Name ) //Name is a DWORD (RVA, to a DLL name)
		{

			IMAGE_THUNK_DATA *pThunk = MakePtr<PIMAGE_THUNK_DATA>(pDosHeader, pImportDesc->FirstThunk);
			std::string strDllName = MakePtr<char*>(pDosHeader, pImportDesc->Name);
			string_case<TO_LOWER>(strDllName);
			if (pThunk->u1.Function)
			{
				bool bRecurse = (stLibraries.find(strDllName) == stLibraries.end() && stIgnoreLibraries.find(strDllName) == stIgnoreLibraries.end());
				stLibraries.insert(strDllName);
				if (bRecurse)
					LoadImports(GetModuleHandle(strDllName.c_str()));
			}
			while ( pThunk->u1.Function )
			{
				IAT_FuncLookupMap::iterator it = mpFunctionLookup.find(pThunk->u1.Function);
				if (it != mpFunctionLookup.end())
					it->second->stImport.insert(&(pThunk->u1.Function));
				pThunk++;
			}
			pImportDesc++;  // Advance to next imported module descriptor
		}
		return true;
	}

	ImportAddressTable::ImportAddressTable()
	{
		hmStart = GetModuleHandle(NULL);

		//LoadImports(hmStart);
	}
	ImportAddressTable::ImportAddressTable(HMODULE hm_start)
	{
		hmStart = hm_start;
		//LoadImports(hmStart);
	}

	unsigned short ImportAddressTable::AddRedirect(std::string strLibrary,std::string strFunction,DWORD pfnRedirect)
	{
		string_case<TO_LOWER>(strLibrary);
		if (!pfnRedirect)
			return 1;
		IAT_Function iatFunc(strLibrary,strFunction);
		if (mpFunctions.find(iatFunc) != mpFunctions.end())
			return 2;

		HMODULE hModule = GetModuleHandleA(strLibrary.c_str());
		if (!hModule) return 3;
		DWORD pfnOriginal = (DWORD)GetProcAddress(hModule,strFunction.c_str());
		if (!pfnOriginal) return 4;
		IAT_FuncMap::iterator it = mpFunctions.insert(IAT_FuncMapPair(iatFunc,IAT_FuncData(pfnOriginal,pfnRedirect))).first;
		it->second.ptFuncInfo = const_cast<IAT_Function*>(&(it->first));
		mpOrigFunctionLookup[pfnRedirect] = mpFunctionLookup[pfnOriginal] = const_cast<IAT_FuncData*>(&(it->second));
		return 0;
	}
	void ImportAddressTable::AddIgnore(std::string strLibrary)
	{
		string_case<TO_LOWER>(strLibrary);
		stIgnoreLibraries.insert(strLibrary);
	}
	bool ImportAddressTable::PrepareImports()
	{
		static bool bRanBefore = false;
		if (!bRanBefore)
		{
			bRanBefore=true;
			return LoadImports(hmStart);
		}
		return false;
	}
	void ImportAddressTable::Install()
	{
		for (IAT_FuncMap::iterator it = mpFunctions.begin();it!=mpFunctions.end();++it)
			it->second.Install();
	}
	void ImportAddressTable::Restore()
	{
		for (IAT_FuncMap::iterator it = mpFunctions.begin();it!=mpFunctions.end();++it)
			it->second.Restore();
	}
}
