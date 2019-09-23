//-----------------------------------------------------------------------------
//    ParseError.cpp
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#include "ParseError.h"

ParseError::ParseError(Token&obj,Token::TokenType expected)
{
	objToken = obj;
	tt = expected;
}

string ParseError::toString()
{
	string output = string("expected ") + Token::TokenIdToString(tt) + " found " + Token::TokenIdToString(objToken.iNum);
	if  (objToken.iNum == Token::LEX_ERROR)
	{
		stringstream ss("");
		ss << objToken.dtValue;
		output += string("(") + ss.str() + ")";
	}
	return output;
}
