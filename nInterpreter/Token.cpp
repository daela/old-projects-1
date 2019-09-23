//-----------------------------------------------------------------------------
//    Token.cpp
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#include "Token.h"

//	Constructor so I can construct objects on the fly.
//  Also allows default construction so STL containers are happy.
Token::Token(TokenType num,DataValue dv) : iNum(num), dtValue(dv)
{

}

void Token::AddChild(Token&obj)
{
	lstChildren.push_back(obj);
}

string Token::TokenIdToString(TokenType tt)
{
	switch (tt)
	{
	//LEX_LPAREN, LEX_RPAREN, LEX_LBRACK, LEX_RBRACK, LEX_COMMA,
		case LEX_LPAREN: return "LPAREN";
		case LEX_RPAREN: return "RPAREN";
		case LEX_LBRACK: return "LBRACK";
		case LEX_RBRACK: return "RBRACK";
		case LEX_COMMA: return "COMMA";
	//LEX_COLON, LEX_SEMI, LEX_QUOTE, LEX_DQUOTE, LEX_PERIOD, LEX_POS,
		case LEX_COLON: return "COLON";
		case LEX_SEMI: return "SEMI";
		case LEX_QUOTE: return "QUOTE";
		case LEX_DQUOTE: return "DQUOTE";
		case LEX_PERIOD: return "PERIOD";
		case LEX_POS: return "POS";
	//LEX_NEG, LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_DIV, LEX_MOD, LEX_EQ,
		case LEX_NEG: return "NEG";
		case LEX_PLUS: return "PLUS";
		case LEX_MINUS: return "MINUS";
		case LEX_TIMES: return "TIMES";
		case LEX_DIV: return "DIV";
		case LEX_MOD: return "MOD";
		case LEX_EQ: return "EQ";
	//LEX_NE, LEX_GT, LEX_LT, LEX_GE, LEX_LE, LEX_AND, LEX_OR, LEX_NOT,
		case LEX_NE: return "NE";
		case LEX_GT: return "GT";
		case LEX_LT: return "LT";
		case LEX_GE: return "GE";
		case LEX_LE: return "LE";
		case LEX_AND: return "AND";
		case LEX_OR: return "OR";
		case LEX_NOT: return "NOT";
	//LEX_ALGO, LEX_ASSN, LEX_BEGIN, LEX_BOOL, LEX_CHAR, LEX_DEFAR,
		case LEX_ALGO: return "ALGO";
		case LEX_ASSN: return "ASSN";
		case LEX_BEGIN: return "BEGIN";
		case LEX_BOOL: return "BOOL";
		case LEX_CHAR: return "CHAR";
		case LEX_DEFAR: return "DEFAR";
	//LEX_DEFUN, LEX_DEFVAR, LEX_DO, LEX_ELSE, LEX_END, LEX_ENDFUN,
		case LEX_DEFUN: return "DEFUN";
		case LEX_DEFVAR: return "DEFVAR";
		case LEX_DO: return "DO";
		case LEX_ELSE: return "ELSE";
		case LEX_END: return "END";
		case LEX_ENDFUN: return "ENDFUN";
	//LEX_ENDIF, LEX_ENDALG, LEX_ENDDO, LEX_FALSE, LEX_IF, LEX_INT,
		case LEX_ENDIF: return "ENDIF";
		case LEX_ENDALG: return "ENDALG";
		case LEX_ENDDO: return "ENDDO";
		case LEX_FALSE: return "FALSE";
		case LEX_IF: return "IF";
		case LEX_INT: return "INT";
	//LEX_READ, LEX_REAL, LEX_RETURN, LEX_THEN, LEX_TO, LEX_TRUE,
		case LEX_READ: return "READ";
		case LEX_REAL: return "REAL";
		case LEX_RETURN: return "RETURN";
		case LEX_THEN: return "THEN";
		case LEX_TO: return "TO";
		case LEX_TRUE: return "TRUE";
	//LEX_VOID, LEX_WHILE, LEX_WRITE, LEX_IDENT, LEX_CHRLIT,
		case LEX_VOID: return "VOID";
		case LEX_WHILE: return "WHILE";
		case LEX_WRITE: return "WRITE";
		case LEX_IDENT: return "IDENT";
		case LEX_CHRLIT: return "CHRLIT";
	//LEX_STRLIT, LEX_INTLIT, LEX_FLOLIT
		case LEX_STRLIT: return "STRLIT";
		case LEX_INTLIT: return "INTLIT";
		case LEX_FLOLIT: return "FLOLIT";
	}
	return "ERROR";
}