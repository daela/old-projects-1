#include <windows.h>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
std::string StrTrim(std::string str)
{
	std::string::size_type posA = str.find_first_not_of(" \t\r\n");
	if (posA != std::string::npos)
		return str.substr(posA,str.find_last_not_of(" \t\r\n")-posA+1);
	return "";
}
std::string GetAppDir(void)
{
	char szName [ MAX_PATH+1 ];
	GetModuleFileName(NULL,szName,MAX_PATH);
	std::string strName = szName;
	return strName.substr(0,strName.find_last_of("\\/")+1);
}
bool FileExists(std::string strFileName)
{
	WIN32_FIND_DATA obj;
	return (FindFirstFile(strFileName.c_str(),&obj) != INVALID_HANDLE_VALUE);
}
std::string GetConquerDir(void)
{
	std::string strLine;
	std::string strFile = GetAppDir()+"settings.txt";
	std::string strDescription = "\r\n\r\nsettings.txt should contain the path to Conquer.exe\r\ne.g:\r\nC:\\Program Files\\Conquer 2.0\\";
	if (!FileExists(strFile))
	{
		MessageBoxA(NULL,(std::string("The file 'settings.txt' doesn't exist!")+strDescription).c_str(),"Error",MB_OK);
		return "";
	}
	std::ifstream ifs(strFile.c_str());
	if (!ifs)
	{
		MessageBoxA(NULL,std::string("Failed to open the file 'settings.txt' even though it exists!").c_str(),"Error",MB_OK);
		return "";
	}
	if (!getline(ifs,strLine))
	{
		MessageBoxA(NULL,std::string("Couldn't read a line from 'settings.txt' even though it was opened successfully!").c_str(),"Error",MB_OK);
		return "";
	}
	ifs.close();
	strLine = StrTrim(strLine);

	if (strLine.empty())
	{
		MessageBoxA(NULL,(std::string("The first line of 'settings.txt' is empty!")+strDescription).c_str(),"Error",MB_OK);
		return "";
	}
	if (strLine[strLine.size()-1] != '\\')
		strLine += '\\';
	return strLine;
}
//#define LMUSIC_ENABLE
int main(int argc,char*argv[])
{
	std::string strConquerDir = GetConquerDir();
	std::string strActual = strConquerDir + "sound/music.mp3";
	std::string strBackup = strConquerDir + "sound/_music.mp3";
	if (strConquerDir.empty())
		return 2;
	#ifdef LMUSIC_ENABLE
		if (!FileExists(strActual) && FileExists(strBackup))
			if (!MoveFile(strBackup.c_str(),strActual.c_str()))
				std::cout << "ERROR: Couldn't copy the file.  MAKE SURE YOU ARE RUNNING THIS AS ADMINISTRATOR!" << std::endl;
			else
				std::cout << "Login music enabled." << std::endl;
		else
			std::cout << "Login music is already enabled." << std::endl;
	#else
		if (FileExists(strActual))
			if (!MoveFileEx(strActual.c_str(),strBackup.c_str(),MOVEFILE_REPLACE_EXISTING))
				std::cout << "ERROR: Couldn't copy the file.  MAKE SURE YOU ARE RUNNING THIS AS ADMINISTRATOR!" << std::endl;
			else
				std::cout << "Login music disabled." << std::endl;
		else
			std::cout << "Login music is already disabled." << std::endl;
	#endif
	system("pause");
	return 0;
}
