//-----------------------------------------------------------------------------
//				Driver.cpp
//
//	Author:		Jacob McIntosh
//	Section:	01
//	Date:		March 30, 2004
//
//	Driver.cpp is the test driver for the Heap class
//
//-----------------------------------------------------------------------------

#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h> //sprintf
#include "Heap.h"
using namespace std;

//this will only work for ints, would make it generic with a stringstream
//but the g++ on the ra server's stringstream is broken.... :/
string HeapString(const vector<int> & vc)
{
	vector<int>::const_iterator cit, cend;
	string str;
	char buf[10];
	for (cit = vc.begin(), cend = vc.end();cit != cend;++cit)
	{
		if (cit != vc.begin())
			str += " ";
		sprintf(buf,"%d",*cit);
		str += buf;
	}
	return str;
}

int main(void)
{
	Heap<int> hp;
	vector<int> vc;

	cout << "Testing heapify example from slides.." << endl;
	hp.clear();
	hp.insert(4);
	hp.insert(1);
	hp.insert(3);
	hp.insert(2);
	hp.insert(16);
	hp.insert(9);
	hp.insert(10);
	hp.insert(14);
	hp.insert(8);
	hp.insert(7);
	hp.heapify();
	cout << "Input:           4 1 3 2 16 9 10 14 8 7" << endl;
	cout << "Output:          " << HeapString(hp.buffer()) << endl;
	cout << "Expected Output: 16 14 10 8 7 9 3 2 4 1" << endl;
	cout << endl;
	hp.clear();


	cout << "Testing insertion example from slides.." << endl;
	hp.clear();
	hp.insert(45);
	hp.insert(29);
	hp.insert(35);
	hp.insert(24);
	hp.insert(27);
	hp.insert(23);
	hp.insert(23);
	hp.insert(15);
	hp.insert(18);
	hp.insert(20);
	hp.insert(22);
	hp.heapify();
	hp.push(39);
	cout << "Input:           45 29 35 24 27 23 23 15 18 20 22, Pushing: 39" << endl;
	cout << "Output:          " << HeapString(hp.buffer()) << endl;
	cout << "Expected Output: 45 29 39 24 27 35 23 15 18 20 22 23" << endl;
	cout << endl;
	hp.clear();

	
	cout << "Testing deletion example from slides..." << endl;
	hp.insert(45);
	hp.insert(29);
	hp.insert(35);
	hp.insert(24);
	hp.insert(27);
	hp.insert(23);
	hp.insert(23);
	hp.insert(15);
	hp.insert(18);
	hp.insert(20);
	hp.insert(22);
	hp.heapify();
	hp.pop();
	cout << "Input:           45 29 35 24 27 23 23 15 18 20 22" << endl;
	cout << "Output:          " << HeapString(hp.buffer()) << endl;
	cout << "Expected Output: 35 29 23 24 27 22 23 15 18 20" << endl;
	cout << endl;
	hp.clear();


	cout << "Testing heapsort example from slides..." << endl;
	hp.insert(16);
	hp.insert(14);
	hp.insert(10);
	hp.insert(8);
	hp.insert(7);
	hp.insert(9);
	hp.insert(3);
	hp.insert(2);
	hp.insert(4);
	hp.insert(1);
	hp.heapify();
	vc = hp.sort();
	cout << "Input:           16 14 10 8 7 9 3 2 4 1" << endl;
	cout << "Output:          " << HeapString(vc) << endl;
	cout << "Expected Output: 1 2 3 4 7 8 9 10 14 16" << endl;
	cout << endl;
	hp.clear();

	return 0;
}
