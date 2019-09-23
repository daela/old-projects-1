#include "nxCommon.hpp"
#include <unistd.h>
#include <cctype>


bool nxApplicationPath(std::string&strPath,std::string&strFile)
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
        if (uLen == -1) uLen=0; //in error case, we'll just empty the string.
    #endif
    if (!uLen)
    {
        strPath=strFile="";
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
            strPath="";
            strFile.append(buffer,uLen);
        }
    }
    return true;
}

#if defined(OS_WINDOWS_)
HINSTANCE nxInstanceHandle(HINSTANCE hInst)
{
	static HINSTANCE hInstance = NULL;
	if (!hInst) return hInstance;
	hInstance = hInst;
	return hInstance;
}
#endif
