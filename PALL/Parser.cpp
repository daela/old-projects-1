//-----------------------------------------------------------------------------
//    Parser.cpp
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#include "Parser.h"

Parser::Parser(Lexer*lex)
{
	ptLex = lex;
}


void Parser::PrintTree(ostream&ofs)
{
	spaces = -1;
	TreeWalk(lstParseTree,ofs);

}
void Parser::TreeWalk(list<Token>&lstTree,ostream&ofs)
{
	list<Token>::iterator it;
	++spaces;
	string strSpaces;
	strSpaces.resize(spaces*2,' ');
	for (it=lstTree.begin();it!=lstTree.end();++it)
	{
		switch (it->iNum)
		{
			case Token::LEX_NONTERMINAL:
				ofs << strSpaces << it->dtValue << endl;
				break;
			default:
				ofs << strSpaces << Token::TokenIdToString(it->iNum) << endl;
				break;
		}
		TreeWalk(it->lstChildren,ofs);
	}
	--spaces;
}
bool Parser::Validate()
{
	lstParseTree.clear();
	return _program(lstParseTree,true);
}
// Non-terminal functions
bool Parser::_program(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<program>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();

	try
	{
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_ALGO,lstChildren);

		_declarations(lstChildren);

		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_BEGIN,lstChildren);

		_stmt_sequence(lstChildren);

		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_ENDALG,lstChildren);
		lstTree.push_back(objParse);
		return true; //
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }

	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false; //
}
bool Parser::_declarations(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<declarations>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	//streampos lexPos = ptLex->GetPosition();

	bool anything = false;
	while (_function_decl(lstChildren)) anything=true;
	while (_array_decl(lstChildren) || _variable_decl(lstChildren)) anything=true;

	if (!anything)
	{
		if (req) lstTree.push_back(objParse);
		return false;
	}
	lstTree.push_back(objParse);
	return true; //matches empty too
}

