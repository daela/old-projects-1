#ifndef __IMPORTADDRESSTABLE_H__
#define __IMPORTADDRESSTABLE_H__

#include <windows.h>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include "strmanip.h"
namespace nac
{
	class ImportAddressTable
	{
		private:
			class IAT_Function
			{
				public:
					std::string strLibrary;
					std::string strFunction;

					IAT_Function(std::string strLibrary_,std::string strFunction_);

					bool operator<(const IAT_Function&obj) const;
			};
			class IAT_FuncData
			{
				public:
					DWORD pfnOriginal;
					DWORD pfnRedirect;
					std::set<DWORD*> stImport;
					IAT_Function*ptFuncInfo;
					IAT_FuncData(DWORD pfnOriginal_,DWORD pfnRedirect_);
					void Install();
					void Restore();
			};

			typedef std::map<IAT_Function,IAT_FuncData> IAT_FuncMap;
			typedef std::pair<IAT_Function,IAT_FuncData> IAT_FuncMapPair;
			typedef std::map<DWORD,IAT_FuncData*> IAT_FuncLookupMap;
			typedef std::pair<DWORD,IAT_FuncData*> IAT_FuncLookupMapPair;
			IAT_FuncMap mpFunctions;
			IAT_FuncLookupMap mpFunctionLookup;
			IAT_FuncLookupMap mpOrigFunctionLookup;
			std::set<std::string> stLibraries;
			std::set<std::string> stIgnoreLibraries;
			HMODULE hmStart;

			template<class T,class A,class B> static T MakePtr( A objA, B objB );

			static void WriteIAT(DWORD*ptr,DWORD val);



			bool LoadImports(HMODULE hModule);

		public:
			static DWORD GetParentPID(DWORD dwPID);

			ImportAddressTable();
			ImportAddressTable(HMODULE hm_start);

			unsigned short AddRedirect(std::string strLibrary,std::string strFunction,DWORD pfnRedirect);
			void AddIgnore(std::string strLibrary);
			bool PrepareImports();
			void Install();
			void Restore();
			template<class T> T Real(T pfnRedirected) const
			{
				IAT_FuncLookupMap::const_iterator it = mpOrigFunctionLookup.find((DWORD)pfnRedirected);
				if (it != mpOrigFunctionLookup.end())
				{
				return (T)(it->second->pfnOriginal);
				}
				return NULL;
			}


	};
}
#endif
