//-----------------------------------------------------------------------------
//    Token.h
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#ifndef __TOKEN_H_JWM__
#define __TOKEN_H_JWM__
#pragma once

#include "DataValue.h"
#include <string>
#include <set>
#include <list>
using namespace std;
class Token
{
public:
	enum TokenType
	{
		LEX_ERROR=0, LEX_LPAREN, LEX_RPAREN, LEX_LBRACK, LEX_RBRACK, LEX_COMMA,
		LEX_COLON, LEX_SEMI, LEX_QUOTE, LEX_DQUOTE, LEX_PERIOD, LEX_POS,
		LEX_NEG, LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_DIV, LEX_MOD, LEX_EQ,
		LEX_NE, LEX_GT, LEX_LT, LEX_GE, LEX_LE, LEX_AND, LEX_OR, LEX_NOT,
		LEX_ALGO, LEX_ASSN, LEX_BEGIN, LEX_BOOL, LEX_CHAR, LEX_DEFAR,
		LEX_DEFUN, LEX_DEFVAR, LEX_DO, LEX_ELSE, LEX_END, LEX_ENDFUN,
		LEX_ENDIF, LEX_ENDALG, LEX_ENDDO, LEX_FALSE, LEX_IF, LEX_INT,
		LEX_READ, LEX_REAL, LEX_RETURN, LEX_THEN, LEX_TO, LEX_TRUE,
		LEX_VOID, LEX_WHILE, LEX_WRITE, LEX_IDENT, LEX_CHRLIT,
		LEX_STRLIT, LEX_INTLIT, LEX_FLOLIT, LEX_NONTERMINAL
	};
	TokenType iNum;			// Token number
	DataValue dtValue;
	list<Token> lstChildren;
	static string TokenIdToString(TokenType tt);
	void AddChild(Token&);
	Token(TokenType num=LEX_ERROR,DataValue dv=DataValue());
};

#endif