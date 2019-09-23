#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <map>
#include <iterator>
#include <container/DFA.hpp>
#include <nxFile.hpp>
/*
class Lexer
{
	public:
	//	typedef std::istream_iterator<char> file_iterator;
		DFA dfaKeywords;
		// operators _and_ whitespace can divide tokens.
		// var+var, for example, is not one token
		DFA dfaOperators;

		inline void Init()
		{
			dfaKeywords.AddIdentifier(1,"string");
			dfaKeywords.AddIdentifier(2,"int");
			dfaOperators.AddIdentifier(3,"+=");
			dfaOperators.AddIdentifier(4,"-=");
			dfaOperators.AddIdentifier(5,"!=");
			dfaOperators.AddIdentifier(6,"==");
			dfaOperators.AddIdentifier(7,"+");
			dfaOperators.AddIdentifier(8,"-");
		}
		inline static bool IsWS(const char&ch)
		{
			switch (ch)
			{
				case ' ':
				case '\r':
				case '\n':
				case '\t':
					return true;
			}
			return false;
		}
		inline static bool IsNum(const char&ch)
		{
			return (ch >= '0' && ch <= '9');
		}
		inline static bool IsQuote(const char&ch)
		{
			return (ch == '\'' || ch == '\"');
		}

		template <class T>
		static void SkipWS(T&it,const T&itEnd)
		{
			while (it != itEnd)
				if (IsWS(*it))
					++it;
				else
					return;
		}

		template <class T>
		unsigned long GetToken(T&it, const T&itEnd) const
		{
			// Skip any whitespace that may precede the token
			SkipWS(it,itEnd);
			// Check for EOF
			if (it == itEnd) return 0;
			// Check if the first character is an operator character or a keyword character
			const DFA*ptDFA = dfaOperators.Input(*it);
			const DFA*ptOther;
			// Set the appropriate inverse (operators and keywords can't share the same characters)
			// this approach allows bla+bla, whitespace doesnt have to be present.
			if (ptDFA)
				ptOther = &dfaKeywords;
			else
			{
				ptDFA = dfaKeywords.Input(*it);
				ptOther = &dfaOperators;
			}
			//Keep a string of the lexeme in case it's not an operator or keyword
			std::string strBuf;
			strBuf += *it;
			++it;
			//TODO CHECK FOR CHAR/STRLIT/INT/FLOLIT (can do it based on the first character)
			if (IsQuote(*it))
			{
				//check for a quote

			}
			else
			{
				bool bIsNum = IsNum(*it);;

				// While we haven't hit EOF, the next charcter isn't whitespace and it isn't the first character of
				// a lexeme from the other category
				while (it != itEnd && !IsWS(*it) && !ptOther->Input(*it))
				{
					if (!IsNum(*it)) bIsNum = false;
					// Update Our DFA state
					if (ptDFA) ptDFA = ptDFA->Input(*it);
					strBuf += *it;
					++it;
				}
				// If the DFA isn't NULL then we'll output the ID (if valid)
				if (ptDFA && ptDFA->GetID())
				{
					return ptDFA->GetID();
				}
				else if (bIsNum)
				{
					std::cout << "Integer Literal: " << strBuf;
					return 100;
				}
			}
			std::cout << "Unrecognized: " << strBuf;
			return 99;
		}
};

*/



class Lexer
{
	protected:
		DFA objKeywords;
		DFA objOperators;
		bool Init(void);
	public:

