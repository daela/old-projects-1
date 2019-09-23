//-----------------------------------------------------------------------------
//				driver.cpp
//
//	Author:		Jacob McIntosh
//	Section:	01
//	Date:		March 9, 2004
//
//	Class driver implementation.  Parses a file for insertion of words into an
//  AVL tree
//
//-----------------------------------------------------------------------------

#pragma warning(disable : 4786)

#include "BSTree.h"
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include "AVLTree.h"

using namespace std;


void Print(const string & word,const unsigned long count)
{
	cout << word << ' ' << count << endl;
}
int main(int argc,char*argv[])
{
	AVLTree<string> avlTree;
	BSTree<string> bsTree;
	string str, last;
	char c;

	if (argc != 2)
	{
		cerr << "Parameters: <filename>.  use quotations around long path names." << endl;
		return 1;
	}

	ifstream ifsInput(argv[1],ios::in | ios::binary);

	if (!ifsInput)
	{
		cerr << "Couldn't open input file." << endl;
		return 2;
	}

	
	while (ifsInput.get(c) && !ifsInput.eof())
	{
		switch (c)
		{
			case '.':
			case ',':
			case '\'':
			case '\"':
			case ':':
			case ';':
			case '<':
			case '>':
			case '/':
			case '?':
			case '[':
			case ']':
			case '{':
			case '}':
			case '\\':
			case '|':
			case '!':
			case '#':
			case '$':
			case '%':
			case '^':
			case '&':
			case '*':
			case '(':
			case ')':
			case '_':
			case '=':
			case '`':
			case ' ':
			case '\r':
			case '\n':
			case '\t':
				if (str.size())
				{
					avlTree.Insert(str);
					bsTree.Insert(str);
					last = str;
				}
				
				str = "";
				continue;
			default:
				str += c;
				continue;
		}

	}
	ifsInput.close();
	//x.Output(Print);
	cout << "AVL Height: " << avlTree.Height() << endl;
	cout << "BST Height: " << bsTree.Height() << endl;
	cout << "AVL Size:   " << avlTree.Size() << endl;
	cout << "BST Size:   " << bsTree.Size() << endl;


	clock_t stime, etime;
	double elapse; 
	int i;
	
	cout << "Searching for \"" << last << "\"..." << endl;
	stime = clock();
	for (i = 0; i < 100000; i++)
		avlTree.Find(last);
	etime = clock();
	elapse =(double)(etime-stime)/CLOCKS_PER_SEC;
	cout << "Search in AVL tree took " << elapse << " seconds " << endl;


	stime = clock();
	for (i = 0; i < 100000; i++)
		bsTree.Find(last);
	etime = clock();
	elapse =(double)(etime-stime)/CLOCKS_PER_SEC;
	cout << "Search in BST tree took " << elapse << " seconds " << endl;

	avlTree.Output(Print);

	return 0;
}