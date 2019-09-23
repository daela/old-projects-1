//-----------------------------------------------------------------------------
//				Driver.cpp
//
//	Author:		Jacob McIntosh
//	Section:	01
//	Date:		April 15, 2004
//
//	Driver.cpp is the test driver for the DiGraph class
//
//-----------------------------------------------------------------------------

#include <fstream>
#include <iostream>
#include "digraph.h"

using namespace std;

void OutputBool(bool b)
{
	if (b)
		cout << "true";
	else
		cout << "false";
}

void OutputVector(vector<int>&vcPath)
{

	if (vcPath.size())
	{
		vector<int>::iterator it, end;
		for (it = vcPath.begin(), end = vcPath.end();it!=end;++it)
		{
			cout << *it << ' ';
		}
	}
	else
		cout << "No Path.";
}
void main(void)
{
	DiGraph<char> di;
	

	ifstream ifsInput("input.txt");
	if (!ifsInput)
		return;
	vector<int> vcPath;

	
	di.AddVertex('A');
	di.AddVertex('B');
	di.AddVertex('C'); 
	di.Print(cout);
	cout << endl << endl;

	di.AddEdge(1,2);
	di.AddEdge(3,2);
	di.Print(cout);
	cout << endl << endl;

	di.RemoveEdge(1,2);
	di.Print(cout);
	cout << endl << endl;

	OutputBool(di.HasEdge(3,2)); 	cout << endl;
	OutputBool(di.HasEdge(3,1));    cout << endl;
	cout << endl << endl;

	cout << di.GetData(3) << " ";
	cout << di.GetData(2) << " ";
	cout << di.GetData(1) << endl;
	cout << endl << endl;

	di = DiGraph<char>();
	di.Read(ifsInput);

	di.Print(cout); // tests both print and read
	cout << endl << endl;

	cout << "Same as previous test. (function already called)";
	cout << endl << endl;

	OutputBool(di.DepthFirstSearch(1,7));	cout << endl;
	OutputBool(di.DepthFirstSearch(4,5));	cout << endl;
	OutputBool(di.DepthFirstSearch(2,8));	cout << endl;
	cout << endl << endl;

	OutputVector(di.ShortestPath(1,5));	cout << endl;
	OutputVector(di.ShortestPath(5,1));	cout << endl;
	OutputVector(di.ShortestPath(8,4));	cout << endl;
	cout << endl << endl;

	ifsInput.close();
}