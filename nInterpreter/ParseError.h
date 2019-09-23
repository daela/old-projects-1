//-----------------------------------------------------------------------------
//    ParseError.h
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#ifndef __PARSEERROR_H__JWM__
#define __PARSEERROR_H__JWM__
#pragma once
#include <string>
#include <sstream>
#include "Token.h"
using namespace std;
class ParseError
{
	public:
		ParseError(Token&obj,Token::TokenType expected);
		Token objToken;
		Token::TokenType tt;
		string toString();	
};
#endif