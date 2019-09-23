//-----------------------------------------------------------------------------
//    Executer.h
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#ifndef __EXECUTER_H_JWM__
#define __EXECUTER_H_JWM__
#pragma once
#include "SymbolTable.h"
#include "DataValue.h"
#include "Token.h"
#include <iostream>
#include <vector>
#include <conio.h>
using namespace std;
class Executer
{
public:
	SymbolTable symTable;
	void Execute(list<Token>&lstTree);
	DataValue TableWalk(list<Token>&lstTree);
	DataValue ProcessFactor(Token&obj);
	DataValue ProcessTerm(Token&obj);
	DataValue*ProcessDesignator(Token&obj,vector<int>&subscripts);
	DataValue ProcessExpression(Token&obj);
	DataValue ProcessSimpleExpression(Token&obj);
	vector<DataValue> vcExpressions;

	bool isBody;

};

#endif