#ifndef __NXFILE_HPP__
#define __NXFILE_HPP__
#include "type.hpp"
#include "nxStreamHandle.hpp"
#include "nxMemory.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class nxFile
{
	const nxStreamHandle &objIn;
	nxULong uFileSize; // size of file on disk
	nxULong uFilePos;
	protected:
	bool simple_read(nxMemory&obj) const;
	public:
	nxFile(const nxStreamHandle&obj);
	nxFile (const nxFile&obj);
	nxFile(const nxStreamHandle&obj,const nxULong&filepos,const nxULong&filesize);
	nxFile(const nxFile&obj,const nxULong&filepos,const nxULong&filesize);
	bool IsOpen(void) const;
	virtual const nxULong & size(void) const; // size of actual data (decompressed)
	virtual bool read(nxMemory&obj) const;
	bool write(std::ostream&ofs) const;
	virtual ~nxFile();
};

#endif // __NXFILE_HPP__
