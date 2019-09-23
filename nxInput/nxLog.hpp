#ifndef NXLOG_HPP_INCLUDED
#define NXLOG_HPP_INCLUDED

#include "nxCommon.hpp"
#include <boost/interprocess/sync/interprocess_recursive_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>

// Null output stream
extern std::ostream&nxNullOut;

/**
	LOGGING FUNCTIONS:
		(optional) nxSetLogFile("wherever/you/want.log")
			If you don't call this function, the file is placed in the same directory as the application, and named:
				No error: <program name without exe extension>.log
				No error with NX_LOGSTREAM_PID defined: <program name without exe extension>.<pid>.log
				Error determining application pathname: <pid>.log
			Of course, if specified non-absolutely, the path will be relative to the current working directory
			but you can retrieve the application's directory with the nxApplicationPath function listed above.
			NOTE: if you call it with an empty string, std::cerr will be used instead of a file.
			NOTE: if you have multiple instances of a process (non-forked, just independently ran) they wont be able
				  to open the same file for output.  For this reason, you should enable PIDs in the log files.  If your
				  process will only have one instance, or all other instances will be forked from the same instance
				  (which implies that they will inherit the file handles) you won't run into this issue.

		Outputting log messages: it can take anything an ostream can, as it really is just an ostream.
			nxLog << "I have " << someint << "apples." << std::endl << "anything you can put into an ostream";
			nxLog.write(buf,buf.size());

		If thread safety isn't disabled, each usage of nxLog, up to the end of the statement, is thread safe.
		No other thread may be outputting at the same time.  Thus, you should consider any starvation issues that you
		may have.  You can disable thread safety (per file) by defining NX_LOGSTREAM_UNSAFE before including this file.
		Note that if you have some files that use safety, and others that do not, the safe threads will not
		output at the same time as each other, but the unsafe threads will be acting independently, and can thus
		interfere.  If you opt to go with a mixed-configuration, for whatever reason, you must deal with this.

		If you absolutely need to output in separate statements, and want to maintain thread safety, you can:
			nxLockLog();
			nxLog << whatever;
			nxLog << whatever else;
			nxUnlockLog();

		If you do not make a call to nxSetLogFile, the log file is not created until your first output statement.
		If you do call nxSetLogFile, the log file is created at that point.

		You can disable logging by defining NX_LOGSTREAM_DISABLED prior to including this file.  Note that any
		operations performed in the output will still be performed.  That is...
			nxLog << ++i << i++ << SomeFuncWithSideEffects(++h + (str += "hello").size()) << std::endl;
		will still perform all the operations it would have if logging was enabled, only, the data will go nowhere.
		Further, you may note that this can be done on a per-file basis.  You can have some cpp files that disable
		logging and some that don't.

**/


// Will set the log's filename to whatever is specified.
bool nxSetLogFile(const std::string &strLogFile);

/// THE BELOW CLASSES AND FUNCTIONS ARE NOT MEANT TO BE USED DIRECTLY, ONLY MEANT TO BE USED AS AN ANONYMOUS OBJECT BY THE nxLog MACRO
class nxUnsafeLogWrapper
{
	public:
		typedef boost::interprocess::scoped_lock<boost::interprocess::interprocess_recursive_mutex> scoped_lock;
		operator std::ostream&(void);
};
class nxThreadSafeLogWrapper : public nxUnsafeLogWrapper
{
	public:
		nxThreadSafeLogWrapper();
	private:
		scoped_lock lock;

};

void nxLogMutexLock(void);
void nxLogMutexUnlock(void);

/// THE FOLLOWING DEFINES THE MACROS THE USER IS INTENDED TO USE
#if !defined(NX_LOGSTREAM_DISABLED)
	#if !defined(NX_LOGSTREAM_UNSAFE)
		#define nxLockLog() (nxLogMutexLock())
		#define nxUnlockLog() (nxLogMutexUnlock())
		#define nxLog (nxThreadSafeLogWrapper().operator std::ostream&())
	#else
		#define nxLockLog()
		#define nxUnlockLog()
		#define nxLog (nxUnsafeLogWrapper().operator std::ostream&())
	#endif
#else
	#define nxLockLog()
	#define nxUnlockLog()
	#define nxLog nxNullOut
#endif

#endif // NXLOG_HPP_INCLUDED
