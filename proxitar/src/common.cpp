#include <nx/common.hpp>
#include <nx/numeric/toa.hpp>
#ifndef OS_WINDOWS_
#include <sys/stat.h>
#include <unistd.h> //readlink
#include <fcntl.h> // open
#include <errno.h>
#include <time.h>
#include <signal.h>
#endif
namespace nx
{
	bool dead_process(const pid_t&pid)
	{
		#ifdef OS_WINDOWS_
		HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,pid);
		if (hProcess)
		{
			DWORD dwExit;
			if (GetExitCodeProcess(hProcess,&dwExit))
			{
				CloseHandle(hProcess);
				return (dwExit != STILL_ACTIVE);
			}
			CloseHandle(hProcess);
			return false;
		}
		return true;
		#else
		char ptBuf[50];
		ptBuf[0]='/';
		ptBuf[1]='p';
		ptBuf[2]='r';
		ptBuf[3]='o';
		ptBuf[4]='c';
		ptBuf[5]='/';
		char*ptSuffix=ptBuf+6+nx::numeric::toa(static_cast<uintpid_t>(pid),ptBuf+6);
		ptSuffix[0]='/';
		ptSuffix[1]='s';
		ptSuffix[2]='t';
		ptSuffix[3]='a';
		ptSuffix[4]='t';
		ptSuffix[5]='u';
		ptSuffix[6]='s';
		ptSuffix[7]='\0';
		// try to open status and check if it's a zombie
		int fd=::open(ptBuf,O_RDONLY,(mode_t)0400);
		if (fd != -1)
		{
			unsigned int uMatch=0, length, i;
			const char ptMatch[] = { '\n', 'S', 't', 'a', 't', 'e', ':' };
			while ((length=read(fd,ptBuf,sizeof(ptBuf))))
			{
				for (i=0;i<length;++i)
				{
					// if we're using sched, we must match our prefix first
					if (uMatch < sizeof(ptMatch))
					{
						// if it matches, increment the match counter; otherwise, start over
						if (ptBuf[i] == ptMatch[uMatch]) ++uMatch;
						else uMatch=0;
					}
					else
					{
						if ((ptBuf[i] >= 'a' && ptBuf[i] <= 'z') || (ptBuf[i] >= 'A' && ptBuf[i] <= 'Z'))
						{
							::close(fd);
							return (ptBuf[i] == 'z' || ptBuf[i] == 'Z');
						}
					}
						
				}
			}
			::close(fd);
		}
		// if we couldn't open status, or never found our match in status.. we failed to do the zombie check
		// so we'll fall back on two other methods
		struct stat st;
		ptSuffix[0]='\0';
		return (::kill(pid,0) != 0 && ::stat(ptBuf,&st) != 0);
		#endif
	}

	const endian::EndianUnion endian::objEndianUnion = { 0x01u };
	const endian::type & endian::little = objEndianUnion.uMem;
	const endian::type & endian::big = *(reinterpret_cast<const endian::type*>(&objEndianUnion.uData)+(sizeof(endian::larger_type)-1));

	bool application_path(std::string&strPath,std::string&strFile)
	{
		// Retrieve the file's complete path and name
		char buffer[MAX_PATH+1];
		#if defined(OS_WINDOWS_)
		const char chSeparator = '\\';
		#warning Only supporting non-unicode file paths.
		DWORD uLen = GetModuleFileNameA(NULL,buffer,sizeof(buffer));

		#else
		const char chSeparator = '/';
		ssize_t uLen = readlink("/proc/self/exe", buffer, sizeof(buffer));
		if (uLen == -1) 
		{
			// Maybe this (ancient) kernel doesn't support /proc/self, so we'll try to use the pid directly
			std::stringstream ss;
			// The next line could cause a compiler error if pid_t isn't streamable into ostream... but
			// the standard says pid_t must be a 'signed integer type', so this will never happen in practice.
			ss << "/proc/" << getpid() << "/exe";
			uLen = readlink(ss.str().c_str(), buffer, sizeof(buffer));
			if (uLen == -1) uLen=0; //in error case, we'll just empty the string.
		}
		#endif
		if (!uLen)
		{
			strPath.clear();
			strFile.clear();
			return false;
		}
		buffer[uLen] = '\0';

		// Separate the path and the filename
		for (size_t i = uLen;i--;)
		{
			// If we found a directory separator
			if (buffer[i] == chSeparator)
			{
				strPath.append(buffer,i+1);
				strFile.append(buffer+i+1,uLen-i-1);
				break;
			}
			if (!i) // last iteration, didn't find a separator
			{
				strPath.clear();
				strFile.append(buffer,uLen);
			}
		}
		return true;
	}

	void millisleep(const unsigned int&millisecs)
	{
		if (millisecs)
		{
			#ifdef OS_WINDOWS_
			// If our event object was created successfully
			HANDLE hEvent = CreateEvent( NULL, true, FALSE, NULL );
			if (hEvent != NULL)
			{
				// Clear the event's signalled state
				ResetEvent(hEvent);
				// Create a multimedia timer with 1ms precision that will signal the event upon expiration
				UINT uTimer=timeSetEvent( millisecs, 1, (LPTIMECALLBACK)hEvent, 0, TIME_ONESHOT | TIME_CALLBACK_EVENT_SET );
				if (uTimer)
				{
					// Wait for the event to be signalled, and wait at most the number of milliseconds specified
					// The timer will wake us up early, though, as it has higher precision than WaitForSingleObject
					WaitForSingleObject( hEvent, millisecs );
					// Kill the timer event just in case something crazy happened
					timeKillEvent(uTimer);
					// Cleanup
					CloseHandle(hEvent);
					return;
				}
				// Cleanup
				CloseHandle(hEvent);
			}
			// Fall back to the normal ~15ms accuracy sleep.
			Sleep(millisecs);
			#else
			struct timespec tspec;
			// Get the number of seconds
			tspec.tv_sec = millisecs/1000ul;
			// Subtract the seconds off of the milliseconds, and then convert to nanoseconds
			tspec.tv_nsec = (millisecs-(tspec.tv_sec*1000ul))*1000000ul;
			// Loop until the time elapses, calling again for the remainder if interrupted by a signal.
			while(nanosleep(&tspec,&tspec)==-1 && errno == EINTR) ; //LOOP IS EMPTY

			#endif
		}
	}


}
