//-----------------------------------------------------------------------------
//    Executer.cpp
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#include "Executer.h"

void Executer::Execute(list<Token>&lstTree)
{
	isBody=false;
	symTable.Clear();
	TableWalk(lstTree);
}
DataValue Executer::TableWalk(list<Token>&lstTree)
{
	list<Token>::iterator it;
	for (it=lstTree.begin();it!=lstTree.end();++it)
	{
		if (it->iNum == Token::LEX_BEGIN)
		{
			isBody = true;
		}
		else if (it->iNum == Token::LEX_NONTERMINAL)
		{
			Symbol obj;
			if (obj.CreateSymbol(*it))
			{
				if (!symTable.AddSymbol(obj))
				{
					cout << "Error: Symbol '" << obj.strName << "' is already defined." << endl;
				}
				continue;
			}
			string strNT = it->dtValue.GetStrData();
			if (strNT == "<assignment_stmt>")
			{
				//assign <expression> to <designator>
				//string varname = it->dtValue.GetStrData();
				list<Token>::iterator tit = it->lstChildren.begin(); //assign
				++tit; //expression
				DataValue dtValue = ProcessExpression(*tit);
				++tit; //to
				++tit; //designator
				string varname = tit->lstChildren.front().dtValue.GetStrData();
				vector<int> subscripts;
				DataValue*dvptr = ProcessDesignator(*tit,subscripts);
				
				if (!dvptr)
				{
					cout << "ERROR: Variable \'" << varname << "\' not defined before assignment or invalid subscripts provided." << endl;
				}
				else if (!symTable.SetValue(varname,subscripts,dtValue))
				{
					cout << "ERROR: Variable \'" << varname << "\' assigned incompatible type." << endl;
				}
				continue;
			}
			else if (strNT == "<if_stmt>")
			{
				//IF <expression> THEN <stmt_sequence>
				list<Token>::iterator tit = it->lstChildren.begin();
				++tit;
				DataValue dv = ProcessExpression(*tit);
				if (dv.IsTrue())
				{
					//true
					++tit;
					++tit;
					TableWalk(tit->lstChildren);
				}
				else
				{
					++tit;
					++tit;
					++tit;
					if (tit->iNum == Token::LEX_ELSE)
					{
						++tit;
						TableWalk(tit->lstChildren);
					}
				}
				continue;
			}
			else if (strNT == "<while_stmt>")
			{
				//WHILE <expression> DO <stmt_sequence>
				list<Token>::iterator tit = it->lstChildren.begin();
				++tit; //points to <expression>
				list<Token>::iterator tit2 = tit;
				++tit2;
				++tit2; //points to <stmt_sequence>
				while (ProcessExpression(*tit).IsTrue())
				{
					TableWalk(tit2->lstChildren);
				}
				continue;
			}
			else if (strNT == "<output_stmt>")
			{
				list<Token>::iterator tit = it->lstChildren.begin();
				++tit;
				++tit;
				while (tit->iNum == Token::LEX_NONTERMINAL && tit->dtValue.GetStrData() == "<expression>")
				{
					cout << ProcessExpression(*tit);
					++tit;
					++tit;
				}
				cout << endl;
				continue;
			}
			else if (strNT == "<input_stmt>")
			{
				//<input_stmt> ::= "READ" "(" <designator> ")" ";"
				list<Token>::iterator tit = it->lstChildren.begin();
				++tit;
				++tit;
				//<designator> ::= IDENT [...]

				vector<int> subscripts;
				string varname = tit->lstChildren.front().dtValue.GetStrData();
				Symbol*sym=symTable.GetSymbol(varname);
				DataValue*dvptr=ProcessDesignator(*tit,subscripts);
				if (!dvptr || !sym)
				{
					cout << "Error: variable \"" << varname << "\" not defined prior to use in READ or invalid subscripts provided.";
				}
				else
				{
					//somehow read this in.
					string strRead;
					cin >> strRead;
					cin.ignore(1000, '\n');
					bool err = false;
						
					switch (sym->data_type)
					{
						case Token::LEX_INT:
							if (strRead.find_first_not_of("0123456789",0,10) != string::npos)
								err = true;
							else
								dvptr->SetIntData(atoi(strRead.c_str()));
							break;
						case Token::LEX_REAL:
							if (strRead.find_first_not_of("0123456789.",0,11) != string::npos)
								err = true;
							else
							{
								if (strRead[0] == '.') { err = true; break; }
								string::size_type off;
								off = strRead.find('.',0);
								if  (off != string::npos)
								{
									if (strRead.find('.',off+1) != string::npos)
									{
										err = true;
										break;
									}
								}
								dvptr->SetFloatData(atof(strRead.c_str()));
							}
							break;
						case Token::LEX_CHAR:
							if (strRead.size() != 1)
							{
								err = true;
								break;
							}
							dvptr->SetCharData(strRead[0]);
							break;
					};
					if (err)
					{
						cout << "Error: invalid input format for variable \"" << varname << "\" of type " << Token::TokenIdToString(sym->data_type) <<  "." << endl;
					}

				}
				
				continue;
			}
		}
		TableWalk(it->lstChildren);
	}
	return DataValue();
}
DataValue Executer::ProcessFactor(Token&obj)
{
	DataValue dv;
	int numChildren = obj.lstChildren.size();
	list<Token>::iterator it = obj.lstChildren.begin();
	if (numChildren == 1)
	{
		
		char buf[20];
		dv = it->dtValue;
		switch (it->iNum)
		{
			case Token::LEX_INTLIT:
				sprintf(buf,"(%d)",dv.GetIntData());
				dv.strExtra = buf;
				return dv;
			case Token::LEX_STRLIT:
				dv.strExtra = string("(")+dv.GetStrData() + ")";
				return dv;
			case Token::LEX_CHRLIT:
				dv.strExtra = string("(")+dv.GetCharData() + ")";
				return dv;
			case Token::LEX_FLOLIT:
				sprintf(buf,"(%f)",dv.GetFloatData());
				dv.strExtra = buf;
				return dv;
			case Token::LEX_TRUE:
				dv.SetBoolData(true);
				dv.strExtra = "(true)";
				return dv;
			case Token::LEX_FALSE:
				dv.SetBoolData(false);
				dv.strExtra = "(false)";
				return dv;
		}
		vector<int> subscripts;
		DataValue*dvptr = ProcessDesignator(*it,subscripts);
		dv = (dvptr?*dvptr:DataValue());
		dv.strExtra = string("(") + dv.strExtra + ")";
		return dv;
	}
	else if (numChildren == 2)
	{
		//!<factor>
		dv.strExtra += "!";
		++it;
		DataValue dr = ProcessFactor(*it);
		dv.strExtra += dr.strExtra;
		dv.strExtra = string("(") + dv.strExtra + ")";
		dr = !dr;
		dv.SetData(dr.GetType(),dr.GetData());
		return dv;
	}
	else if (numChildren == 3)
	{
		dv.strExtra += "(";
		++it;
		DataValue dr = ProcessExpression(*it);
		dv.strExtra += dr.strExtra + ")";
		dv.strExtra = string("(") + dv.strExtra + ")";
		dv.SetData(dr.GetType(),dr.GetData());
		return dv;
	}
	return dv;

}

