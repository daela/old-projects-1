#ifndef __EVENT_H__
#define __EVENT_H__

#include <windows.h>
#include <string>
namespace nac
{
	class Event
	{
		private:
			HANDLE hEvent;
			std::string strName;
		public:
			Event();
			~Event();
			bool Create(const std::string&strName_);
			bool Open(const std::string&strName_);
			bool Wait(DWORD dwMilliseconds=INFINITE);
			bool Signal(void);
			void Close(void);
	};
}
#endif
