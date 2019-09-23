#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <set>
#include <windows.h>
#include "../include/md5.h"
using namespace std;

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
	std::string strDescription = "settings.txt should contain the path to Conquer.exe\r\ne.g:\r\nC:\\Program Files\\Conquer 2.0\\";
	if (!FileExists(strFile))
	{
		cout << "ERROR: The file 'settings.txt' doesn't exist!" << endl << strDescription << endl;
		return "";
	}
	std::ifstream ifs(strFile.c_str());
	if (!ifs)
	{
		cout << "ERROR: Failed to open the file 'settings.txt' even though it exists!" << endl << strDescription << endl;
		return "";
	}
	if (!getline(ifs,strLine))
	{
		cout << "ERROR: Couldn't read a line from 'settings.txt' even though it was opened successfully!" << endl << strDescription << endl;
		return "";
	}
	ifs.close();
	strLine = StrTrim(strLine);

	if (strLine.empty())
	{
		cout << "The first line of 'settings.txt' is empty!" << endl << strDescription << endl;
		return "";
	}
	if (strLine[strLine.size()-1] != '\\')
		strLine += '\\';
	return strLine;
}

//////////////////////////////////////////////////

void output(ostream&ofs,vector<string>&data)
{
    vector<string>::iterator it;
    for (it=data.begin();it != data.end();++it)
    {
        if (it != data.begin())
            ofs << " ";
        ofs << *it;
    }
    ofs << endl;
}
string gettype(string thetype)
{
	if (thetype == "Plume") return "Plume";
	if (thetype == "Headband") return "Headband";
    if (thetype == "Warrior`sHelmet") return "Helmet";
    if (thetype == "Warrior`sArmor") return "Armor";
    if (thetype == "Shield") return "Shield";
    if (thetype == "Trojan`sCoronet") return "Coro";
    if (thetype == "Trojan`sArmor") return "Mail";
    if (thetype == "Taoist`sCap") return "Cap";
    if (thetype == "Taoist`sRobe") return "Robe";
    if (thetype == "Taoist`sBag") return "Bag";
    if (thetype == "Taoist`sBracelet") return "Brace";
    if (thetype == "Archer`sCoat") return "Coat";
    if (thetype == "Archer`sHat") return "Hat";

    if (thetype == "Blade") return "Blade";
    if (thetype == "Taoist`sBackSword") return "BackSword";
    if (thetype == "Hook") return "Hook";
    if (thetype == "Whip") return "Whip";
    if (thetype == "Axe") return "Axe";
    if (thetype == "Hammer") return "Hammer";
    if (thetype == "Club") return "Club";
    if (thetype == "Scepter") return "Scepter";
    if (thetype == "Dagger") return "Dagger";
    if (thetype == "Archer`sBow") return "Bow";
    if (thetype == "Sword") return "Sword";

    if (thetype == "Glaive") return "Glaive";
    if (thetype == "PoleAxe") return "PoleAxe";
    if (thetype == "Longhammer") return "LngHammer";
    if (thetype == "Spear") return "Spear";
    if (thetype == "Wand") return "Wand";
    if (thetype == "Halbert") return "Halbert";


    if (thetype == "Coat") return "Dress";
    if (thetype == "Dress") return "Dress";
    if (thetype == "Necklace") return "Necky";
    if (thetype == "Ring") return "Ring";
	if (thetype == "HeavyRing") return "HvyRing";
    if (thetype == "Boots" || thetype == "Boot") return "Boots";
    if (thetype == "Earring" || thetype == "Earrings") return "Earrings";
    return "";
}
string getquality(string theid)
{
    switch (theid[5])
    {
        case '0': return "[F]";
        case '6': return "[R]";
        case '7': return "[U]";
        case '8': return "[E]";
        case '9': return "[S]";
        case '3': //n1
        case '4': //n2
        case '5': //n3
        default: break;
    }
    return "";
}

void OutputError()
{
	cout << "Invalid arguments." << endl << "You should pass the program parameters via a shortcut, the Rename <whatever>.bat files are examples" << endl << "Alternatively, you can just run one of the .bat files." << endl;
	system("pause");
}

