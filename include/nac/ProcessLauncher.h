#ifndef __PROCESSLAUNCHER_H__
#define __PROCESSLAUNCHER_H__
#include <windows.h>
#include <string>
#include "strmanip.h"
#include "Process.h"
namespace nac
{

	class ProcessLauncher
	{
		private:
			std::string strFilePath;
			std::string strDirectory;
		public:
			ProcessLauncher(std::string strFilePath_,std::string strDirectory_="");
			bool Launch(Process&objProc,const std::string& strParameters="",bool bSuspended=false) const;
	};

}

#endif
