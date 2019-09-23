#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
        #define OS_WINDOWS_ 1
#elif defined(__linux) || defined(__linux__) || defined(linux) || defined(__gnu_linux__) || defined(LINUX)
        #define OS_LINUX_ 1
#elif defined(__APPLE__)
        #define OS_MAC_ 1
#elif defined (__SVR4) && defined (__sun)
        #define OS_SOLARIS_ 1
#else
        #define OS_OTHER_ 1
#endif

#ifndef OS_WINDOWS_
#include "mumble_wrapper.hpp"
#include <nx/thread.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include "ventrilo_chat.hpp"

int child_reader_thread::main(void*ptHandle)
{
	if (ptHandle!= NULL)
	{
		join(*reinterpret_cast<nx::thread::handle*>(ptHandle));
		objQueue.push("");
		return 0;
	}
	int fd1[2];
	int fd2[2];
	pid_t pid;

	if ( (pipe(fd1) < 0) || (pipe(fd2) < 0) )
	{
		return -2;
	}
	if ( (pid = fork()) < 0 )
	{
		return -3;
	}
	else  if (pid == 0)     // CHILD PROCESS
	{
		close(fd1[1]);
		close(fd2[0]);

		if (fd1[0] != STDIN_FILENO)
		{
			if (dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO)
			{
				std::cerr << "dup2 error to stdin" << std::endl;
			}
			close(fd1[0]);
		}

		if (fd2[1] != STDOUT_FILENO)
		{
			if (dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
			{
				std::cerr << "dup2 error to stdout" << std::endl;
			}
			close(fd2[1]);
		}

		if ( execl("/bin/bash", "bash", "-c", strCommand.c_str(), NULL) < 0 )
		{
			std::cerr << "exec error for " << strCommand << std::endl;
			return -4;
		}

		return 0;
	}
	else        // PARENT PROCESS
	{
		int rv;
		close(fd1[0]);
		close(fd2[1]);

		const unsigned int blocksize=255;
		std::string strOutput;
		std::string::size_type sz = strOutput.size();
		strOutput.resize(sz+blocksize);
		while ((rv = read(fd2[0], (&strOutput[0])+sz, blocksize)) > 0 )
		{
			
			
			std::string::size_type posStart=0;
			std::string::size_type pos=sz;
			bool gotOne=false;
			sz += rv;

			for (;pos < sz;++pos)
			{
				if (strOutput[pos] == '\n')
				{
					std::string strLine=strOutput.substr(posStart,pos-posStart-(pos && strOutput[pos-1] == '\r'));
					if (!strLine.empty())
					{
						objQueue.push(strLine);
					}

					posStart=pos+1;
					gotOne=true;
				}
			}
			if (gotOne)
			{
				strOutput.erase(0,posStart);
				sz -= posStart;
			}
			strOutput.resize(sz+blocksize);
		}
		if (sz)
		{
			strOutput.resize(sz);
			//std::cout << "Leftovers: " << strOutput << std::endl;
		}

		close(fd1[1]); // close stdin to the child
		close(fd2[0]); // close stdout of the child
		return 0;
	}
	return 0;
}

// default constructor for main app, always.  could have thread spawning built in somehow?
int mumble_wrapper::main(void*ptData)
{
	//for (arg_vector::iterator it=m_arguments.begin();it!=m_arguments.end();++it)
	//{
	//	std::cout << "Value: " << *it << std::endl;
	//}

	nx::thread::handle hReader,hMonitor;

	child_reader_thread objThread;
	objThread.strCommand="unbuffer mumble 2>&1";
	// spawn the reader
	objThread.spawn(hReader,NULL);
	// spawn the monitor that sends a blank message when the reader dies
	objThread.spawn(hMonitor,(void*)&hReader);

	std::string strLine;

	bool bProcessed=true;
	std::string strMessage;
	std::map<std::string,std::string> mpMessage;
	std::map<std::string,std::string> mpSessionToName;
	std::map<std::string,std::string> mpChannelToName;
	while (true)
	{
		objThread.objQueue.pop(strLine);
		if (strLine.empty()) break;


		if (*strLine.rbegin() == ':')
		{
			bProcessed=false;
			mpMessage.clear();
			strMessage=strLine.substr(0,strLine.size()-1);
		}
		else if (!strMessage.empty())
		{
			std::string::size_type pos=strLine.find(':');
			if (pos == std::string::npos) continue;

			std::string strKey=strLine.substr(0,pos);
			pos=strLine.find_first_not_of(" \t\"",pos+1);
			if (pos == std::string::npos) continue;

			std::string::size_type posEnd;
			posEnd=strLine.find_last_not_of(" \t\"");
			std::string strValue=strLine.substr(pos,posEnd-pos+1);
			mpMessage[strKey] = strValue; 


			// handlers

			if (!bProcessed)
			{
				if (strMessage == "UserState")
				{
					if (mpMessage.find("name")!=mpMessage.end() && mpMessage.find("session") != mpMessage.end())
					{
						mpSessionToName[mpMessage["session"]] = mpMessage["name"];
						bProcessed=true;
					}
				}
				else if (strMessage == "ChannelState")
				{
					if (mpMessage.find("name")!=mpMessage.end() && mpMessage.find("channel_id") != mpMessage.end())
					{
						mpChannelToName[mpMessage["channel_id"]] = mpMessage["name"];
						bProcessed=true;
					}
				}
				else if (strMessage == "TextMessage")
				{
					if (mpMessage.find("actor")!=mpMessage.end() && mpMessage.find("channel_id") != mpMessage.end() && mpMessage.find("message") != mpMessage.end())
					{
						ventrilo_message objMessage;
						objMessage.strName=mpSessionToName[mpMessage["actor"]];
						objMessage.strMessage=mpMessage["message"];
						objMessage.bPrivate=false;

						string_to_lower(objMessage.strName);
						string_to_lower(objMessage.strMessage);
						objMessageQueue.push(objMessage);	

						bProcessed=true;
					}
				}
			}
		}	
	}
	nx::thread::join(hMonitor);
	// monitor thread already joined reader
	
	return 0; 
}
#endif

