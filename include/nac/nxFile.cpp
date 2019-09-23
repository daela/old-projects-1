//http://www.cs.unc.edu/Research/compgeom/gzstream/
#include "nxFile.hpp"
nxFile::nxFile(const nxStreamHandle&obj) : objIn(obj)
{
	uFilePos = 0;
	objIn.Lock();
	std::streampos sPos = objIn.Stream().tellg();
	objIn.Stream().seekg(0,std::ios::end);
	uFileSize = objIn.Stream().tellg();
	objIn.Stream().seekg(sPos,std::ios::beg);
	objIn.Unlock();
}
nxFile::nxFile (const nxFile&obj) : objIn (obj.objIn)
{
	uFilePos = obj.uFilePos;
	uFileSize = obj.uFileSize;
}
nxFile::nxFile(const nxStreamHandle&obj,const nxULong&filepos,const nxULong&filesize) : objIn(obj)
{
	uFilePos = filepos;
	uFileSize = filesize;
}
nxFile::nxFile(const nxFile&obj,const nxULong&filepos,const nxULong&filesize) : objIn(obj.objIn)
{
	uFilePos = filepos;
	uFileSize = filesize;
}
const nxULong & nxFile::size(void) const
{
	return uFileSize;
}
bool nxFile::read(nxMemory&obj) const
{
	return simple_read(obj);
}
bool nxFile::simple_read(nxMemory&obj) const
{
	obj.resize(uFileSize);
	objIn.Lock();
	std::streampos sPos = objIn.Stream().tellg();
	objIn.Stream().seekg(uFilePos,std::ios::beg);
	objIn.Stream() >> obj;
	objIn.Stream().clear();
	objIn.Stream().seekg(sPos,std::ios::beg);
	objIn.Unlock();
	return objIn.Stream();
}
bool nxFile::write(std::ostream&ofs) const
{
	nxMemory obj(size());
	if (read(obj))
	{
		ofs << obj;
		return true;
	}
	return false;
}

nxFile::~nxFile() { };

bool nxFile::IsOpen(void) const
{
	return objIn.IsOpen();
}
