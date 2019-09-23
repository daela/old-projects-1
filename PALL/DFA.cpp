//-----------------------------------------------------------------------------
//    DFA.cpp
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#include "DFA.h"

DFAEdge::DFAEdge(char a)
{
	ch = a;
	id = 0;
}
bool DFAEdge::operator<(const DFAEdge&obj) const
{
	return (ch<obj.ch);
}


DFA::DFA(IOHandler*ioHandler)
{
	ioh = ioHandler;
}
void DFA::AddString(int id,string objStr)
{
	DFAEdge*ptDfa = &dfa;
	set<DFAEdge>::iterator edgeIt;
	int len = objStr.length();
	string::iterator strIt = objStr.begin();
	while  (strIt!=objStr.end())
	{
		edgeIt = ptDfa->edges.find(*strIt);
		if (edgeIt == ptDfa->edges.end())
		{
			ptDfa->edges.insert(DFAEdge(*strIt));
			edgeIt = ptDfa->edges.find(*strIt);
		}
		++strIt;
		if (strIt == objStr.end())
		{
			edgeIt->id = id;
		}
		else ptDfa = &(*edgeIt);
	}
}
int DFA::GetString(string&strFound)
{
	strFound =  "";
	DFAEdge*ptDfa = &dfa;
	set<DFAEdge>::iterator edgeIt;
	ioh->skipWS();
	while (true)
	{
		char ch;
		if (ioh->isEOF())
		{
			ch = '\0';
		}
		else
		{
			ch = ioh->toLower(ioh->nextChar());
		}
		edgeIt = ptDfa->edges.find(ch);
		if (edgeIt == ptDfa->edges.end())
		{
			return ptDfa->id;
		}
		strFound += ch;
		ioh->getChar();
		ptDfa = &(*edgeIt);
	}
	return 0;
}
bool DFA::ValidFirst(char ch)
{
	return (dfa.edges.find(ch) != dfa.edges.end());
}