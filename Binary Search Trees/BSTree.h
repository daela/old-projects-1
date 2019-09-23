//-----------------------------------------------------------------------------
//				BSTree.h
//
//	Author:		Jacob McIntosh
//	Section:	01
//	Date:		March 9, 2004
//
//	BSTree.h contains the class for a standard binary search tree.
//
//-----------------------------------------------------------------------------

#ifndef __BSTREE_H_MCINTOSH__
#define __BSTREE_H_MCINTOSH__
#pragma once

#include "Node.h"
#include <list>
using namespace std;


//The insert/delete functions of this class create a list of parents that are traversed
//during the operations in the variable "traversals".  These traversals are used for
//extending the class as i did in the AVLTree class.
template<class T>
class BSTree
{
	protected:

		//npos, reserved for errors
		static const unsigned long npos;


		Node<T> *root;
		//traversal list
		list< Node<T>* > traversals;
		//size of list
		unsigned long size;

		//calculate the height of a node based on it's children, inline suggested
		inline unsigned long calc_height(Node<T>*ptr) const
		{
			unsigned long l = (ptr->left?ptr->left->height+1:0);
			unsigned long r = (ptr->right?ptr->right->height+1:0);
			return (_MAX(l,r));
		}
		

		//deletion of a node without two children, true/false determines if this was applicable
		bool SimpleDelete(Node<T>*,Node<T>*);

		//adjusts the height of all the nodes in traversals
		virtual void AdjustHeight(void);

		//takes a function pointer to a function accepting the data,count
		void InOrder_Traverse(Node<T>*,void (*)(const T&,const unsigned long)) const; //function pointer

	public:
		BSTree() { root = NULL; size = 0; }
		~BSTree() { delete root; } //node class will delete all children. see ~Node

		//refer to inorder_traverse
		void Output(void (*)(const T&,const unsigned long)) const; //function pointer


		void Delete(const T &);
		void Insert(const T &);

		//returns npos if it wasn't found, otherwise returns the count
		unsigned long Find(const T &);

		inline unsigned long Size(void) const
		{ return size; }

		//returns npos if the tree is empty
		inline unsigned long Height(void) const
		{ return (root?root->height:npos); }
};

template<class T>
const unsigned long BSTree<T>::npos = -1;

template<class T>
unsigned long BSTree<T>::Find(const T & obj)
{
	Node<T>*ptr = root;
	while (ptr)
	{
		if (obj == ptr->data)
			return ptr->count;
		else if (obj > ptr->data)
			ptr = ptr->right;
		else
			ptr = ptr->left;
	}
	return npos;
}

template<class T>
void BSTree<T>::InOrder_Traverse(Node<T>*node,void (*Visit)(const T&,const unsigned long)) const
{
	if (node->left)
		InOrder_Traverse(node->left,Visit);

	(*Visit)(node->data,node->count);

	if (node->right)
		InOrder_Traverse(node->right,Visit);
	
}

template<class T>
void BSTree<T>::Output(void (*Visit)(const T&,const unsigned long)) const
{
	if (root)
		InOrder_Traverse(root,Visit);
}


//Performs the delete if there is zero or one children to the node
//returns true if it could perform this, false if there were two children.
template<class T>
bool BSTree<T>::SimpleDelete(Node<T>*node,Node<T>*parent)
{
	bool ret = false;
	if (!node->left || !node->right) // if either one of them are null
	{
		if (node->right) //check if its the right one
			if (parent) 
				if (node->data > parent->data) //determine parent's direction
					parent->right = node->right; //swap values
				else
					parent->left = node->right;
			else //no parent
				root = node->right;
		else
			if (parent)
				if (node->data > parent->data)
					parent->right = node->left;
				else
					parent->left = node->left;
			else
				root = node->left;
		
		if (parent)
			parent->height = calc_height(parent);
		else
			root->height = calc_height(root);

		ret = true;
		node->left = node->right = NULL; //deleting this so it doesnt delete the whole subtree
										 //when the node is deleted.
	}
	
	return ret;
}

//Recalculates the heights of all nodes in traversals, due to the
//layout being deepest node first, by doing this operation in that
//order the changes propogate up the list.
template<class T>
void BSTree<T>::AdjustHeight(void) //uses traversals
{
	unsigned long x;
	list<Node<T>*>::iterator it,end;

	for (it = traversals.begin(), end = traversals.end();it!=end;++it)
	{
		x = calc_height(*it);
		if (x == (*it)->height) //whenever you get to a height that stays the same, you're done!
			break;
		(*it)->height = x;
	}
}

//Finds and deletes a node, uses SimpleDelete if possible and if it cannot
//it uses the rightmost node of left child method.
template<class T>
void BSTree<T>::Delete(const T & obj) {
	Node<T> *ptr, *tmp, *dest;
	tmp = NULL;
	ptr = root;
	traversals.clear();

	//find the node
	while (ptr)
	{
		if (ptr->data == obj)
			break;
		traversals.push_front(ptr);

		tmp = ptr;

		if (obj < ptr->data)
			ptr = ptr->left;
		if (obj > ptr->data)
			ptr = ptr->right;
		//keep up with this
	}
	//no node found
	if (!ptr)
	{
		traversals.clear(); //so that derived classes that use this list of parents wont perform useless operations
		return; // not in tree
	}

	dest = ptr;

	//attempt simple delete
	if (SimpleDelete(dest,tmp))
	{
		delete dest; //done
	}
	else//use more complex method
	{//rightmost child of left subtree
		traversals.push_front(ptr);

		tmp = ptr;
		ptr = ptr->left;
		while (ptr->right)
		{
			traversals.push_front(ptr);
			tmp = ptr;
			ptr = ptr->right;
		}
		//found rightmost child and it's parent
		SimpleDelete(ptr,tmp); //has to satisfy the true condition... it's rightmost, 1 or 0 children.
		dest->count = ptr->count;
		dest->data = ptr->data; //swap them
		delete ptr;		//delete node
	}
	--size; //if we got here, we deleted a value
	AdjustHeight(); //adjust the heights that were changed
}


//Inserts a value into the tree
template<class T>
void BSTree<T>::Insert(const T & obj) { 
	Node<T> *ptr;


	//this needs no real remarks, it merely finds the proper location to insert the value
	//and makes space for it.
	traversals.clear();
	if (!root)
	{
		ptr = new Node<T>(obj);
		root = ptr;
		size = 1;
	}
	else
	{
		
		ptr = root;
		while (true) //infinite loop because we'll be checking the children of the nodes outside the loop condition
					 //due to the possiblility of multiple outcomes. we break out/continue everywhere that's necessary.
					 //(NOTE: if there is a mindset that infinite loops are intrinsically bad, you havent been in
					 //       the field long enough)
		{
			traversals.push_front(ptr); //keeping traversal path for height adjustments
		

			if (obj == ptr->data)
			{
				++(ptr->count);
				traversals.clear();
				break; //exit loop
			}	
			else if (obj < ptr->data)
				if (ptr->left)
				{
					ptr = ptr->left;
					continue;
				}
				else
					ptr->left = new Node<T>(obj);
			else if (obj > ptr->data)
				if (ptr->right)
				{
					ptr = ptr->right;
					continue;
				}
				else
					ptr->right = new Node<T>(obj);

			++size; //if we got here, we created a new node.
			break;
		}

		AdjustHeight();
			
	}
}
#endif