#ifndef __PIPE_H__
#define __PIPE_H__

#include <windows.h>
#include <string>
#include <sstream>
namespace nac
{
	class Pipe
	{
		private:
			HANDLE hPipe;
			std::string strName;
			static std::string Qualify(const std::string&str);
			bool bServer;
			bool bConnected;
		public:
			static std::string GenerateName(DWORD dwPID=0);
			Pipe();
			~Pipe();
			bool Create(const std::string&strName_,DWORD dwBufferSize=1024);
			bool Open(const std::string&strName_);
			bool WaitForConnection(void);
			static bool WaitForServer(const std::string&strName,bool bForever=true);
			void Disconnect(void);
			void Close(void);
			bool Write(const unsigned char*ptBuf,DWORD dwBytes);
			DWORD Read(unsigned char*ptBuf,DWORD dwBytes);
			bool ReadAll(unsigned char*ptBuf,DWORD dwBytes);
	};
}
#endif
