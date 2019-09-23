#include "nxIteratedFile.hpp"

char nxIteratedFile::get(unsigned long&pos)
{
    // Makes this thread safe.. putting this here makes all the iterators thread safe.
    nxScopedRecursiveLock objGuard(csLock);
	// If you're asking for the current position, give the value
	if (pos == uPos)
		return chVal;
	// if the position we're asking for is the next positon (and we arent overflowing the number)
	else if (pos && pos == uPos + 1)
		// increment the position to the next one
		++uPos;
	//if the position is valid
	else if (pos < uSize)
	{
		// set the position and seek to the proper location in the file
		uPos = pos;
		ifsFile.seekg(uPos,std::ios::beg);
	}
	else
	{
		// Set EOF
		pos = uPos = uSize;
		return (chVal = '\0');
	}
	return (chVal = ifsFile.get());
}

bool nxIteratedFile::open(const std::string&strFile)
{
	if (ifsFile.is_open())
		ifsFile.close();
	ifsFile.open(strFile.c_str(),std::ios::binary);
	if (ifsFile)
	{
		ifsFile.seekg(0,std::ios::end);
		uSize = ifsFile.tellg();
		ifsFile.seekg(0,std::ios::beg);
		uPos = 0;
		if (uPos < uSize)
			chVal = ifsFile.get();
		else
			chVal = '\0';
	}
	else
	{
		uPos = uSize = 0;
		chVal = '\0';
	}

	return (ifsFile);
}
