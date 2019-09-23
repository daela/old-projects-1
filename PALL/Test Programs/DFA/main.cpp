
#include <set>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

class DFAEdge
{
public:
	DFAEdge(char a = '\0')
	{
		ch = a;
		id = 0;
	}
	char ch;
	unsigned long id;
	set<DFAEdge> edges;
	bool operator<(const DFAEdge&obj) const
	{
		return (ch<obj.ch);
	}
};


class DFA
{
public:
	DFAEdge dfa;
	void AddString(string objStr,int id)
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
	int GetString(string&strFound,istream&ifs)
	{
		strFound =  "";
		DFAEdge*ptDfa = &dfa;
		set<DFAEdge>::iterator edgeIt;
		int iChr;
		//skip leading WS
		while (true)
		{
			iChr=ifs.peek();
			if  (iChr == char_traits<char>::eof())
			{
				ifs.clear();
				break;
			}
			switch (iChr)
			{
				case '\r':
				case '\n':
				case '\t':
				case ' ':
					ifs.get();
					continue;
			}
			break;
		}
		while (true)
		{
			char ch;
			if ((iChr=ifs.peek()) == char_traits<char>::eof())
			{
				ifs.clear();
				ch = '\0';
			}
			else
			{
				ch = (char)iChr;
			}
			edgeIt = ptDfa->edges.find(ch);
			if (edgeIt == ptDfa->edges.end())
			{
				return ptDfa->id;
			}
			strFound += ch;
			ifs.get();
			ptDfa = &(*edgeIt);
		}
		return 0;
	}
};



void main(void)
{
	DFA myDfa;
	myDfa.AddString("hello",1);
	myDfa.AddString("howdy",2);
	myDfa.AddString("hell",3);

	string result;
	stringstream ss("howdy  hello  hell  howdy  hello help");
	int id;
	while (true)
	{
		id=myDfa.GetString(result,ss);
		if (id != 0)
			cout << result << endl;
		else
		{
			cout << result << (char)ss.get() <<endl;
			break;
		}
	}
}
