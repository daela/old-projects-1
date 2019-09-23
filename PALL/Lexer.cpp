//-----------------------------------------------------------------------------
//    Lexer.cpp
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#include "Lexer.h"

Lexer::Lexer() : dfaKeywords(this),dfaOperators(this)
{ }
void Lexer::Init()
{
	dfaOperators.AddString(Token::LEX_LPAREN,"(");
	dfaOperators.AddString(Token::LEX_RPAREN,")");
	dfaOperators.AddString(Token::LEX_LBRACK,"[");
	dfaOperators.AddString(Token::LEX_RBRACK,"]");
	dfaOperators.AddString(Token::LEX_COMMA,",");
	dfaOperators.AddString(Token::LEX_COLON,":");
	dfaOperators.AddString(Token::LEX_SEMI,";");
	dfaOperators.AddString(Token::LEX_QUOTE,"'");
	dfaOperators.AddString(Token::LEX_DQUOTE,"\"");
	dfaOperators.AddString(Token::LEX_PERIOD,".");
	dfaOperators.AddString(Token::LEX_POS,"#");
	dfaOperators.AddString(Token::LEX_NEG,"~");
	dfaOperators.AddString(Token::LEX_PLUS,"+");
	dfaOperators.AddString(Token::LEX_MINUS,"-");
	dfaOperators.AddString(Token::LEX_TIMES,"*");
	dfaOperators.AddString(Token::LEX_DIV,"/");
	dfaOperators.AddString(Token::LEX_MOD,"%");
	dfaOperators.AddString(Token::LEX_EQ,"=");
	dfaOperators.AddString(Token::LEX_NE,"!=");
	dfaOperators.AddString(Token::LEX_GT,">");
	dfaOperators.AddString(Token::LEX_LT,"<");
	dfaOperators.AddString(Token::LEX_GE,">=");
	dfaOperators.AddString(Token::LEX_LE,"<=");
	dfaOperators.AddString(Token::LEX_AND,"&");
	dfaOperators.AddString(Token::LEX_OR,"|");
	dfaOperators.AddString(Token::LEX_NOT,"!");
	dfaKeywords.AddString(Token::LEX_ALGO,"algorithm");
	dfaKeywords.AddString(Token::LEX_ASSN,"assign");
	dfaKeywords.AddString(Token::LEX_BEGIN,"begin");
	dfaKeywords.AddString(Token::LEX_BOOL,"boolean");
	dfaKeywords.AddString(Token::LEX_CHAR,"character");
	dfaKeywords.AddString(Token::LEX_DEFAR,"defarray");
	dfaKeywords.AddString(Token::LEX_DEFUN,"defun");
	dfaKeywords.AddString(Token::LEX_DEFVAR,"defvar");
	dfaKeywords.AddString(Token::LEX_DO,"do");
	dfaKeywords.AddString(Token::LEX_ELSE,"else");
	dfaKeywords.AddString(Token::LEX_END,"end");
	dfaKeywords.AddString(Token::LEX_ENDFUN,"endfun");
	dfaKeywords.AddString(Token::LEX_ENDIF,"endif");
	dfaKeywords.AddString(Token::LEX_ENDALG,"endalg");
	dfaKeywords.AddString(Token::LEX_ENDDO,"endwhile");
	dfaKeywords.AddString(Token::LEX_FALSE,"false");
	dfaKeywords.AddString(Token::LEX_IF,"if");
	dfaKeywords.AddString(Token::LEX_INT,"integer");
	dfaKeywords.AddString(Token::LEX_READ,"read");
	dfaKeywords.AddString(Token::LEX_REAL,"real");
	dfaKeywords.AddString(Token::LEX_RETURN,"return");
	dfaKeywords.AddString(Token::LEX_THEN,"then");
	dfaKeywords.AddString(Token::LEX_TO,"to");
	dfaKeywords.AddString(Token::LEX_TRUE,"true");
	dfaKeywords.AddString(Token::LEX_VOID,"void");
	dfaKeywords.AddString(Token::LEX_WHILE,"while");
	dfaKeywords.AddString(Token::LEX_WRITE,"write");
}
bool Lexer::GetToken(Token&obj)
{
	string strBuf;
	int id;
	DataValue dv;
	while (!isEOF())
	{
		skipWS();
		/////////////
		//  Check for chr/strlit
		//
		if (nextChar() == '-')
			bool b=true;
		if (nextChar() == '\'' || nextChar() == '\"')
		{
			char ch = getChar();
			strBuf =  "";
			while (!isEOF())
			{
				if  (nextChar() == '\r' || nextChar() == '\n')
				{
					dv.SetStrData("Newline in constant.");
					throw Token(Token::LEX_ERROR,dv);
				}
				if (nextChar() == ch)
				{
					getChar();
					if (ch == '\'')
					{
						if (strBuf.length() != 1)
						{
							dv.SetStrData("Character literals must have exactly one character.");
							throw Token(Token::LEX_ERROR,dv);
						}
						dv.SetCharData(strBuf[0]);
						obj=Token(Token::LEX_CHRLIT,dv);
						return true;
					}
					else
					{
						dv.SetStrData(strBuf);
						obj=Token(Token::LEX_STRLIT,dv);
						return true;
					}
				}
				strBuf += getChar();
			}
			dv.SetStrData("EOF in constant.");
			throw Token(Token::LEX_ERROR,dv);
		}
		/////////////
		//  Check for Operators, then keywords
		//
		StorePosition();
		
		id = dfaOperators.GetString(strBuf);
		if (id == 0)
		{
			ResetPosition();
			id = dfaKeywords.GetString(strBuf);
			if (id == 0)
			{

				/////////////
				//  Check for numbers/idents
				//
				while (!isEOF() && !isWS(nextChar()) && !dfaOperators.ValidFirst(nextChar()))
				{
					strBuf += toLower(getChar());
				}
				//if it's all numbers
				if (strBuf.find_first_not_of("0123456789",0,10) == string::npos)
				{
					//read in anything, including dots  (will cover floats and invalid idents)
					while (!isEOF() && !isWS(nextChar()) && (nextChar() == '.' || !dfaOperators.ValidFirst(nextChar())))
					{
						strBuf += toLower(getChar());
					}
				}
				bool found = false;
				if (strBuf.find_first_not_of("0123456789.",0,11) == string::npos)
				{
					string::size_type off;
					off = strBuf.find('.',0);
					if (off == string::npos)
					{
						dv.SetIntData(atoi(strBuf.c_str()));
						obj=Token(Token::LEX_INTLIT,dv);
						return true;
					}
					else
					{
						if (strBuf.find('.',off+1) == string::npos)
						{
							dv.SetFloatData(atof(strBuf.c_str()));
							obj=Token(Token::LEX_FLOLIT,dv);
							return true;
						}
					}
				}
				//validate identifier
				try
				{
					if (strBuf.length() > 20)
						throw 2;
					for (string::iterator it = strBuf.begin();it != strBuf.end();++it)
					{
						if (it == strBuf.begin())
						{
							if (!isAlpha(*it))
							{
								throw 0;
							}
						}
						if (!isAlpha(*it) && *it != '_' && !isNum(*it))
						{
							throw 1;
						}
					}
				} catch(int iError)
				{
					switch (iError)
					{
						case 0:
							dv.SetStrData("Unrecognized lexeme ("+strBuf+"): identifiers must begin with a letter.");
							throw Token(Token::LEX_ERROR,dv);
						case 1:
							dv.SetStrData("Unrecognized lexeme ("+strBuf+"): identifiers can only contain underscores and alphanumeric characters.");
							throw Token(Token::LEX_ERROR,dv);
						case 2:
							dv.SetStrData("Unrecognized lexeme ("+strBuf+"): identifiers can be at most 20 characters long.");
							throw Token(Token::LEX_ERROR,dv);
					}
				}
				dv.SetStrData(strBuf);
				obj=Token(Token::LEX_IDENT,dv);
				return true;
			}
		}
		if (id)
		{
			dv.Clear();
			obj=Token((Token::TokenType)id,dv);
			return true;
		}
		dv.SetStrData("Undefined error.");
		throw Token(Token::LEX_ERROR,dv);
	}
	//int 0 means eof
	dv.SetStrData("EOF");
	obj=Token(Token::LEX_ERROR,dv);
	return false;
}