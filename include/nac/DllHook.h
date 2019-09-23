#ifndef __DLLHOOK_H__
#define __DLLHOOK_H__

#include <map>
#include <set>
#include <queue>
#include <string>
#include <windows.h>
#include "strmanip.h"

namespace nac
{
	class DllHook
	{
		private:
			// various typedefs
			typedef std::map<DWORD,DWORD> dword_map;
			typedef std::pair<DWORD,DWORD> dword_pair;
			typedef std::map<std::string,DWORD> func_map;
			typedef std::pair<std::string,DWORD> func_pair;
			typedef std::map<std::string,DllHook> dll_map;
			typedef std::pair<std::string,DllHook> dll_pair;

			// private constructor
			DllHook(const std::string&strLibrary_);
			// handle to main process
			static HMODULE hModuleProcess;
			static HMODULE hModuleSelf;
			// list of intercepted libraries, indexed by name
			static dll_map mpLibraries;
			// list of built-in redirect originals to maintain order

//			static HMODULE WINAPI hkLoadLibraryA(LPCSTR lpFileName);
//			static HMODULE WINAPI hkLoadLibraryW(LPCWSTR lpFileName);
//			static FARPROC WINAPI hkGetProcAddress(HMODULE hModule,LPCSTR lpProcName);
			// requests write permissions to the memory prior to writing (crash prevention)
			static bool WriteMemory(DWORD*ptLocation,DWORD ptValue);

			// name of the module
			std::string	strLibrary;
			// module handle from last function mapping update, used to detect reloading
			HMODULE		hModule;
			// map of function names to their replacements
			func_map	mpModuleFunctions;
			// lookup map for converting from original module function to redirected function
			dword_map	mpModuleToRedirect;
			// lookup map for converting from redirected function to original module function
			dword_map	mpRedirectToModule;
			// lookup map for converting from a special function to it's user specified redirect
			std::set<DWORD> stTopChain;
			dword_map	mpRedirectChain;
			// adds two values together as DWORDs and returns as the type specified for T
			template<class T,class A,class B> static T MakePtr(A objA, B objB )
			{
				return (T)((DWORD)(objA)+(DWORD)(objB));
			}
			// maintains a valid function mapping, used for fixing imports after reloading
			bool Update(void);
			// static to call Update() for all hooks
			static void UpdateLibraries(void);
			// updates the import table with the appopriate function addresses
			void UpdateThunk(IMAGE_THUNK_DATA*pThunk,bool bInstall) const;

		public:
			// static constructor (automatically called)
			static void Init(HINSTANCE hInstance);
			// DllHook object creation/retrieval function
			static DllHook* Get(const std::string&strLibrary_);
			// hook installation/removal
			static bool Install(bool bInstall=true);

			// adds a function for redirection
			bool SetRedirect(const std::string&strFunction,const DWORD&dwReplacement);

			// retrieves the original function for a given replacement
			template<class T> T operator()(T pfnFunction) const
			{
				dword_map::const_iterator it;
				if ((it=mpRedirectChain.find((DWORD)pfnFunction)) == mpRedirectChain.end())
					return (T)NULL;
				if (stTopChain.find(it->second) != stTopChain.end())
				{
					if ((it=mpRedirectToModule.find(it->second)) == mpRedirectToModule.end())
						return (T)NULL;
				}
				return (T)it->second;
			}
	};


}

#endif
