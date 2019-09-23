//-----------------------------------------------------------------------------
//    main.cpp
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#include "Token.h"
#include "DataValue.h"
#include "DFA.h"
#include "Lexer.h"
#include "Parser.h"
#include "Executer.h"
#include <iostream>
#include <string>
using namespace std;

int main(void)
{

	Lexer objLex;
	objLex.Init();

	while (true)
	{
		string strInput = "";
		while (strInput.empty())
		{
			cout << "Input a filename or 'q' to quit: ";
			if  (!getline(cin,strInput) || strInput == "q")
			{
				strInput = "";
				break;
			}
		}
		// User typed "q" if it's empty (got here with "break")
		if (strInput.empty()) break;
		if (!objLex.Open(strInput.c_str()))
		{
			cout << "Error opening file: " << strInput << endl;
			continue;
		}
		Parser objParse(&objLex);
		bool created = false;
		try
		{
			if (objParse.Validate())
			{
				created = true;
				Executer exe;
				exe.Execute(objParse.lstParseTree);
				//cout << exe.symTable << endl << endl;
				vector<DataValue>::iterator it;
			}
			else
			{
				cout << "Invalid source file, parse tree was not created." << endl;
			}
		} catch (Token objToken)
		{
			cout << "Lexer Error: " << objToken.dtValue << endl;
		}
		/*if  (!created)
		{
			string strTemp;
			do
			{
				cout << "Would you like to view the partial parse-tree (y/n)? ";
				cin >> strTemp;
				cin.ignore();
				cin.clear();
			} while (strTemp!="y"&&strTemp!="Y"&&strTemp!="n"&&strTemp!="N");
			if  (tolower(strTemp[0]) == 'y')
				objParse.PrintTree(cout);
		}*/
		objLex.Close();
		cout << "End of file." << endl;

	}
	return 0;
}