DataValue*Executer::ProcessDesignator(Token&obj,vector<int>&subscripts)
{
	//<designator> ::= IDENT [ "[" <expression> { "," <expression> } "]" ]
	list<Token>::iterator it = obj.lstChildren.begin();
	//find the entry in the symbol table
	string varname = it->dtValue.GetStrData();
	Symbol*sym = symTable.GetSymbol(varname);
	if (!sym) return NULL;
	++it;
	subscripts.clear();
	if (it != obj.lstChildren.end() && it->iNum == Token::LEX_LBRACK)
	{
		do
		{
			++it; //expression
			DataValue tmp = ProcessExpression(*it);
			if (tmp.GetType() != DataValue::DT_INT)
				return NULL;
			subscripts.push_back(tmp.GetIntData()-1);
			++it;
		} while (it->iNum == Token::LEX_COMMA);
	}
	return sym->GetValuePtr(subscripts);
}
DataValue Executer::ProcessExpression(Token&obj)
{
	DataValue dv;
	list<Token>::iterator it = obj.lstChildren.begin(), it2;
	dv = ProcessSimpleExpression(*it);
	if (obj.lstChildren.size() == 3)
	{
		++it; //operator
		it2 = it;
		++it2; //operand
		DataValue lhs,rhs;
		lhs = dv;
		rhs = ProcessSimpleExpression(*it2);
		switch (it->iNum)
		{
			case Token::LEX_EQ:
				lhs = (lhs == rhs);
				dv.strExtra += string(" = ");
				break;
			case Token::LEX_NE:
				lhs = (lhs != rhs);
				dv.strExtra += string(" != ");
				break;
			case Token::LEX_LT:
				lhs = (lhs < rhs);
				dv.strExtra += string(" < ");
				break;
			case Token::LEX_LE:
				lhs = (lhs <= rhs);
				dv.strExtra += string(" <= ");
				break;
			case Token::LEX_GT:
				lhs = (lhs > rhs);
				dv.strExtra += string(" > ");
				break;
			case Token::LEX_GE:
				lhs = (lhs >= rhs);
				dv.strExtra += string(" >= ");
				break;
			default:
				lhs.Clear();
		}
		dv.SetData(lhs.GetType(),lhs.GetData());
		dv.strExtra += rhs.strExtra;
	}
	else if (obj.lstChildren.size() != 1)
		return DataValue();
	dv.strExtra = string("(") + dv.strExtra + ")";
	return dv;
}
DataValue Executer::ProcessSimpleExpression(Token&obj)
{
	DataValue dv,lhs,rhs;
	list<Token>::iterator it = obj.lstChildren.begin(), it2;
	int posneg = 0;
	if (it->iNum == Token::LEX_POS)
		posneg = 1;
	if (it->iNum == Token::LEX_NEG)
		posneg = 2;
	if (posneg)
		++it;
	dv = ProcessTerm(*it);
	int numOperations = obj.lstChildren.size();
	if (posneg)
	{
		if (posneg == 1)
		{
			lhs = +dv;
			dv.strExtra = string("#") + dv.strExtra;
		}
		if (posneg == 2)
		{
			lhs = -dv;
			dv.strExtra = string("~") + dv.strExtra;
		}
		dv.SetData(lhs.GetType(),lhs.GetData());
		
		numOperations = (numOperations-2)/2;
	}
	else
	{
		numOperations = (numOperations-1)/2;
	}
	//
	//
	while (numOperations)
	{
		++it; //operator
		it2 = it;
		++it2; //operand
		lhs = dv;
		rhs = ProcessTerm(*it2);
		switch (it->iNum)
		{
			case Token::LEX_PLUS:
				lhs = lhs + rhs;
				dv.strExtra += string(" + ");
				break;
			case Token::LEX_MINUS:
				lhs = lhs - rhs;
				dv.strExtra += string(" - ");
				break;
			case Token::LEX_OR:
				lhs = lhs || rhs;
				dv.strExtra += string(" | ");
				break;
			default:
				lhs.Clear();
		}
		dv.SetData(lhs.GetType(),lhs.GetData());
		dv.strExtra += rhs.strExtra;
		--numOperations;
		++it;
	}
	dv.strExtra = string("(") + dv.strExtra + ")";
	return dv;
}
DataValue Executer::ProcessTerm(Token&obj)
{
	DataValue dv;
	list<Token>::iterator it = obj.lstChildren.begin(), it2;
	int numOperations = (obj.lstChildren.size()-1)/2;
	dv = ProcessFactor(*it);
	while (numOperations)
	{
		++it; //operator
		it2 = it;
		++it2; //operand
		DataValue lhs = dv;
		DataValue rhs = ProcessFactor(*it2);
		switch (it->iNum)
		{
			case Token::LEX_TIMES:
				lhs = lhs * rhs;
				dv.strExtra += string(" * ");
				break;
			case Token::LEX_DIV:
				lhs = lhs / rhs;
				dv.strExtra += string(" / ");
				break;
			case Token::LEX_MOD:
				lhs = lhs % rhs;
				dv.strExtra += string(" % ");
				break;
			case Token::LEX_AND:
				lhs = lhs && rhs;
				dv.strExtra += string(" & ");
				break;
			default:
				lhs.Clear();
		}
		dv.SetData(lhs.GetType(),lhs.GetData());
		dv.strExtra += rhs.strExtra;
		--numOperations;
		++it;
	}
	dv.strExtra = string("(") + dv.strExtra + ")";
	return dv;
}
