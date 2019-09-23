//-----------------------------------------------------------------------------
//    SymbolTable.cpp
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#include "SymbolTable.h"

unsigned long Symbol::CalculateOffset(vector<int>&subscripts)
{
	if (subscripts.size() != vcSubscripts.size())
		return 0;
	unsigned long offset = 0, total_size = 1;
		//[3][4][5]
	vector<int>::reverse_iterator rPit,rSit;
	for (rPit=subscripts.rbegin(),rSit=vcSubscripts.rbegin();rPit!=subscripts.rend();++rPit,++rSit)
	{
		//[2][2][2]
		//2 * 1 = 2
		//
		offset += *rPit * total_size;
		total_size *= *rSit;
	}
	return offset;
}
Token::TokenType Symbol::DataTypeToTokenType(DataValue::DataType dt)
{
	switch (dt)
	{
		case DataValue::DT_INT:
			return Token::LEX_INT;
		case DataValue::DT_FLOAT:
			return Token::LEX_REAL;
		case DataValue::DT_CHAR:
			return Token::LEX_CHAR;
		case DataValue::DT_BOOL:
			return Token::LEX_BOOL;
		default:
			;
	}
	return Token::LEX_ERROR;
}
bool Symbol::SetValue(vector<int>&subscripts,DataValue obj)
{
	if (subscripts.size() == vcSubscripts.size())
	{
		unsigned long offset = CalculateOffset(subscripts);
		if (data_type == DataTypeToTokenType(obj.GetType()))
		{
			vcValues[offset] = obj;
			return true;
		}
		else if (data_type == Token::LEX_REAL && DataTypeToTokenType(obj.GetType()) == Token::LEX_INT)
		{
			vcValues[offset].SetFloatData((float)obj.GetIntData());
			return true;
		}
	}
	return false;
}
DataValue Symbol::GetValue(vector<int>&subscripts)
{
	if (subscripts.size() == vcSubscripts.size())
	{
		unsigned long offset = CalculateOffset(subscripts);
		return vcValues[offset];
	}
	return DataValue();
}
DataValue*Symbol::GetValuePtr(vector<int>&subscripts)
{
	if (subscripts.size() == vcSubscripts.size())
	{
		unsigned long offset = CalculateOffset(subscripts);
		return &(vcValues[offset]);
	}
	return NULL;
}
void Symbol::SetSize(vector<int>&subscripts)
{
	if (subscripts.empty())
	{
		vcSubscripts.clear();
		vcValues.clear();
		vcValues.resize(1);
	}
	else
	{
		vcSubscripts.clear();
		vcSubscripts = subscripts;
		//copy(subscripts.begin(),subscripts.end(),vcSubscripts.begin());
		unsigned long total_size = 1;
		for (vector<int>::iterator it=vcSubscripts.begin();it!=vcSubscripts.end();++it)
			total_size *= *it;
		vcValues.clear();
		vcValues.resize(total_size);
	}
}
bool Symbol::CreateSymbol(Token&objToken)
{
	//<array_decl> ::= "DEFARRAY" <type> ":" IDENT "[" INTLIT { "," INTLIT }"]" ";"
	//<variable_decl> ::= "DEFVAR" <type> ":" IDENT ";"
	if (objToken.iNum == Token::LEX_NONTERMINAL)
	{
		string strNT = objToken.dtValue.GetStrData();
		if (strNT == "<variable_decl>")
		{
			list<Token>::iterator it = objToken.lstChildren.begin(); //DEFVAR
			++it; //<type>
			data_type = it->lstChildren.front().iNum;
			++it; //:
			++it; //IDENT
			strName = it->dtValue.GetStrData();
			SetSize(vector<int>());
			return true;
		}
		if (strNT == "<array_decl>")
		{
			list<Token>::iterator it = objToken.lstChildren.begin(); //DEFARRAY
			++it; //<type>
			data_type = it->lstChildren.front().iNum;
			++it; //:
			++it; //IDENT
			strName = it->dtValue.GetStrData();
			++it; //[
			vector<int> subscripts;
			do
			{
				++it; //INTLIT
				subscripts.push_back(it->dtValue.GetIntData());
				++it; //, or ]
			} while (it->iNum == Token::LEX_COMMA);
			SetSize(subscripts);
			return true;
		}
	}
	return false;
}
////////////////////
bool SymbolTable::AddSymbol(Symbol obj)
{
	if (GetSymbol(obj.strName))
		return false;
	vcSymbols.push_back(obj);
	return true;
}
int SymbolTable::SetValue(string strName,vector<int>&vcSubscripts,DataValue obj)
{
	Symbol*sym = GetSymbol(strName);
	if (sym)
	{
		return (sym->SetValue(vcSubscripts,obj))?1:2;;
	}
	return 3;
}
Symbol*SymbolTable::GetSymbol(string name)
{
	vector<Symbol>::iterator it;
	for (it=vcSymbols.begin();it!=vcSymbols.end();++it)
	{
		if (it->strName == name)
			return &(*it);
	}
	return NULL;
}
DataValue SymbolTable::GetSymbolValue(string name,vector<int>&vcSubscripts)
{
	Symbol*sym = GetSymbol(name);
	if (sym)
	{
		return sym->GetValue(vcSubscripts);
	}
	return DataValue();
}
void SymbolTable::Clear()
{
	vcSymbols.clear();
}
