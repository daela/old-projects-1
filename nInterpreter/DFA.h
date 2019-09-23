//-----------------------------------------------------------------------------
//    DFA.h
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#ifndef __DFA_H_JWM_
#define __DFA_H_JWM_
#pragma once

#include <set>
#include <string>
#include <fstream>
#include "IOHandler.h"
class DFAEdge
{
public:
	DFAEdge(char a = '\0');
	char ch;
	unsigned long id;
	set<DFAEdge> edges;
	bool operator<(const DFAEdge&obj) const;
};


class DFA
{
public:
	DFA(IOHandler*ioHandler);
	IOHandler*ioh;
	DFAEdge dfa;
	void AddString(int id, string objStr);
	int GetString(string&strFound);
	bool ValidFirst(char ch);
};

#endif