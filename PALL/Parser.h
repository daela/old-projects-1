//-----------------------------------------------------------------------------
//    Parser.h
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#ifndef __PARSER_H_JWM__
#define __PARSER_H_JWM__
#pragma once

#include "Lexer.h"
#include "ParseError.h"
#include "SymbolTable.h"

class Parser
{
public:
	Parser(Lexer*lex);
	SymbolTable symTable;

	list<Token> lstParseTree;
	
	// Pointer to the lexer
	Lexer*ptLex;
	int spaces;
	void PrintTree(ostream&);
	void TreeWalk(list<Token>&,ostream&);
	bool Validate(); //same as <S>
	bool _program(list<Token>&,bool=false);
	bool _declarations(list<Token>&,bool=false);
	bool _function_decl(list<Token>&,bool=false);
	bool _array_decl(list<Token>&,bool=false);
	bool _variable_decl(list<Token>&,bool=false);
	bool _type(list<Token>&,bool=false);
	bool _stmt_sequence(list<Token>&,bool=false);
	bool _assignment_stmt(list<Token>&,bool=false);
	bool _input_stmt(list<Token>&,bool=false);
	bool _output_stmt(list<Token>&,bool=false);
	bool _if_stmt(list<Token>&,bool=false);
	bool _while_stmt(list<Token>&,bool=false);
	bool _return_stmt(list<Token>&,bool=false);
	bool _designator(list<Token>&,bool=false);
	bool _expression(list<Token>&,bool=false);
	bool _simple_expression(list<Token>&,bool=false);
	bool _term(list<Token>&,bool=false);
	bool _factor(list<Token>&,bool=false);


	void checkIdent(Token&objToken,Token::TokenType tt,list<Token>&lstChildren);
	void outputError(string strFunction, ParseError perr);
	bool isIdent(Token&objToken,Token::TokenType tt,list<Token>&lstChildren);

};

#endif