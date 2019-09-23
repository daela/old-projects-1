//-----------------------------------------------------------------------------
//    IOHandler.h
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#ifndef __IOHANDLER_H__JWM__
#define __IOHANDLER_H__JWM__
#pragma once
#include <fstream>
#include <string>
using namespace std;

class IOHandler
{
	protected:
		ifstream inFile;		// The input file stream
		streamoff soPosition;	// Storage for saving the file stream position
	public:
		// Open a file
		bool Open(string fname);
		// Close a file
		void Close();
		// Checks if the next read would result in EOF
		bool isEOF();
		// Tells which char will be read next without advancing the file pointer.
		char nextChar();
		// Gets the next char and advance the file pointer
		char getChar();
		// Checks if a given character is "whitespace" (space, tab, cr, lf)
		bool isWS(char ch);
		// Advances the file pointer to the first non-whitespace char it can find.
		void skipWS();
		// Store the file stream position so you can return to it later
		void StorePosition();
		// Restore the previously stored file stream position
		void ResetPosition();

		streampos GetPosition();
		void SetPosition(streampos);
		// functions that mimic <ctype.h> or <cctype>
		// created to avoid potential scope problems involved
		// if both headers end up included and you
		// have used "using namespace std;"
		static bool isNum(char ch)
		{ return (ch >= '0' && ch <= '9'); }
		static bool isAlpha(char ch)
		{ ch = toLower(ch); return (ch >= 'a' && ch <= 'z'); }
		static bool isEOL(char ch)
		{ return (ch == '\r' || ch == '\n'); }
		static char toLower(char ch)
		{ if (ch  >= 'A' && ch  <= 'Z') ch+='a'-'A'; return ch; }
		static char toUpper(char ch)
		{ if (ch  >= 'a' && ch  <= 'z') ch-='a'-'A'; return ch; }
		
};

#endif