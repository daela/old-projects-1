#ifndef __NXSTREAMHANDLE_HPP__
#define __NXSTREAMHANDLE_HPP__
#include "type.hpp"
#include "nxMutex.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
class nxStreamHandle : public nxMutex
{
	public:
		std::istream*ptIn;
		std::string strName;
		nxStreamHandle(const std::string&name = "");
		virtual ~nxStreamHandle();
		const nxULong size() const;
		bool IsOpen(void) const;
		std::istream& Stream(void) const;

};
class nxStringStreamHandle : public nxStreamHandle
{
	public:
		nxStringStreamHandle();
		operator std::stringstream&();
		virtual ~nxStringStreamHandle();
};
class nxFileHandle : public nxStreamHandle
{
	public:

		nxFileHandle(const std::string&filename);
		virtual ~nxFileHandle();
};

#endif // __NXSTREAMHANDLE_HPP__
