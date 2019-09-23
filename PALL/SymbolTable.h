//-----------------------------------------------------------------------------
//    SymbolTable.h
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#ifndef __SYMBOLTABLE_H_JWM__
#define __SYMBOLTABLE_H_JWM__
#pragma once
#include "Token.h"
#include "DataValue.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
using namespace std;

class Symbol
{
public:
	string strName;
	vector<int> vcSubscripts;
	vector<DataValue> vcValues;
	Token::TokenType data_type;
	unsigned long CalculateOffset(vector<int>&subscripts);
	Token::TokenType DataTypeToTokenType(DataValue::DataType dt);
	bool SetValue(vector<int>&subscripts,DataValue obj);
	DataValue GetValue(vector<int>&subscripts);
	DataValue*GetValuePtr(vector<int>&subscripts);
	void SetSize(vector<int>&subscripts);
	bool CreateSymbol(Token&objToken);
};

class SymbolTable
{
public:
	vector<Symbol> vcSymbols;
	bool AddSymbol(Symbol obj);
	int SetValue(string strName,vector<int>&vcSubscripts,DataValue obj);
	Symbol*GetSymbol(string name);
	DataValue GetSymbolValue(string name,vector<int>&vcSubscripts);
	void Clear();
};




#endif