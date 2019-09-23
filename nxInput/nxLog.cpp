#include "nxLog.hpp"
#include <string>
#include <fstream>
#include <sstream>




namespace
{
	class nullstream : public std::ostream
	{
		private:
			class nullbuf : public std::streambuf
			{
				protected:
					virtual int overflow(int c) { return traits_type::not_eof(c); }
			} m_sbuf;
		public:
			nullstream() : std::ios(&m_sbuf), std::ostream(&m_sbuf) {}
	} objNullStream;
}
std::ostream&nxNullOut = objNullStream;

namespace
{
	std::ofstream ofsLog;

	boost::interprocess::interprocess_recursive_mutex logmutex;

	bool bUseDefaultName=false;
	bool bUse_cerr=false;
	// Only called internally, synchronized such that if the user's call to set the log file comes at the same time the
	// first output occurs which would thereby lead to this function being called, this will use the user's specified
	// filename instead of the default name.  This ensures thread safety.  This is only ever called once.
	bool nxSetLogFile()
	{
		nxThreadSafeLogWrapper::scoped_lock lock(logmutex);
		if (!ofsLog.is_open() && !bUse_cerr)
		{
			bUseDefaultName=true;
			bool bRet=::nxSetLogFile("");
			bUseDefaultName=false;
			return bRet;
		}
	}
}

bool nxSetLogFile(const std::string&strLogFile)
{
	nxThreadSafeLogWrapper::scoped_lock lock(logmutex);
	ofsLog.clear();
	if (ofsLog.is_open()) ofsLog.close();
	std::string strOutFile;
	if (strLogFile.empty())
	{
		if (bUseDefaultName)
		{
			std::string strPath, strFile, strPid;
			nxApplicationPath(strPath,strFile);

			#if !defined(NX_LOGSTREAM_PID)
				if (strPath.empty() && strFile.empty())
			#endif
				{
					std::stringstream ss;
					if (!(strPath.empty() && strFile.empty()))
						ss << '.'; // only add the dot if there's a filename to precede it.
					#if defined(OS_WINDOWS_)
						ss << GetCurrentProcessId();
					#else
						ss << getpid();
					#endif
					strPid = ss.str();
				}

			#if defined(OS_WINDOWS_)
				// Remove the .exe extension if it's there (should be)
				{
					std::string::size_type uOffset = strFile.size();
					if (uOffset>4 &&
						std::tolower(strFile[uOffset-4]) == '.' &&
						std::tolower(strFile[uOffset-3]) == 'e' &&
						std::tolower(strFile[uOffset-2]) == 'x' &&
						std::tolower(strFile[uOffset-1]) == 'e')
						strFile.resize(uOffset-4);
				}
			#endif
			strOutFile=strPath+strFile+strPid+".log";
		}
		else
		{
			bUse_cerr=true;
			return true;
		}
	}
	else
	{
		strOutFile = strLogFile;
	}
	ofsLog.open(strOutFile.c_str());
	return true;
}

nxThreadSafeLogWrapper::nxThreadSafeLogWrapper()
: lock(logmutex)
{ }

nxUnsafeLogWrapper::operator std::ostream&(void)
{
	static const bool bInitialized = nxSetLogFile(); // ensures that the default initializer is only called once
	return (bUse_cerr?std::cerr:ofsLog);
}
void nxLogMutexLock(void)
{
	logmutex.lock();
}
void nxLogMutexUnlock(void)
{
	logmutex.unlock();
}
