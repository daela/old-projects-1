#ifndef INCLUDED_VENTRILO_CHAT_HPP
#define INCLUDED_VENTRILO_CHAT_HPP

#include <nx/os.hpp>

#include <vector>
#include <string>
#include <iostream>
#include "proxybot_common.hpp"
struct ventrilo_message
{
	Timestamp objTime;
	std::string strName;
	std::string strMessage;
	bool bPrivate;
	bool Load(const std::string&strLine);
	friend std::ostream&operator<<(std::ostream&os,const ventrilo_message&rhs);
};
std::ostream&operator<<(std::ostream&os,const ventrilo_message&rhs);

class ventrilo_chat
{
	#ifdef OS_WINDOWS_
	HWND hwnd;
	HWND hwndMessage;
	HWND hwndChat;
	HWND hwndSend;
	HWND hwndTTS;
	HWND hwndTimestamp;
	static void CheckTheBox(HWND hwndBox,const bool&state);
	#endif
	bool bMumble;

	public:
	ventrilo_chat(const bool mumble=false);
	private:

	std::vector<char> vcBuffer;
	public:
	typedef std::vector<char>::size_type bufsize_t;
	bool Find();
	bool CheckWindow();
	bool Send(const std::string&strText);
	bool GetText();
	bool GetLastLine(bufsize_t&uEnd,std::string&strLine) const;
};
#endif
