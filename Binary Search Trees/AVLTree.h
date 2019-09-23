//-----------------------------------------------------------------------------
//				AVLTree.h
//
//	Author:		Jacob McIntosh
//	Section:	01
//	Date:		March 9, 2004
//
//	AVLTree.h contains the class for the AVL balanced binary tree class
//  which is derived from the general binary tree class, BSTree.
//
//-----------------------------------------------------------------------------

#ifndef __AVLTREE_H_MCINTOSH__
#define __AVLTREE_H_MCINTOSH__
#pragma once

#include "Node.h"
#include "BSTree.h"
#include <list>
using namespace std;

template<class T>
class AVLTree : public BSTree<T>
{
	protected:
		inline char calc_balance(Node<T>*ptr) const
		{ return (char)((ptr->left)?ptr->left->height:-1) - (char)((ptr->right)?ptr->right->height:-1); }

		void BalanceTree(bool = true);

		void RotateLeft(Node<T>*,Node<T>*);
		void RotateRightLeft(Node<T>*,Node<T>*);
		void RotateRight(Node<T>*,Node<T>*);
		void RotateLeftRight(Node<T>*,Node<T>*);

		void AdjustHeight(void);

	public:
		inline void Insert(const T & obj)
		{ BSTree<T>::Insert(obj); BalanceTree(); }

		inline void Delete(const T & obj)
		{ BSTree<T>::Delete(obj); BalanceTree(false); }

};

template<class T>
void AVLTree<T>::AdjustHeight(void)
{
	//completely overloading adjustheight, could have called the original one but
	//then i'd be stuck scanning the whole traversal list instead of knowing when
	//i can stop early.. now, i could take measures to fix that in the BSTree class
	//but then it would no longer be generic.
	unsigned long x;
	list<Node<T>*>::iterator it,end;

	for (it = traversals.begin(), end = traversals.end();it!=end;++it)
	{
		x = calc_height(*it);

		(*it)->balance = calc_balance(*it);
		if (x == (*it)->height) //whenever you get to a height that stays the same, you're done!
			break;
		(*it)->height = x;
	}
}

//observations were made based on the slides/java graphics to produce these functions
template<class T>
void AVLTree<T>::RotateLeft(Node<T>*node,Node<T>*parent)
{
	Node<T> *child = node->right;
	node->right = child->left; 
	child->left = node;

	if (parent)
		if (node->data < parent->data)
			parent->left = child;
		else
			parent->right = child;
	else
		root = child;

	node->height = calc_height(node);
	child->height = calc_height(child);

	node->balance = calc_balance(node);
	child->balance = calc_balance(child);
	if (parent)
		parent->balance = calc_balance(parent);
}

template<class T>
void AVLTree<T>::RotateRightLeft(Node<T>*node,Node<T>*parent)
{
	Node<T> *child = node->right;
	Node<T> *ch_lft = child->left;

	node->right = ch_lft->left;
	child->left = ch_lft->right;

	if (parent)
		if (node->data < parent->data)
			parent->left = ch_lft;
		else
			parent->right = ch_lft;
	else
		root = ch_lft;

	ch_lft->right = child;
	ch_lft->left = node;

	node->height = calc_height(node);
	child->height = calc_height(child);
	ch_lft->height = calc_height(ch_lft);

	node->balance = calc_balance(node);
	child->balance = calc_balance(child);
	ch_lft->balance = calc_balance(ch_lft);
	if (parent)
		parent->balance = calc_balance(parent);

}

template<class T>
void AVLTree<T>::RotateRight(Node<T>*node,Node<T>*parent)
{
	Node<T> *child = node->left;

	node->left = child->right; 
	child->right = node;

	if (parent)
		if (node->data < parent->data)
			parent->left = child;
		else
			parent->right = child;
	else
		root = child;

	node->height = calc_height(node);
	child->height = calc_height(child);

	node->balance = calc_balance(node);
	child->balance = calc_balance(child);
	if (parent)
		parent->balance = calc_balance(parent);
}

template<class T>
void AVLTree<T>::RotateLeftRight(Node<T>*node,Node<T>*parent)
{
	Node<T> *child = node->left;
	Node<T> *ch_rght = child->right;

	node->left = ch_rght->right;
	child->right = ch_rght->left;

	if (parent)
		if (node->data < parent->data)
			parent->left = ch_rght;
		else
			parent->right = ch_rght;
	else
		root = ch_rght;

	ch_rght->left = child;
	ch_rght->right = node;

	node->height = calc_height(node);
	child->height = calc_height(child);
	ch_rght->height = calc_height(ch_rght);

	node->balance = calc_balance(node);
	child->balance = calc_balance(child);
	ch_rght->balance = calc_balance(ch_rght);
	if (parent)
		parent->balance = calc_balance(parent);
}



//using the traversal list
template<class T>
void AVLTree<T>::BalanceTree(bool insertion) //for insertion only one imbalance can occur so lets optimize this
{
	Node<T> *ptr;
	bool imbalanced;
	
	//proceed up the traversal
	while (traversals.size())
	{
		imbalanced = false;
		ptr = *(traversals.begin());
		traversals.pop_front();
		
		//cehck the balances and perform the appropriate rotations
		if (ptr->balance == -2)
		{
			imbalanced = true;
			if (ptr->right->right) //wont ever be an imbalance if there's two children
				//simple left
				RotateLeft(ptr,(ptr==root)?NULL:*(traversals.begin()));
			else
				//right left
				RotateRightLeft(ptr,(ptr==root)?NULL:*(traversals.begin()));
		}
		else if (ptr->balance == 2)
		{
			imbalanced = true;
			if (ptr->left->left)
				//simple right
				RotateRight(ptr,(ptr==root)?NULL:*(traversals.begin()));
			else
				//left right
				RotateLeftRight(ptr,(ptr==root)?NULL:*(traversals.begin()));
		}

		if (imbalanced)
		{
			//adjust the heights/balances
			AdjustHeight();
			if (insertion)
				break; //only one imbalance will occur if it's insertion.
		}
	}
	
}



#endif