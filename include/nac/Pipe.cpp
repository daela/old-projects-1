#include "Pipe.h"
namespace nac
{
	Pipe::Pipe()
	: hPipe(INVALID_HANDLE_VALUE),strName(""), bServer(false), bConnected(false)
	{ }

	Pipe::~Pipe()
	{
		Close();
	}

	std::string Pipe::Qualify(const std::string&str)
	{
		return std::string("\\\\.\\pipe\\") + str;
	}
	bool Pipe::Create(const std::string&strName_,DWORD dwBufferSize)
	{
		Close();
		hPipe = CreateNamedPipe(Qualify(strName_).c_str(),
					PIPE_ACCESS_DUPLEX,
					PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT,
					1,
					dwBufferSize,
					dwBufferSize,
					0,
					NULL);
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			strName = strName_;
			bServer=true;
			bConnected=false;
			return true;
		}
		return false;
	}
	bool Pipe::Open(const std::string&strName_)
	{
		Close();
		hPipe = CreateFile(Qualify(strName_).c_str(),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			strName = strName_;
			bServer=false;
			bConnected=true;
			return true;
		}
		return false;
	}
	bool Pipe::WaitForConnection(void)
	{
		if (bServer && hPipe != INVALID_HANDLE_VALUE)
			if (ConnectNamedPipe(hPipe, NULL)?TRUE:(GetLastError() == ERROR_PIPE_CONNECTED))
			{
				bConnected = true;
				return true;
			}
		return false;
	}
	bool Pipe::WaitForServer(const std::string&strName,bool bForever)
	{
		if (WaitNamedPipe(Qualify(strName).c_str(),(bForever?NMPWAIT_WAIT_FOREVER:NMPWAIT_USE_DEFAULT_WAIT)))
			return true;
		return false;
	}
	void Pipe::Disconnect(void)
	{
		if (bServer && bConnected && hPipe != INVALID_HANDLE_VALUE)
		{
			FlushFileBuffers(hPipe);
			DisconnectNamedPipe(hPipe);
			bConnected = false;
		}
	}
	void Pipe::Close(void)
	{
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			Disconnect();
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
			strName = "";
			bServer = false;
			bConnected = false;
		}
	}
	bool Pipe::Write(const unsigned char*ptBuf,DWORD dwBytes)
	{
		DWORD dwWritten;
		if (bConnected && hPipe != INVALID_HANDLE_VALUE)
			if (WriteFile(hPipe,ptBuf,dwBytes,&dwWritten,NULL))
				return true;
		return false;
	}
	DWORD Pipe::Read(unsigned char*ptBuf,DWORD dwBytes)
	{
		DWORD dwRead = 0;
		if (bConnected && hPipe != INVALID_HANDLE_VALUE)
			ReadFile(hPipe,ptBuf,dwBytes,&dwRead,NULL);
		//Because I'm blocking if there's no data, if dwRead is 0 then there's an error.
		return dwRead;
	}
	bool Pipe::ReadAll(unsigned char*ptBuf,DWORD dwBytes)
	{
		if (bConnected && hPipe != INVALID_HANDLE_VALUE)
		{
			DWORD dwCount = 0;
			DWORD dwTemp = 0;
			while (dwTemp = Read(ptBuf+dwCount,dwBytes-dwCount))
			{
				dwCount += dwTemp;
				if (dwCount == dwBytes)
				{
					return true;
				}
			}
		}
		return false;
	}
	std::string Pipe::GenerateName(DWORD dwPID)
	{
		if (dwPID == 0)
			dwPID = GetCurrentProcessId();
		std::stringstream ss;
		ss << "NAC_IPC_PIPE_" << dwPID;
		return ss.str();
	}
}
