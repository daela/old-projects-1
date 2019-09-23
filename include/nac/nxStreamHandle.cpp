#include "nxStreamHandle.hpp"
nxStreamHandle::nxStreamHandle(const std::string&name)
{
	ptIn = &std::cin;
	strName = name;
}
nxStreamHandle::~nxStreamHandle() { }
const nxULong nxStreamHandle::size() const
{
	if (!ptIn)
		return 0u;
	Lock();
	std::streampos sPos = ptIn->tellg();
	ptIn->seekg(0,std::ios::end);
	nxULong uFileSize = ptIn->tellg();
	ptIn->seekg(sPos,std::ios::beg);
	Unlock();
	return uFileSize;
}
bool nxStreamHandle::IsOpen(void) const { return (nxMem*)ptIn != nxNULL; }
std::istream& nxStreamHandle::Stream(void) const
{
	return *ptIn;
}




nxStringStreamHandle::nxStringStreamHandle()
{
	ptIn = new std::stringstream();
}
nxStringStreamHandle::operator std::stringstream&()
{ return *static_cast<std::stringstream*>(ptIn); }
nxStringStreamHandle::~nxStringStreamHandle()
{
	delete ptIn;
}


nxFileHandle::nxFileHandle(const std::string&filename) : nxStreamHandle(filename)
{
	ptIn = new std::ifstream(filename.c_str(),std::ios::binary);
	if (!*ptIn)
	{
		delete ptIn;
		ptIn = (std::istream*)nxNULL;
	}
}
nxFileHandle::~nxFileHandle()
{
	static_cast<std::ifstream*>(ptIn)->close();
	delete ptIn;
}
