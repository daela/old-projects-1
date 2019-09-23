//-----------------------------------------------------------------------------
//    IOHandler.cpp
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#include "IOHandler.h"

// Check if the next read will result in end of file
bool IOHandler::isEOF()
{
	if (inFile.peek() == char_traits<char>::eof())
	{
		// Oddly enough, the end of file flag is set even if you peek
		// so we'll clear it.
		inFile.clear();
		return true;
	}
	return false;
}
// Retrieve the character that the next read would retrieve.
char IOHandler::nextChar()
{
	return inFile.peek();
}
// Retrieve a character and advance the file pointer.
char IOHandler::getChar()
{
	return inFile.get();
}
// Check if a given character is "whitespace"
bool IOHandler::isWS(char ch)
{
	switch (ch)
	{
		case '\0':
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			return true;
		default:
			return false;
	}
	return false;
}
// Advance the file to the first non-whitespace character found.
void IOHandler::skipWS()
{
	while  (!isEOF() && isWS(nextChar()))
		getChar();
}
// Store the file stream position so you can return to it later
void IOHandler::StorePosition()
{
	soPosition = inFile.tellg();
}
// Restore the previously stored file stream position
void IOHandler::ResetPosition()
{
	inFile.seekg(soPosition);
	inFile.clear();
}

streampos IOHandler::GetPosition()
{
	return inFile.tellg();
}
void IOHandler::SetPosition(streampos sp)
{
	inFile.seekg(sp);
	inFile.clear();
}
// Open a file
bool IOHandler::Open(string fname)
{
	inFile.close();
	inFile.clear();
	inFile.open(fname.c_str(),ios::binary);
	if (inFile)
	{
		soPosition = inFile.tellg();
		return true;
	}
	inFile.close();
	return false;
}
// Close a file
void IOHandler::Close()
{
	inFile.clear();
	inFile.close();
}