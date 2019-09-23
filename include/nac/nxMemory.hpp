#ifndef __NXMEMORY_HPP__
#define __NXMEMORY_HPP__

/*
	author:
		nacitar
	classes:
		nxMemory - a class that allows convenient management of a static block of memory.
				 - resizing trashes the data
				 - base class for nxNumber
*/

#include <iostream>
#include <nx/type.hpp>
#include <nx/memory.hpp>

class nxMemory
{
	protected:
		nxMem*ptMemory;
		nxULong uBlockSize;
	public:
		nxMemory(const nxULong&blocksize=0,const nxMem*ptBuf=nxNULL);
		nxMemory(const nxMemory&obj);
		void resize(const nxULong&blocksize);
		~nxMemory();
		// cast to nxMem*
		operator nxMem*() const;
		// buffer size
		const nxULong&size(void) const;

		void copy(const nxMemory& obj);
		// subscript operators
		//nxMem&operator[](const nxULong&uIndex) const;
		//const nxMem&operator[](const nxULong&uIndex) const;
		// assignment
		nxMemory&operator=(const nxMemory&uVal);

		friend std::ostream&operator<<(std::ostream&ofs,const nxMemory&obj);
		friend std::istream&operator>>(std::istream&ifs,nxMemory&obj);
};

#endif // __NXMEMORY_HPP__
