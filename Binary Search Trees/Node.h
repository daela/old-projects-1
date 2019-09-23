//-----------------------------------------------------------------------------
//				Node.h
//
//	Author:		Jacob McIntosh
//	Section:	01
//	Date:		March 9, 2004
//
//	Node.h defines the node class that BSTree uses.
//
//-----------------------------------------------------------------------------

#ifndef __NODE_H_MCINTOSH__
#define __NODE_H_MCINTOSH__
#pragma once

#include <iostream>
//this at some level defines NULL, i would define it myself but
//but theoretically the value could change at some future revision.

//though the BSTree doesnt use a balance factor, having it in the
//node class doesnt hurt a thing, and makes the creation of the
//avl tree much simpler as a derived class.
template<class T>
class Node
{
	public:
		Node(const T & obj)
		{ data = obj; count = 1; height = 0; left = right = NULL; balance = 0; }

		Node()
		{ count = 1; height = 0; left = right = NULL; }

		~Node()
		{ delete left; delete right; } //will delete ALL children, if you think about it

		T data;
		unsigned long count;
		unsigned long height;
		char balance;

		Node<T>*left;
		Node<T>*right;
};

#endif