		enum { T_NONTERMINAL=0, T_INTLIT, T_FLOLIT, T_STRLIT, T_STRING, T_PLUSEQUAL, T_COUNT};
		enum { E_OPENSTRING=T_COUNT, E_BADESCAPE, E_IDENTINT, E_IDENTFLOAT };
		//error >= t_count
		//state < t_count
		Lexer(void)
		{
			std::string strNumbers = "0123456789";
			DFA_Node*ptDFA;
			//DFA_Node&objNull = *objDFA.State(T_NONTERMINAL);

			/// Recognize INTLIT, FLOLIT, and identifiers starting with a number
			// Set the transition for the first number to go to the INTLIT state
			ptDFA=objKeywords.SetTransition(strNumbers,T_INTLIT);
			// Make any further numbers stay in this state
			ptDFA->SetTransition(strNumbers);
			// Set the default (to pick up characters) as an "idents can't start with a number" error
			ptDFA->SetTransition('\0',E_IDENTINT)->SetTransition('\0');
			// Make a decimal point enter into a nonterminal state (needs numbers after the decimal)
			ptDFA = ptDFA->SetTransition('.',T_NONTERMINAL);
			// Make any number enter the float state (after the decimal)
			ptDFA = ptDFA->SetTransition(strNumbers,T_FLOLIT);
			// Make any further numbers stay in this state
			ptDFA->SetTransition(strNumbers);
			// Default, idents can't start with a float
			ptDFA->SetTransition('\0',E_IDENTFLOAT)->SetTransition('\0');

			/// Recognize Strings
			ptDFA = objKeywords.SetTransition('"',E_OPENSTRING)->SetTransition('\0')->SetTransition('"',T_STRLIT); //will be in this state if newline occurs

			/// Add Keywords
			objKeywords.AddString("string",T_STRING);


			objOperators.AddString("+=",T_PLUSEQUAL);
		}
		template <class T>
		unsigned long GetToken(T&it, const T&itEnd) const
		{
			//skip whitespace
			//if the first char read is an operator character, start looking for whitespace _or_ non-operator chars
			//if the first char is a non-operator char, start looking for whitespace _or_ an operator char (if state isn't E_OPENSTRING)


			if (it != itEnd)
			{
				DFA_Node* ptDFA;

				if (objOperators.Input(*it) != &objOperators.NullState())
				{
					ptDFA = (DFA_Node*)&objOperators;


			}
			return 0;
		}

};


int main(int argc,char*argv[])
{
	DFA obj;
	obj.AddString("string",1);
	obj.AddString("str",2);
	//std::cout << obj.CheckString("str") << std::endl;
	//std::cout << obj.CheckString("string") << std::endl;
	DFA_Node*ptDFA = obj.SetTransition("0123456789",3);
	ptDFA->SetTransition("0123456789",3);
	ptDFA = ptDFA->SetTransition(".",0);
	ptDFA = ptDFA->SetTransition("0123456789",4);
	ptDFA = ptDFA->SetTransition("0123456789",4);
	std::cout << obj.CheckString("01234123123123541") << std::endl;
	std::cout << obj.CheckString("01234123123123541.") << std::endl;
	std::cout << obj.CheckString("01234123123123541.1") << std::endl;
	std::cout << obj.CheckString("01234123123123541.1234145.213") << std::endl;
	/*DFA dfaNumbers(999);
	dfaNumbers.SetLoop("0123456789");
	std::string strTest = "string int float";

	obj.SetEdge("0123456789",dfaNumbers);


	obj.AddIdentifier(1,"string");
	obj.AddIdentifier(2,"float");
	obj.AddIdentifier(3,"flo");
	std::cout << obj.GetIdentifier("float") << std::endl;
	std::cout << obj.GetIdentifier("flo") << std::endl;
	std::cout << obj.GetIdentifier("str") << std::endl;
	std::cout << obj.GetIdentifier("12345") << std::endl;
	std::cout << obj.GetIdentifier("123459") << std::endl;
	std::cout << obj.GetIdentifier("string") << std::endl;*/

//	nxFile objFile("c:\\test.txt");
	//nxFile::const_iterator it = objFile.begin();
	//nxFile::const_iterator itEnd = objFile.end();

	/*Lexer objLexer;
	objLexer.Init();
	nxFile::const_iterator it = objFile.begin();
	unsigned long ret;
	while (ret=objLexer.GetToken(it,objFile.end()))
		std::cout << ret << std::endl;*/
	//objLexer.GetToken(it,objFile.end());
	//objLexer.GetToken(it,objFile.end());

	return 0;
}
