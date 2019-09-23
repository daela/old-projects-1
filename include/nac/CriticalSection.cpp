#include "CriticalSection.h"

namespace nac
{
	CriticalSection::CriticalSection()
	{
		InitializeCriticalSection(&cSec);
	}
	CriticalSection::~CriticalSection()
	{
		DeleteCriticalSection(&cSec);
	}
	void CriticalSection::Enter()
	{
		EnterCriticalSection(&cSec);
	}
	void CriticalSection::Leave()
	{
		LeaveCriticalSection(&cSec);
	}
}
