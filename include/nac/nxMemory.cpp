#include <nx/nxMemory.hpp>

nxMemory::nxMemory(const nxULong&blocksize,const nxMem*ptBuf)
{
	ptMemory=nxNULL;
	uBlockSize = 0;
	resize(blocksize);
	if (ptBuf)
		nxMemCpy(ptMemory,ptBuf,uBlockSize);
}
nxMemory::nxMemory(const nxMemory&obj)
{
	ptMemory = nxNULL;
	uBlockSize = 0;
	resize(obj.uBlockSize);
	if (uBlockSize)
		nxMemCpy(ptMemory,obj.ptMemory,uBlockSize);

}
void nxMemory::resize(const nxULong&blocksize)
{
	if (uBlockSize != blocksize)
	{
		uBlockSize = blocksize;
		delete[] ptMemory;
		if (uBlockSize)
			ptMemory = new nxMem[uBlockSize];
		else
			ptMemory = nxNULL;
	}
}
nxMemory::~nxMemory()
{
	delete[] ptMemory;
}
nxMemory::operator nxMem*() const
{
	return ptMemory;
}
const nxULong&nxMemory::size(void) const
{
	return uBlockSize;
}
void nxMemory::copy(const nxMemory& obj)
{
	resize(obj.uBlockSize);
	memcpy(obj.ptMemory,ptMemory,uBlockSize);
}
/*nxMem&nxMemory::operator[](const nxULong&uIndex) const
{
	return ptMemory[uIndex];
}*/
/*const nxMem&nxMemory::operator[](const nxULong&uIndex) const
{
	return ptMemory[uIndex];
}
*/
nxMemory&nxMemory::operator=(const nxMemory&uVal)
{
	for (nxULong uI = (uBlockSize<uVal.uBlockSize?uBlockSize:uVal.uBlockSize);uI--;)
		ptMemory[uI] = uVal.ptMemory[uI];
	return *this;
}

std::ostream& operator<<(std::ostream&ofs,const nxMemory&obj)
{
	return ofs.write((const char*)obj.ptMemory,obj.uBlockSize);
}
std::istream& operator>>(std::istream&ifs,nxMemory&obj)
{
	return ifs.read((char*)obj.ptMemory,obj.uBlockSize);
}
