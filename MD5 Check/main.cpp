#include <windows.h>
#include <string>
#include <set>
#include <queue>
#include <iostream>
#include <fstream>
#include "md5.h"
#include "md5.cpp"

bool FindAllFiles(std::set<std::string>&stFiles,std::string strPath)
{

	if (strPath.empty()) return false;
	switch (strPath[strPath.size()-1])
	{
		case '/':
		case '\\':
			break;
		default:
			strPath += '\\';
	}

	std::queue<std::string> qDirectories;
	qDirectories.push(strPath);
	while (!qDirectories.empty())
	{
		std::string strCurPath = qDirectories.front();
		qDirectories.pop();
		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile((strCurPath+"*").c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE) continue;
		do
		{
			std::string strResult = ffd.cFileName;
			if ((strResult == ".") || (strResult == ".."))
				continue;
			std::string strCurFile = strCurPath+strResult;
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				qDirectories.push(strCurFile+"\\");
			else
				stFiles.insert(strCurFile);
		} while (FindNextFile(hFind,&ffd));
	}


	return true;
}

int main(int argc,char*argv[])
{

	std::set<std::string> stFiles;
	FindAllFiles(stFiles,"C:\\Program Files (x86)\\Electronic Arts\\Battlefield 2142");
	std::ofstream ofs("checksums.txt");
	for (std::set<std::string>::iterator it=stFiles.begin();it!=stFiles.end();++it)
	{
		 ofs << MD5File(it->c_str()) << '\t' << *it << std::endl;
	}
	ofs.close();
	return 0;

}
