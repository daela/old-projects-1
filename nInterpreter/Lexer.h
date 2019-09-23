//-----------------------------------------------------------------------------
//    Lexer.h
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#ifndef __LEXER_H_JWM__
#define __LEXER_H_JWM__
#pragma once

#include "DFA.h"
#include "IOHandler.h"
#include <iostream>
#include <algorithm>
#include "Token.h"
using namespace std;

class Lexer : public IOHandler
{
public:
	DFA dfaKeywords;
	DFA dfaOperators;
	Lexer();
	void Init();
	bool GetToken(Token&obj);

};
#endif
