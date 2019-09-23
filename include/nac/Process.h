#ifndef __PROCESS_H__
#define __PROCESS_H__
#include <windows.h>
#include <Tlhelp32.h>
#include <vector>
#include <string>
#include <map>
#include "Pipe.h"

namespace nac
{

	class Process
	{
		private:
			typedef std::pair<std::vector<unsigned char>*,Pipe*> pair_type;
			static DWORD WINAPI SendThreadData(LPVOID lpParam);
			PROCESS_INFORMATION pi;
			STARTUPINFO si;
		public:
			static DWORD GetParentPID(DWORD dwPID=0);
			static DWORD GetParentMessage(std::vector<unsigned char>&vcBuffer);
			Process();
			~Process();
			bool Suspend(void);
			bool Resume(void);
			bool Terminate(UINT uExitCode=0);
			bool Running(void) const;
			bool InjectDLL(std::string strDllFilePath,std::vector<unsigned char>*ptvcBuffer=NULL) const;
			friend class ProcessLauncher;
	};

}

#endif

