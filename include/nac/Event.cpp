#include "Event.h"
namespace nac
{
	Event::Event() : hEvent(NULL), strName("")
	{ }
	Event::~Event()
	{
		Close();
	}
	void Event::Close(void)
	{
		if (hEvent)
		{
			CloseHandle(hEvent);
			hEvent = NULL;
			strName = "";
		}
	}
	bool Event::Create(const std::string& strName_)
	{
		Close();
		hEvent = CreateEvent(NULL,FALSE,FALSE,strName_.c_str());
		if (hEvent)
		{
			strName = strName_;
			return true;
		}
		return false;
	}
	bool Event::Open(const std::string& strName_)
	{
		Close();
		hEvent = OpenEvent(EVENT_MODIFY_STATE|SYNCHRONIZE,FALSE,strName_.c_str());
		if (hEvent)
		{
			strName = strName_;
			return true;
		}
		return false;
	}
	bool Event::Wait(DWORD dwMilliseconds)
	{
		return (hEvent && WaitForSingleObject(hEvent,dwMilliseconds) == WAIT_OBJECT_0);
	}
	bool Event::Signal(void)
	{
		return (hEvent && SetEvent(hEvent));
	}
}
