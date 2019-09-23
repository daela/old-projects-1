#ifndef __STRMANIP_H__
#define __STRMANIP_H__

#include <string>
#include <locale>
#include <iostream>


namespace nac
{
	enum convert_cases { TO_LOWER, TO_UPPER };

	template <convert_cases>
	void string_case(std::string &, std::locale const & l = std::cout.getloc());

	void split_path(const std::string&strFilePath,std::string&strPath,std::string&strFile);

}

#endif
