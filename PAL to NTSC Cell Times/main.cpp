#include <fstream>
#include <iostream>
#include <string>
//#include <windows.h>
int main(int argc,char*argv[])
{
	std::string strInput = "";
	for (int i=1;i<argc;++i)
	{
		if (!strInput.empty())
			strInput += ' ';
		strInput += argv[i];
	}
	std::ifstream ifs(strInput.c_str());
	if (!ifs)
	{
		std::cout << "(ERROR)Input file couldn't be opened." << std::endl << "File: " << strInput << std::endl;
		return 1;
	}
	std::ofstream ofs((strInput+".ntsc.txt").c_str());
	if (!ofs)
	{
		std::cout << "(ERROR)Output file couldn't be created." << std::endl << "File: " << strInput+".ntsc.txt" << std::endl;
		ifs.close();
		return 2;
	}
	unsigned long uVal;
	while (ifs >> uVal)
	{
		ofs << (unsigned long)((double)(uVal)/25.0*29.97) << std::endl;
	}
	ifs.close();
	ofs.close();
	return 0;
}
