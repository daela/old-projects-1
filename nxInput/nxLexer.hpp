#ifndef NXLEXER_HPP_INCLUDED
#define NXLEXER_HPP_INCLUDED

#include <map>
#include <set>
#include <string>
#include <iostream>
class nxDFA_Node
{
	unsigned long uId;
	std::map<char,nxDFA_Node> mpInput;

	public:
	inline nxDFA_Node(const unsigned long & id=0)
	: uId(id)
	{ }
	const nxDFA_Node* Input(char ch) const;
	inline unsigned long GetIdentifier(void) const
	{ return uId; }

	inline void Clear(void)
	{
		mpInput.clear();
	}

	nxDFA_Node* Add(char ch,unsigned long id=0);
};
class nxDFA : public nxDFA_Node
{
	public:
	bool AddLexeme(const std::string&strLexeme,const unsigned long&id);
};


class nxLexer
{
	std::set<char> stIdentifierCharset;
	std::set<char> stOperatorCharset;
	nxDFA dfaIdentifiers;
	nxDFA dfaOperators;
	inline static bool IsWhitespace(const char&ch)
	{
		switch (ch)
		{
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				return true;
		}
		return false;
	}
	inline static bool IsNum(const char&ch)
	{
		return (ch >= '0' && ch <='9');
	}
	inline bool IsOperatorChar(const char&ch) const
	{
		return (stOperatorCharset.find(ch)!=stOperatorCharset.end());
	}
	inline bool IsIdentifierChar(const char&ch) const
	{
		return (stIdentifierCharset.find(ch)!=stIdentifierCharset.end());
	}
	public:
		enum { LEX_NONE=0,LEX_BADIDENT,LEX_IDENT,LEX_UNKNOWN_OPERATOR,LEX_OPERATOR,LEX_INTLIT,LEX_FLOLIT,LEX_USEROFFSET };
		bool AddIdentifier(const std::string&strIdentifier,const unsigned long&uId);
		bool AddOperator(const std::string&strOperator,const unsigned long&uId);
		nxLexer(const std::string&strIdentifierCharset,const std::string&strOperatorCharset);

		template<class T> unsigned long GetToken(T itStart,T itEnd,T&itOut,std::string&strData) const
		{

			unsigned long uToken=LEX_NONE;

			// Skip whitespace
			while (itStart != itEnd && IsWhitespace(*itStart))
				++itStart;
			// Identify the first character
			if (itStart != itEnd && *itStart != '\0')
			{
				// If it starts with a number, or if it doesn't start with an operator character
				enum { STATE_INT, STATE_FLOAT, STATE_OPERATOR, STATE_IDENTIFIER, STATE_INVALID_IDENTIFIER };
				const nxDFA_Node*ptDFA=NULL;
				const nxDFA_Node*ptLastGood=NULL;
				std::string::size_type uLastGoodSize=1;
				T itLastGood=itStart;
				++itLastGood;

				unsigned char uState;
				strData.clear();

				if (IsNum(*itStart))
					uState=STATE_INT;
				else if (IsOperatorChar(*itStart))
				{
					uState = STATE_OPERATOR;
					ptDFA = &dfaOperators;
				}
				else if (IsIdentifierChar(*itStart))
				{
					uState = STATE_IDENTIFIER;
					ptDFA = &dfaIdentifiers;
				}
				else
					uState = STATE_INVALID_IDENTIFIER;


				while (itStart != itEnd && *itStart != '\0' && !IsWhitespace(*itStart))
				{
				    // Keep up with DFA matches
					if (ptDFA) ptDFA=ptDFA->Input(*itStart);
					// we stop reading identifiers when we find an operator char, or whitespace (handled in the while condition)
					if (uState == STATE_INT && (*itStart == '.'))
					{
						uState = STATE_FLOAT;
					}
					else if (uState != STATE_OPERATOR)
					{
						if (IsOperatorChar(*itStart))
								break;
						if (uState == STATE_INT || uState == STATE_FLOAT)
						{
							if (!IsNum(*itStart))
									uState = STATE_INVALID_IDENTIFIER;
						}
					}
					else // operator
					{

						if (!IsOperatorChar(*itStart))
							break;
					}

                    strData += *itStart;
                    if (uState==STATE_OPERATOR && !ptDFA)
                    {
                        // revert to the last good iterator (defaults to one beyond start)
                        itStart = itLastGood;
                        // revert string size to last good size (defaults to one)
                        strData.resize(uLastGoodSize);
                        // rever the DFA pointer
                        ptDFA = ptLastGood;
                        break;
                    }

                    ++itStart;
					if (ptDFA && uState==STATE_OPERATOR && ptDFA->GetIdentifier())
					{
					    itLastGood = itStart;
					    ptLastGood = ptDFA;
					    uLastGoodSize = strData.size();
					}
				}

				if (strData.empty())
				{
					uToken=LEX_NONE;
				}
				else
				{
					switch (uState)
					{
						case STATE_IDENTIFIER:
						{

							if (ptDFA)
								uToken=ptDFA->GetIdentifier();
							if (!uToken)
								uToken = LEX_IDENT;
							break;
						}
						case STATE_OPERATOR:
						{
							if (ptDFA)
								uToken=ptDFA->GetIdentifier();
							if (!uToken)
								uToken = LEX_UNKNOWN_OPERATOR;
							break;
						}
						case STATE_INVALID_IDENTIFIER:
							uToken=LEX_BADIDENT;
							break;
						case STATE_INT:
							uToken=LEX_INTLIT;
							break;
						case STATE_FLOAT:
							uToken=LEX_FLOLIT;
							break;
					}
				}


			}
			itOut = itStart;
			return uToken;

		}
};


#endif // NXLEXER_HPP_INCLUDED