bool Parser::_function_decl(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<function_decl>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();

	try
	{
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_DEFUN,lstChildren);
		if (!_type(lstChildren,req)) throw 0;
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_COLON,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_IDENT,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_LPAREN,lstChildren);

		if (_type(lstChildren))
		{
			ptLex->GetToken(objToken);
			checkIdent(objToken,Token::LEX_COLON,lstChildren);
			ptLex->GetToken(objToken);
			checkIdent(objToken,Token::LEX_IDENT,lstChildren);
			//{ "," <type> ":" IDENT }
			ptLex->GetToken(objToken);
			while (isIdent(objToken,Token::LEX_COMMA,lstChildren))
			{
				if (!_type(lstChildren,req)) throw 0;
				ptLex->GetToken(objToken);
				checkIdent(objToken,Token::LEX_COLON,lstChildren);
				ptLex->GetToken(objToken);
				checkIdent(objToken,Token::LEX_IDENT,lstChildren);
				ptLex->GetToken(objToken);
			}
		}
		else
		{	// expecting a token to be current, so read here.
			ptLex->GetToken(objToken);
		}
		checkIdent(objToken,Token::LEX_RPAREN,lstChildren);
		while (_variable_decl(lstChildren));
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_BEGIN,lstChildren);
		while (_stmt_sequence(lstChildren));
		_return_stmt(lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_ENDFUN,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_SEMI,lstChildren);

		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_array_decl(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<array_decl>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();

	try
	{
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_DEFAR,lstChildren);
		if (!_type(lstChildren,req)) throw 0;
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_COLON,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_IDENT,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_LBRACK,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_INTLIT,lstChildren);
		ptLex->GetToken(objToken);
		while (isIdent(objToken,Token::LEX_COMMA,lstChildren))
		{
			ptLex->GetToken(objToken);
			checkIdent(objToken,Token::LEX_INTLIT,lstChildren);
			ptLex->GetToken(objToken);
		}
		checkIdent(objToken,Token::LEX_RBRACK,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_SEMI,lstChildren);
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_variable_decl(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<variable_decl>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();

	try
	{
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_DEFVAR,lstChildren);
		if (!_type(lstChildren,req)) throw 0;
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_COLON,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_IDENT,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_SEMI,lstChildren);
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_type(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<type>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();
	try
	{
		ptLex->GetToken(objToken);
		if (isIdent(objToken,Token::LEX_INT,lstChildren) ||
			isIdent(objToken,Token::LEX_REAL,lstChildren) ||
			isIdent(objToken,Token::LEX_CHAR,lstChildren) ||
			isIdent(objToken,Token::LEX_BOOL,lstChildren) ||
			isIdent(objToken,Token::LEX_VOID,lstChildren) )
		{
			lstTree.push_back(objParse);
			return true;
		}
		// Should I make it give some sort of meaningful error? throw?
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_stmt_sequence(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<stmt_sequence>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	//streampos lexPos = ptLex->GetPosition();

	bool anything = false;
	while (_assignment_stmt(lstChildren) || _input_stmt(lstChildren) ||
		   _output_stmt(lstChildren) || _if_stmt(lstChildren) || _while_stmt(lstChildren) )
	{
		anything = true;
	}

	if (!anything)
	{
		if (req) lstTree.push_back(objParse);
		return false;
	}
	lstTree.push_back(objParse);
	return true; //matches empty too
}
bool Parser::_assignment_stmt(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<assignment_stmt>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();

	try
	{
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_ASSN,lstChildren);
		if (!_expression(lstChildren,req)) throw 0;
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_TO,lstChildren);
		if (!_designator(lstChildren,req)) throw 0;
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_SEMI,lstChildren);

		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_input_stmt(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<input_stmt>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();

	try
	{
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_READ,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_LPAREN,lstChildren);
		if (!_designator(lstChildren,req)) throw 0;
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_RPAREN,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_SEMI,lstChildren);
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_output_stmt(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<output_stmt>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();

	try
	{
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_WRITE,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_LPAREN,lstChildren);
		if (_expression(lstChildren))
		{
			ptLex->GetToken(objToken);
			while (isIdent(objToken,Token::LEX_COMMA,lstChildren))
			{
				if (!_expression(lstChildren,req)) throw 0;
				ptLex->GetToken(objToken);
			}
		}
		else
		{
			ptLex->GetToken(objToken);
		}
		checkIdent(objToken,Token::LEX_RPAREN,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_SEMI,lstChildren);
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_if_stmt(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<if_stmt>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();

	try
	{
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_IF,lstChildren);
		if (!_expression(lstChildren,req)) throw 0;
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_THEN,lstChildren);
		if (!_stmt_sequence(lstChildren,req)) throw 0;
		ptLex->GetToken(objToken);
		if  (isIdent(objToken,Token::LEX_ELSE,lstChildren))
		{
			if (!_stmt_sequence(lstChildren,req)) throw 0;
			ptLex->GetToken(objToken);
		}
		checkIdent(objToken,Token::LEX_ENDIF,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_SEMI,lstChildren);
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_while_stmt(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<while_stmt>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();

	try
	{
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_WHILE,lstChildren);
		if (!_expression(lstChildren,req)) throw 0;
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_DO,lstChildren);
		if (!_stmt_sequence(lstChildren,req)) throw 0;
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_ENDDO,lstChildren);
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_SEMI,lstChildren);
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_return_stmt(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<return_stmt>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();

	try
	{
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_RETURN,lstChildren);
		if (!_expression(lstChildren,req)) throw 0;
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_SEMI,lstChildren);
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_designator(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<designator>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();
	streampos lexPos2;

	try
	{
		ptLex->GetToken(objToken);
		checkIdent(objToken,Token::LEX_IDENT,lstChildren);
		lexPos2 = ptLex->GetPosition(); //store state in case optional part isnt there
		
		ptLex->GetToken(objToken);
		if (isIdent(objToken,Token::LEX_LBRACK,lstChildren))
		{
			if (!_expression(lstChildren,req)) throw 0;
			ptLex->GetToken(objToken);
			while (isIdent(objToken,Token::LEX_COMMA,lstChildren))
			{
				if (!_expression(lstChildren,req)) throw 0;
				ptLex->GetToken(objToken);
			}
			checkIdent(objToken,Token::LEX_RBRACK,lstChildren);
		}
		else
		{
			ptLex->SetPosition(lexPos2);
		}
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_expression(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<expression>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();
	streampos lexPos2;

	try
	{
		if (!_simple_expression(lstChildren,req)) throw 0;

		lexPos2 = ptLex->GetPosition(); //store state in case optional part isnt there
		ptLex->GetToken(objToken);
		if (isIdent(objToken,Token::LEX_EQ,lstChildren) ||
			isIdent(objToken,Token::LEX_NE,lstChildren) ||
			isIdent(objToken,Token::LEX_LT,lstChildren) ||
			isIdent(objToken,Token::LEX_LE,lstChildren) ||
			isIdent(objToken,Token::LEX_GT,lstChildren) ||
			isIdent(objToken,Token::LEX_GE,lstChildren))
		{
			if (!_simple_expression(lstChildren,req)) throw 0;
		}
		else
		{
			ptLex->SetPosition(lexPos2);
		}
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_simple_expression(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<simple_expression>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();
	streampos lexPos2;

	try
	{
		ptLex->GetToken(objToken);
		if (isIdent(objToken,Token::LEX_POS,lstChildren) ||
			isIdent(objToken,Token::LEX_NEG,lstChildren))
		{
			//nothing special
		}
		else
		{
			ptLex->SetPosition(lexPos);
		}
		
		do
		{
			if (!_term(lstChildren,req)) throw 0;
			lexPos2 = ptLex->GetPosition();
			ptLex->GetToken(objToken);
		} while (isIdent(objToken,Token::LEX_PLUS,lstChildren) ||
			   isIdent(objToken,Token::LEX_MINUS,lstChildren) ||
			   isIdent(objToken,Token::LEX_OR,lstChildren));
		ptLex->SetPosition(lexPos2);
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_term(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<term>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();
	streampos lexPos2;

	try
	{
		do
		{
			if (!_factor(lstChildren,req)) throw 0;
			lexPos2 = ptLex->GetPosition();
			ptLex->GetToken(objToken);
		} while (isIdent(objToken,Token::LEX_TIMES,lstChildren) ||
			   isIdent(objToken,Token::LEX_DIV,lstChildren) ||
			   isIdent(objToken,Token::LEX_MOD,lstChildren) ||
			   isIdent(objToken,Token::LEX_AND,lstChildren));
		ptLex->SetPosition(lexPos2);
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
bool Parser::_factor(list<Token>&lstTree,bool req)
{
	DataValue dv;
	dv.SetStrData("<factor>");
	Token objParse = Token(Token::LEX_NONTERMINAL,dv);
	Token objToken;
	list<Token>&lstChildren = objParse.lstChildren;
	streampos lexPos = ptLex->GetPosition();

	try
	{
		ptLex->GetToken(objToken);
		if (isIdent(objToken,Token::LEX_LPAREN,lstChildren))
		{
			if (!_expression(lstChildren,req)) throw 0;
			ptLex->GetToken(objToken);
			checkIdent(objToken,Token::LEX_RPAREN,lstChildren);
		}
		else if (isIdent(objToken,Token::LEX_INTLIT,lstChildren) ||
				 isIdent(objToken,Token::LEX_FLOLIT,lstChildren) ||
				 isIdent(objToken,Token::LEX_CHRLIT,lstChildren) ||
				 isIdent(objToken,Token::LEX_STRLIT,lstChildren) ||
				 isIdent(objToken,Token::LEX_TRUE,lstChildren) ||
				 isIdent(objToken,Token::LEX_FALSE,lstChildren))
		{
			//fine
		}
		else if (isIdent(objToken,Token::LEX_NOT,lstChildren))
		{
			if (!_factor(lstChildren,req)) throw 0;
		}
		else
		{
			ptLex->SetPosition(lexPos);
			if (!_designator(lstChildren,req)) throw 0;
		}
		lstTree.push_back(objParse);
		return true;
	}
	catch (ParseError perr)
	{ if (req) outputError(dv.GetStrData(),perr); }
	catch (int) { }
	
	ptLex->SetPosition(lexPos);
	if (req) lstTree.push_back(objParse);
	return false;
}
void Parser::checkIdent(Token&objToken,Token::TokenType tt,list<Token>&lstChildren)
{
	if (objToken.iNum != tt) throw ParseError(objToken,tt);
	lstChildren.push_back(objToken);
}

void Parser::outputError(string strFunction, ParseError perr)
{
	cout  << strFunction << ": " << perr.toString() << endl;
}

bool Parser::isIdent(Token&objToken,Token::TokenType tt,list<Token>&lstChildren)
{
	if (objToken.iNum == tt)
	{
		lstChildren.push_back(objToken);
		return true;
	}
	return false;
}