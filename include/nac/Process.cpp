#include "Process.h"
namespace nac
{
	DWORD Process::GetParentPID(DWORD dwPID)
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

	DWORD WINAPI Process::SendThreadData(LPVOID lpParam)
	{
		pair_type&pr=*static_cast<pair_type*>(lpParam);
		if (pr.second->WaitForConnection())
		{
			DWORD dwDataSize = pr.first->size();
			//second condition keeps from accessing an empty vector's contents
			if (pr.second->Write((unsigned char*)&dwDataSize,sizeof(DWORD)) && (!dwDataSize || pr.second->Write(&((*(pr.first))[0]),dwDataSize)))
				return 1;
		}
		return 0;
	}

	bool Process::Suspend(void)
	{
		return (SuspendThread(pi.hThread) != (DWORD)-1);
	}
	bool Process::Resume(void)
	{
		return (ResumeThread(pi.hThread) != (DWORD)-1);
	}
	bool Process::Terminate(UINT uExitCode)
	{
		return (TerminateProcess(pi.hProcess,uExitCode) != 0);
	}
	bool Process::Running(void) const
	{
		DWORD dwExit;
		return (GetExitCodeThread(pi.hThread,&dwExit) && dwExit == STILL_ACTIVE);
	}

	DWORD Process::GetParentMessage(std::vector<unsigned char>&vcBuffer)
	{
		DWORD dwDataSize = 0;
		std::string strPipeName = nac::Pipe::GenerateName(nac::Process::GetParentPID());
		nac::Pipe cPipe;
		if (nac::Pipe::WaitForServer(strPipeName) && cPipe.Open(strPipeName))
		{
			//Retrieve the size
			if (cPipe.ReadAll((unsigned char*)&dwDataSize,sizeof(DWORD)))
			{
				if (dwDataSize)
				{
					vcBuffer.resize(dwDataSize);
					if (!cPipe.ReadAll(&(vcBuffer[0]),dwDataSize))
					{
						dwDataSize=0;
						vcBuffer.clear();
					}
				}
				else
					vcBuffer.clear();
			}
			cPipe.Close();
		}
		return dwDataSize;
	}
	Process::Process()
	{
		memset(&pi,0,sizeof(pi));
		memset(&si,0,sizeof(si));
		si.cb = sizeof(si);
	}
	Process::~Process()
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	bool Process::InjectDLL(std::string strDllFilePath,std::vector<unsigned char>*ptvcBuffer) const
	{
		HANDLE hProcess;
		LPVOID szRemoteDllName, pfnLoadLibrary;
		bool bRet = false;
		//this only works with Windows NT based systems (CreateRemoteThread)
		if ((GetVersion() < 0x80000000) && pi.dwProcessId)
		{
			hProcess = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_VM_OPERATION|SYNCHRONIZE,FALSE,pi.dwProcessId);
			if (hProcess)
			{
				if ((pfnLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA")) &&
					(szRemoteDllName = (LPVOID)VirtualAllocEx(hProcess, NULL, strDllFilePath.size(), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE)) )
				{
					HANDLE hThread;
					Pipe svPipe;
					pair_type pr(ptvcBuffer,&svPipe);
					if (!ptvcBuffer || svPipe.Create(nac::Pipe::GenerateName()))
					{
						HANDLE hDataThread = NULL;
						if (!ptvcBuffer || (hDataThread = CreateThread(NULL,0,SendThreadData,(LPVOID)&pr,0,NULL)))
						{
							if (WriteProcessMemory(hProcess, (LPVOID)szRemoteDllName, strDllFilePath.c_str(),strDllFilePath.size(), NULL) &&
								(hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pfnLoadLibrary, (LPVOID)szRemoteDllName, 0, NULL)))
							{
								DWORD dwExit = 0;

								WaitForSingleObject(hThread,INFINITE);
								if (GetExitCodeThread(hThread,&dwExit) && dwExit != 0)
									bRet = true;
								CloseHandle(hThread);
							}
							if (ptvcBuffer)
							{
								TerminateThread(hDataThread,0);
								CloseHandle(hDataThread);
								svPipe.Close();
							}
						}
					}
					VirtualFreeEx(hProcess,szRemoteDllName,0,MEM_RELEASE);
				}
				CloseHandle(hProcess);
			}
		}
		return bRet;
	}
}
