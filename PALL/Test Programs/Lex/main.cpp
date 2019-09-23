#include <string>
#include <iostream>
#include <fstream>
#include <set>
using namespace std;

class DFA
{
	public:
		DFA(char c='\0') { ch = c; strToken = ""; }
		char ch;
		string strToken;
		set<DFA> stChildren;
		void AddString(string strToken,string strText,bool isOperator)
		{
			DFA*ptDFA = this;
			for (string::iterator it = strText.begin();it != strText.end();++it)
				ptDFA = &(*(ptDFA->stChildren.insert(DFA(*it)).first()));
			ptDFA->strToken = strToken;
			if (isOperator) stOperators.insert(strText[0]);
		}
		static inline bool IsNum(const char&ch)
		{
			return ((ch >= '0' && ch <= '9'));
		}
		static inline bool IsAlpha(const char&ch)
		{
			return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
		}
		static inline bool IsAlphaU(const char&ch)
		{
			return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_');
		}
		static inline bool IsAlphaNumeric(const char&ch)
		{
			return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')  || (ch >= '0' && ch  <= '9'));
		}
		static inline bool IsWS(const char&ch)
		{
			return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
		}
		string GetToken(istream&ifs)
		{
			int obj;
			DFA*ptDFA = this;
			int mode = 0;
			vector<char> vcInput;
			while ((obj=ifs.peek()) != char_traits<char>::eof())
			{
				char ch = (char)obj;
				if (mode == 0)
				{
					//Skip leading whitespace
					if (IsWS(ch))
					{
						ifs.get();
						continue;
					}
					set<DFA>::iterator it;
					if (IsAlphaU(ch))
						mode = 1;
					else if (IsNum(ch))
						mode = 2;
					else
						mode = 3; //symbol
				}
				if (mode == 1)
				{
					if (IsAlphaU(ch))
					{
						if (ptDFA && (it=ptDFA->stChildren.find(DFA(ch))) != stChildren.end())
							ptDFA = &(*it);
						else
							ptDFA = NULL;
					}
					else
						break;
				}
				if (mode == 2)
				{
					if ((it=ptDFA->stChildren.find(DFA(ch))) != stChildren.end())
						ptDFA = &(*it);
					else
						break;
				}
				ifs.get();
				set<DFA>::iterator it;
				
				
				if ((it=ptDFA->stChildren.find(DFA((char)obj))) != ptDFA->stChildren.end())
				{
					ptDFA = &(*it);
				}
				else
				{
					
				}
			}
		}
};


void main(void)
{


}