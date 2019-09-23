#include <windows.h>
#include <string>
#include <sstream>
#include <fstream>
#include "madCHook.hpp"
#include "md5.h"
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
std::string GetConquerExe(std::string strConquerDir)
{
	std::string strConquerExe = strConquerDir + "Conquer.exe";
	if (strConquerDir.empty()) return "";

	if (!FileExists(strConquerExe))
	{
		MessageBoxA(NULL,(std::string("Couldn't find Conquer.exe\r\nPlease validate the path provided in settings.txt\r\n\r\nFull Path: ")+strConquerExe).c_str(),"Error",MB_OK);
		return "";
	}
	return strConquerExe;
}

bool bDone;
bool bItemType;
bool bNoEffects;
MadCodeHook madCodeHook;

VOID WINAPI HandleDllMessage(LPCSTR pIpc,PVOID pMessageBuf,DWORD dwMessageLen,PVOID pAnswerBuf,DWORD dwAnswerLen)
{
	if (dwMessageLen >= 1 && dwAnswerLen >= sizeof(bool))
	{
		bool&bAnswer=*static_cast<bool*>(pAnswerBuf);
		switch (static_cast<const char*>(pMessageBuf)[0])
		{
			case 'i':
				bAnswer = bItemType; break;
			case 'e':
				bAnswer = bNoEffects; break;
			case 'q':
				bAnswer = true;
				bDone = true;
				break;
			default:
				bAnswer = false;
		}
	}
}
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
	std::string strCommandLine = lpCmdLine;
	bDone = bItemType = bNoEffects = false;
	if (strCommandLine.find_first_not_of("ie") != std::string::npos)
	{
		MessageBoxA(NULL,
			"You must invoke this program with either no arguments, or a single argument.\r\n"
			"The single argument can only be the following, or a combination of them:\r\n"
			" i - use custom itemtype.dat\r\n"
			" e - disable effects\r\n\r\n"
			"Example of combined flags\r\n"
			" ie - options i and e combined"
			,"Invalid Parameters",MB_OK);
		return 1;
	}

	std::string strAppDir = GetAppDir();
	std::string strConquerDir = GetConquerDir();
	std::string strConquerExe = GetConquerExe(strConquerDir);
	std::string strMadCHook = strAppDir + "madCHook.dll";
	if (strConquerExe.empty())
		return 1;
	if (!FileExists(strMadCHook))
	{
		MessageBoxA(NULL,"You must have madCHook.dll in the same folder as nConquer.","DLL Missing",MB_OK);
		return 1;
	}
	std::string strIpcName;
	{
		std::stringstream ss;
		ss << "NAC_IPC_" << GetCurrentProcessId();
		strIpcName = ss.str();
	}
	if (!madCodeHook.Init(strMadCHook))
	{
		MessageBoxA(NULL,"madCHook.dll was located, but couldn't be loaded.","DLL Load Error",MB_OK);
		return 1;
	}
	bItemType = (strCommandLine.find_first_of('i') != std::string::npos);
	bNoEffects = (strCommandLine.find_first_of('e') != std::string::npos);
	if (bItemType)
	{
		if (!FileExists(strAppDir+"itemtype.dat"))
		{
			MessageBoxA(NULL,(std::string("You specified to use a custom itemtype.dat, but the file couldn't be located.\r\nPlease make sure that it is located in the same folder as this executable.\r\nFolder Path: ")+strAppDir).c_str(),"Error",MB_OK);
			return 3;
		}
		//make sure it's current
		std::string strItemTypeHashFile = strAppDir+"itemtype.hash";
		if (!FileExists(strItemTypeHashFile))
		{
			MessageBoxA(NULL,(std::string("You specified to use a custom itemtype.dat, but the hash of the original itemtype.dat has not been generated.\r\nThis hash is necessary to ensure the itemtype.dat file is up to date.\r\nPlease re-run the item renamer application to remedy this problem.\r\nThe item renamer should be in the following folder.\r\nFolder Path: ")+strAppDir).c_str(),"Error",MB_OK);
			return 4;
		}
		std::ifstream ifs(strItemTypeHashFile.c_str());
		if (!ifs)
		{
			MessageBoxA(NULL,(std::string("You specified to use a custom itemtype.dat, but the hash of the original itemtype.dat could not be opened.\r\nThe file should be in the same folder as this executable.\r\nFolder Path: ")+strAppDir).c_str(),"Error",MB_OK);
			return 5;
		}
		std::string strHash;
		if (!getline(ifs,strHash))
		{
			MessageBoxA(NULL,(std::string("You specified to use a custom itemtype.dat, but the hash of the original itemtype.dat had an error while being read.\r\nYou should re-run the item renamer application to rememdy this problem.\r\nFolder Path: ")+strAppDir).c_str(),"Error",MB_OK);
			return 6;
		}
		ifs.close();
		strHash = StrTrim(strHash);
		std::string strConquerItemTypeFile = strConquerDir+"ini\\itemtype.dat";
		if (!FileExists(strConquerItemTypeFile))
		{
			MessageBoxA(NULL,(std::string("Conquer's itemtype.dat is missing!\r\nStop fucking with the files bitch!\r\nFull Path: ")+strConquerItemTypeFile).c_str(),"Error",MB_OK);
			return 7;
		}
		char*ptHash = MD5File(strConquerItemTypeFile.c_str());
		if (!ptHash)
		{
			MessageBoxA(NULL,(std::string("Error computing conquer's itemtype.dat hash!\r\nIf problem persists, contact the author so he can validate the algorithm.\r\nFull Path: ")+strConquerItemTypeFile).c_str(),"Error",MB_OK);
			return 8;
		}
		if (strHash != ptHash)
		{
			MessageBoxA(NULL,(std::string("The stored hash and the actual hash of conquer's itemtype.dat are different.\r\nThis usually means that a patch has updated the itemtype.dat file.\r\nRe-running the item renamer that is located in the same folder as this executable will remedy this problem.\r\nStored: ")+strHash+"\r\nCalculated: "+ptHash).c_str(),"Error",MB_OK);
			return 9;
		}
	}
	std::string strInjectDll = strAppDir+"nInject.dll";
	if (!FileExists(strInjectDll))
	{
		MessageBoxA(NULL,(std::string("Couldn't find nInject.dll\r\nPlease make sure that it is located in the same folder as this executable.\r\nFolder Path: ")+strAppDir).c_str(),"Error",MB_OK);
		return 10;
	}
	//load conquer here.
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset(&pi,0,sizeof(pi));
	memset(&si,0,sizeof(si));
	si.cb = sizeof(si);

	madCodeHook.CreateIpcQueue(strIpcName.c_str(),HandleDllMessage);
	if (madCodeHook.CreateProcessExA(strConquerExe.c_str(),const_cast<char*>((std::string("\"") + strConquerExe + "\" blacknull").c_str()),
	NULL,NULL,FALSE,CREATE_DEFAULT_ERROR_MODE,NULL,strConquerDir.c_str(),
				&si,&pi,strInjectDll.c_str()))
	{
		while (!bDone)
			Sleep(1);
	}
	else
		MessageBoxA(NULL,"Error launching Conquer.  Contact the creator of the program to remedy the problem.","Launch Error",MB_OK);
	madCodeHook.DestroyIpcQueue(strIpcName.c_str());
	return 0;
}
