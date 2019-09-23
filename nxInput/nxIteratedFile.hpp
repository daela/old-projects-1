#ifndef __NXITERATEDFILE_HPP__
#define __NXITERATEDFILE_HPP__

#include <fstream>
#include "nxUtility.hpp"
/**
   This class allows iterated file reading, suitable for a DFA-oriented file parser.
   The value for a given iterator is cached so if you reuse the value it is available,
   and seeking within the value is attempted to be minimized (will be no seeking at all if you
   read in order).  If you use an iterator to a file that's already went out of scope, you'll enjoy
   a lovely crash.  If you close and reopen with the same file object and access an old iterator,
   the results are undefined.  This code is thread safe, though.. in that the iterators will read
   from the proper file locations even in the presence of multiple threads reading from
   different parts of the file.

   Has a size() method and a method to allow setting an iterator to a specific location.
   These iterators can do more math than most, too.. and can be compared easily.
   Comparing iterators of different files has undefined results.
**/

class nxIteratedFile
{
	public:
		// This class is too simple for comments, really.. just take note that the call to ptFile->get() takes a REFERENCE
		// so that if uPos > uSize, it sets uPos = uSize (EOF)
		// Everything is small, so everything is inline... I want this blazing fast anyway.
		class const_iterator
		{
		    friend class nxIteratedFile;
			private:
				nxIteratedFile*ptFile;
				unsigned long uPos;
				char chVal;
				inline const_iterator(nxIteratedFile&obj,unsigned long pos)
				{
					ptFile = &obj;
					uPos = pos;
					update();
				}
			public:
				inline char&update(void)
				{
					return (chVal = ptFile->get(uPos));
				}
				/*inline const_iterator()
				{
					ptFile = NULL;
					uPos = 0;
					chVal = '\0';
				}*/

				inline const_iterator(const const_iterator&rhs)
				{
					ptFile = rhs.ptFile;
					uPos = rhs.uPos;
					chVal = rhs.chVal;
				}
				inline char operator*(void) const
				{
					return chVal;
				}
				inline const_iterator&operator++(void)
				{
					if (uPos < ptFile->uSize)
						++uPos;
					update();
					return *this;
				}
				inline const_iterator operator++(int)
				{
				    const_iterator itRet(*this);
					operator++();
					return itRet;
				}
				inline const_iterator operator--(int)
				{
					const_iterator itRet(*this);
					operator--();
					return itRet;
				}
				inline const_iterator&operator--(void)
				{
					if (uPos)
						--uPos;
					update();
					return *this;
				}
				inline const_iterator&operator=(const unsigned long&uVal)
				{
					uPos = uVal;
					update();
					return *this;
				}
				inline const_iterator&operator=(const const_iterator&rhs)
				{
					ptFile = rhs.ptFile;
					uPos = rhs.uPos;
					chVal = rhs.chVal;
					return *this;
				}
				inline const_iterator&operator+=(const unsigned long&uVal)
				{
					unsigned long uOld = uPos;
					uPos += uVal;
					if (uPos < uOld) //if there was an overflow
						uPos = ptFile->uSize;
					update();
					return *this;
				}
				inline const_iterator operator+(const unsigned long&uVal)
				{
					const_iterator itRet(*this);
					return (itRet += uVal);
				};
				inline const_iterator&operator-=(const unsigned long&uVal)
				{
					unsigned long uOld = uPos;
					uPos -= uVal;
					if (uPos > uOld) //if there was an overflow
						uPos = ptFile->uSize;
					update();
					return *this;
				}
				inline const_iterator operator-(const unsigned long&uVal)
				{
					const_iterator itRet(*this);
					return (itRet -= uVal);
				};
				inline bool operator==(const const_iterator&rhs) const
				{
					return (uPos == rhs.uPos);
				}
				inline bool operator!=(const const_iterator&rhs) const
				{
					return (uPos != rhs.uPos);
				}
				inline bool operator<(const const_iterator&rhs) const
				{
					return (uPos < rhs.uPos);
				}
				inline bool operator<=(const const_iterator&rhs) const
				{
					return (uPos <= rhs.uPos);
				}
				inline bool operator>(const const_iterator&rhs) const
				{
					return (uPos > rhs.uPos);
				}
				inline bool operator>=(const const_iterator&rhs) const
				{
					return (uPos >= rhs.uPos);
				}
		};
	protected:
		std::ifstream ifsFile;
		unsigned long uPos;
		unsigned long uSize;
		char chVal;
        nxRecursiveMutex csLock;
		char get(unsigned long&pos);
		bool open(const std::string&strFile);
	public:

		inline nxIteratedFile()
		{
			uPos = uSize = 0;
			chVal = '\0';
		}
		inline unsigned long size(void) const
		{
		    return uSize;
		}
		inline nxIteratedFile(const std::string&strFile)
		{
			open(strFile);
		}

		inline operator bool(void) const
		{
			return (ifsFile);
		}
		inline const_iterator pos(const unsigned long&pos)
		{
			return const_iterator(*this,pos);
		}
		inline const_iterator begin(void)
		{
			return const_iterator(*this,0);
		}
		inline const_iterator end(void)
		{
			return const_iterator(*this,uSize);
		}
};

#endif