int main(int argc,char*argv[])
{
    if (argc != 2)
    {
        OutputError();
        return 1;
    }
	int rename = 0;
	bool tokens = false;
	if (argc == 2)
	{
		string param = argv[1];
		if (param.empty() || param.length() > 3 || param[0] != '-')
		{
			OutputError();
			return 1;
		}
		set<char> stInput;
		int mode = 0;
		for (int i=1;i<param.length();++i)
		{
			param[i] = tolower(param[i]);
			if (stInput.find(param[i]) != stInput.end())
			{
				OutputError();
				return 1;
			}
			if (param[i] != 'i' && param[i] != 't' && param[i]!= 'm')
			{
				OutputError();
				return 1;
			}
			stInput.insert(param[i]);
			switch(param[i])
			{
				case 'i': rename = 1; ++mode; break;
				case 't': rename = 2; ++mode; break;
				case 'm': tokens = true; break;
			}
		}
		if (mode > 1)
		{
			OutputError();
			return 1;
		}
	}

	if  (!rename && !tokens)
	{
		cout << "No operations to be performed." << endl;
		system("pause");
		return 0;
	}

	//BLARGH

	string strConquerDir = GetConquerDir();
	string strAppDir = GetAppDir();
	string strConquerItemTypeFile = strConquerDir+"ini\\itemtype.dat";
	string strItemTypeFile = strAppDir+"itemtype.dat";
	string strItemTypeTempFile = strAppDir+"itemtype.tmp";
	string strDecryptExe = strAppDir+"codecrypt.exe";
	string strItemTypeHashFile = strAppDir+"itemtype.hash";
	if (!FileExists(strDecryptExe))
	{
		cout << "Couldn't find codecrypt.exe" << endl << "Please make sure that it is located in the same folder as this executable." << endl << "Folder Path: " << strAppDir << endl;
		system("pause");
		return 0;
	}
	if (!FileExists(strConquerItemTypeFile))
	{
		cout << "Conquer's itemtype.dat is missing!" << endl << "Stop fucking with the files bitch!" << endl << "Full Path: " << strConquerItemTypeFile << endl;
		system("pause");
		return 0;
	}
	if (!CopyFile(strConquerItemTypeFile.c_str(),strItemTypeFile.c_str(),FALSE))
	{
		cout << "Couldn't copy conquer's itemtype.dat to the application folder." << endl << "Full Path: " << strItemTypeFile << endl;
		system("pause");
		return 0;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si,0,sizeof(si));
	memset(&pi,0,sizeof(pi));
	si.cb = sizeof(si);
	cout << "Decrypting itemtype.dat..." << endl;
	if (!CreateProcess(strDecryptExe.c_str(),
					const_cast<char*>((std::string("\"")+strDecryptExe+"\" 0 \""+strItemTypeFile+"\" \""+strItemTypeFile+"\"").c_str()),
					NULL,NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|CREATE_NO_WINDOW,NULL,
					strAppDir.c_str(),
					&si,&pi ))
	{
		cout << "Couldn't spawn the process for codecrypt.exe" << endl << "Full Path: " << strDecryptExe << endl;
		system("pause");
		return 0;
	}
	WaitForSingleObject( pi.hProcess, INFINITE );
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	cout << "Applying changes..." << endl;
	// Begin processing
    ifstream ifs(strItemTypeFile.c_str());
    ofstream ofs(strItemTypeTempFile.c_str());
    string strLine, strTemp;
    while (getline(ifs,strLine))
    {
        stringstream ss(strLine);
        vector<string> vcLines;
        while (ss >> strTemp)
            vcLines.push_back(strTemp);
		// Skip the first line (Amount=###)
        if (vcLines.size() < 5 || strncmp(vcLines[0].c_str(),"Amount",6) == 0)
        {
            output(ofs,vcLines);
            continue;
        }
        //special cases
        bool write = false;
        if (vcLines[0] == "721010") { vcLines[1] = "PeaceToken"; write = true; }
        if (vcLines[0] == "721011") { vcLines[1] = "ChaosToken"; write = true; }
        if (vcLines[0] == "721012") { vcLines[1] = "DesertedToken"; write = true; }
        if (vcLines[0] == "721013") { vcLines[1] = "ProsperousToken"; write = true; }
        if (vcLines[0] == "721014") { vcLines[1] = "DisturbedToken"; write = true; }
        if (vcLines[0] == "721015") { vcLines[1] = "CalmedToken"; write = true; }

		// if we already renamed it, or if there's no renaming to be done, output it.
		if (write || !rename)
		{
            output(ofs,vcLines);
            continue;
        }

        //other cases
        string strPrefix = getquality(vcLines[0]);
        string strPostfix = gettype(vcLines[vcLines.size()-2]);
        if (strPostfix.size())
        {
            if (rename==1) //if we're renaming with prefix only
            {
				//add prefix if it's not there
                if (strPrefix.size() && (vcLines[1].size() < strPrefix.size() || strncmp(vcLines[1].c_str(),strPrefix.c_str(),strPrefix.size()) != 0))
                    vcLines[1] = strPrefix + vcLines[1];
            }
            else
            {
				//rename by type
                //Got the type, now for the name to be changed
                vcLines[1] = strPrefix + strPostfix;
                if (vcLines[4] != "0")
                    vcLines[1] = vcLines[1] + "L" + vcLines[4];
            }
        }
        output(ofs,vcLines);
        continue;
    }
    ifs.close();
    ofs.close();

	memset(&si,0,sizeof(si));
	memset(&pi,0,sizeof(pi));
	si.cb = sizeof(si);
	cout << "Encrypting itemtype.dat..." << endl;
	if (!CreateProcess(strDecryptExe.c_str(),
					const_cast<char*>((std::string("\"")+strDecryptExe+"\" 1 \""+strItemTypeTempFile+"\" \""+strItemTypeFile+"\"").c_str()),
					NULL,NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|CREATE_NO_WINDOW,NULL,
					strAppDir.c_str(),
					&si,&pi ))
	{
		cout << "Couldn't spawn the process for codecrypt.exe" << endl << "Full Path: " << strDecryptExe << endl;
		system("pause");
		return 0;
	}
	WaitForSingleObject( pi.hProcess, INFINITE );
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	DeleteFile(strItemTypeTempFile.c_str());
	// Updating hash..
	char*ptr = MD5File(strConquerItemTypeFile.c_str());
	if (!ptr)
	{
		cout << "Error calculating the hash for conquer's 'itemtype.dat' file!" << endl;
		system("pause");
		return 0;
	}
	ofstream ofsHash(strItemTypeHashFile.c_str());
	ofsHash << ptr;
	ofsHash.close();
	cout << "Items renamed successfully!" << endl;
	system("pause");
    return 0;

}
