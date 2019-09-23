#include "ventrilo_chat.hpp"

// once upon a time I tried to make this work for mumble on windows.... dead code lives here from that endeavor
// QT makes things like this hard!

bool ventrilo_message::Load(const std::string&strLine)
{

	// ventrilo
	//	01/16 22:09:01 DJ proxitar: a

	// mumble
	// 
	std::string::size_type uPos=0;
	bPrivate=false;
	bool bMumble=(strLine[0] == '[');
	if (objTime.Load(strLine,false))
	{
		// find the first space
		uPos=strLine.find(' ');
	
		if (uPos != std::string::npos)
		{
			// find the next space
			uPos=strLine.find(' ',uPos+1);
			if (uPos != std::string::npos)
			{       
				++uPos;
			}
		}
		// check to see if there's a room specifier
		if (bMumble)
		{
			bPrivate=true;
			if (uPos != std::string::npos)
			{
				if (strLine[uPos] == '(')
				{
					bPrivate=false;
					uPos = strLine.find(')',uPos+1);
					if (uPos != std::string::npos) ++uPos;
					if (uPos != std::string::npos) ++uPos; // skip the space too
				}	
			}
		}
		if (uPos == std::string::npos)
		{
			return false;
		}

	}
	else
	{
		objTime=Timestamp();
	}
	std::string::size_type uColonPos=strLine.find(':',uPos);
	if (uColonPos != std::string::npos)
	{
		strName = strLine.substr(uPos,uColonPos-uPos);
		uColonPos+=2;
	}
	else
	{
		strName="";
		uColonPos=uPos;
	}

	strMessage=strLine.substr(uColonPos);
	return true;
}
std::ostream&operator<<(std::ostream&os,const ventrilo_message&rhs)
{
	os << "[" << rhs.objTime << "] " << rhs.bPrivate << " " << rhs.strName << ": " << rhs.strMessage;
	return os;
}
#ifdef OS_WINDOWS_
void ventrilo_chat::CheckTheBox(HWND hwndBox,const bool&state)
{
	if (SendMessage(hwndBox,BM_GETCHECK,0,0)!=(state?BST_CHECKED:BST_UNCHECKED))
		SendMessage(hwndBox,BM_SETCHECK,(WPARAM)(state?BST_CHECKED:BST_UNCHECKED),0);
}
#endif
ventrilo_chat::ventrilo_chat(const bool mumble)
{
	#ifdef OS_WINDOWS_
	hwnd=hwndMessage=hwndChat=hwndSend=hwndTTS=hwndTimestamp=NULL;
	#endif
	bMumble=mumble;
}
bool ventrilo_chat::Find()
{
	#ifdef OS_WINDOWS_
	std::cout << "bMumble: " << bMumble << std::endl;
	if (bMumble)
	{
		hwnd=FindWindowA("QWidget","Mumble -- DF Proximity Bot");
		char buf[255];
		do
		{
			if (hwnd)
			{
				GetWindowTextA(hwnd,buf,sizeof(buf));
				if (strncmp("Mumble -- ",buf,10) == 0)
				{
					hwndMessage=FindWindowExA(hwnd,NULL,"QWidget","qdwChat");
					if (hwndMessage)
					{
						HWND hTmp =FindWindowExA(hwndMessage,NULL,"QWidget","qleChat");
						if (!hTmp)
							std::cout << "Mumble chat box internal subwindow not found; using parent..." << std::endl;
						else
							hwndMessage=hTmp;
					}

					// TODO: make the chat window actually scrapable so we can give commands!
					hwndChat=FindWindowExA(hwnd,NULL,"QWidget","qdwLog");
					if (hwndChat)
						hwndChat=FindWindowExA(hwndChat,NULL,"QWidget","qteLog");
					if (hwndChat)
						hwndChat=FindWindowExA(hwndChat,NULL,"QWidget","qt_scrollarea_viewport");

					if (hwndMessage)
					{
						SetWindowText(hwnd,"Mumble -- DF Proximity Bot");
						return true;
					}
				}
			}
			std::cout << "!!" << buf << std::endl;
		} while ((hwnd=FindWindowExA(NULL,hwnd,"QWidget",NULL)));
	}
	else
	{
		// see if we marked one already
		hwnd=FindWindowA("#32770","Chat - DF Proximity Bot");
		do
		{
			if (hwnd)
			{
				HWND hwndParent=GetParent(hwnd);
				hwndMessage=FindWindowExA(hwnd,NULL,"Edit",NULL);
				hwndChat=FindWindowExA(hwnd,NULL,"RichEdit20A",NULL);
				hwndSend=FindWindowExA(hwnd,NULL,"Button","Send");
				hwndTTS=FindWindowExA(hwnd,NULL,"Button","Enable TTS");	
				hwndTimestamp=FindWindowExA(hwnd,NULL,"Button","Time stamp");	
				
				if (!hwndParent && hwndMessage && hwndChat && hwndSend && hwndTTS && hwndTimestamp)
				{
					SetWindowText(hwnd,"Chat - DF Proximity Bot");
					return true;
				}
			}
		} while ((hwnd=FindWindowExA(NULL,hwnd,"#32770","Chat")));
	}
	return false;
	#else
	return true;
	#endif
}
bool ventrilo_chat::CheckWindow()
{
	#ifdef OS_WINDOWS_
	if (!hwnd || (!bMumble && hwnd != FindWindowA("#32770","Chat - DF Proximity Bot")) || (bMumble && hwnd != FindWindowA("QWidget","Mumble -- DF Proximity Bot")))
		return Find();
	return true;
	#else
	return true;
	#endif
}
bool ventrilo_chat::Send(const std::string&strText)
{
	#ifdef OS_WINDOWS_
	if (CheckWindow())
	{
		if (!bMumble)
		{
			SendMessage(hwndMessage,WM_SETTEXT,0,(LPARAM)strText.c_str());
			SendMessage(hwndSend,BM_CLICK,0,0);
		}
		else
		{
			// To the start of the box
			//SetForegroundWindow(hwndMessage);
			SendMessage(hwndMessage, WM_LBUTTONDOWN, 0, (LPARAM)0);
			SendMessage(hwndMessage, WM_ACTIVATE, WA_ACTIVE, (LPARAM)hwndMessage);
			SendMessage(hwndMessage, WM_KEYDOWN, VK_HOME, (LPARAM)0);
			SendMessage(hwndMessage, WM_KEYUP, VK_HOME, (LPARAM)0);
			for (unsigned int i=0;i<500;++i)
			{
				// Delete everything
				SendMessage(hwndMessage, WM_KEYDOWN, VK_DELETE, (LPARAM)0);
				SendMessage(hwndMessage, WM_KEYUP, VK_DELETE, (LPARAM)0);
			}

			for (std::string::const_iterator it=strText.begin();it!=strText.end();++it)
			{
				SendMessage(hwndMessage, WM_CHAR, *it, (LPARAM)0);
			}
			SendMessage(hwndMessage, WM_KEYDOWN, VK_RETURN, (LPARAM)0);
			SendMessage(hwndMessage, WM_KEYUP, VK_RETURN, (LPARAM)0);
		}
	}
	std::cout << "Sending Text: " << strText << std::endl;
	#endif

	std::string::size_type posStart=0;
	std::string::size_type posEnd;
	std::string strEscapedText;

	while(true)
	{
		posEnd=strText.find('"',posStart);
		if (posEnd == std::string::npos)
			strEscapedText += strText.substr(posStart);
		else
			strEscapedText += strText.substr(posStart,posEnd-posStart);
		if (posEnd == std::string::npos)
			break;
		strEscapedText += "\\\"";
		posStart=posEnd+1;
	}	
	std::string strOutput=std::string("./scripts/send_voice_message.sh \"") + strEscapedText + "\""; // security issue if this text can have ; and such
	system(strOutput.c_str());
	std::cout << strEscapedText << std::endl;
	return true;
}
bool ventrilo_chat::GetText()
{
	#ifdef OS_WINDOWS_
	if (!bMumble)
	{
		if (CheckWindow())
		{
			unsigned int uLength=SendMessage(hwndChat,WM_GETTEXTLENGTH,0,0);
			++uLength;
			if (uLength > vcBuffer.capacity())
				vcBuffer.reserve(uLength);
			vcBuffer.resize(uLength);
			if (!vcBuffer.empty())
				vcBuffer.resize(SendMessage(hwndChat,WM_GETTEXT,(WPARAM)uLength,(LPARAM)&vcBuffer[0]));
			
			return true;
		}
	}
	#endif
	return false;
}
bool ventrilo_chat::GetLastLine(ventrilo_chat::bufsize_t&uEnd,std::string&strLine) const
{
	#ifdef OS_WINDOWS_
	if (uEnd > vcBuffer.size())
		uEnd=vcBuffer.size();
	// first, find the first non-whitespace char
	bool allWS=true;
	while (allWS && uEnd--)
	{
		switch (vcBuffer[uEnd])
		{
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				break;
			default:
				allWS=false;
		}
	}
	// if we found a character
	if (!allWS)
	{
		bufsize_t uStart=uEnd;
		bool bDone=false;
		while (uStart)
		{
			switch (vcBuffer[uStart-1])
			{
				case '\r':
				case '\n':
					bDone=true;
			}
			if (bDone) break;
			--uStart;
		}
		strLine.assign(&vcBuffer[uStart],(&vcBuffer[uEnd])+1);
		uEnd=uStart;
		return true;
	}
	#endif
	return false;
}
