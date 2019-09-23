#ifndef INCLUDED_MUMBLE_WRAPPER_HPP
#define INCLUDED_MUMBLE_WRAPPER_HPP

//should use forkpty instead of "unbuffer" doing it for me.

//http://www.cyberciti.biz/tips/howto-linux-increase-pty-session.html
#include <nx/os.hpp>
#ifndef OS_WINDOWS_
#include <nx/thread.hpp>
#include <nx/safe_queue.hpp>
#include <nx/time.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include "ventrilo_chat.hpp"


class child_reader_thread : public nx::thread
{
	public:
	nx::safe_queue<std::string> objQueue;
	std::string strCommand;	
	int main(void*ptHandle);
};
// default constructor for main app, always.  could have thread spawning built in somehow?
class mumble_wrapper : public nx::thread
{
	
	public:
	nx::safe_queue<ventrilo_message> objMessageQueue;
	int main(void*ptData);
};
#endif

#endif

