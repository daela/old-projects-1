#include "ProcessLauncher.h"
namespace nac
{
	ProcessLauncher::ProcessLauncher(std::string strFilePath_,std::string strDirectory_)
	: strFilePath(strFilePath_), strDirectory(strDirectory_)
	{
		if (strDirectory.empty())
		{
			std::string strTemp;
			split_path(strFilePath,strDirectory,strTemp);
		}
	}


	bool ProcessLauncher::Launch(Process&objProc,const std::string& strParameters,bool bSuspended) const
	{
		std::string strPath, strFile, strCommandLine;
		objProc = Process();
		split_path(strFilePath,strPath,strFile);
		strCommandLine = std::string("\"")+strFilePath+std::string("\"");
		if (!strParameters.empty())
			strCommandLine += std::string(" ")+strParameters;
		if (!CreateProcess(strFilePath.c_str(),
				const_cast<char*>(strCommandLine.c_str()),
				NULL,NULL,FALSE,CREATE_DEFAULT_ERROR_MODE | (bSuspended?CREATE_SUSPENDED:0),
				NULL,(strDirectory.empty()?NULL:strDirectory.c_str()),
				&(objProc.si),&(objProc.pi)))
		{
			objProc = Process();
			return false;
		}
		return true;
	}

}
