//-----------------------------------------------------------------------------
//				Heap.h
//
//	Author:		Jacob McIntosh
//	Section:	01
//	Date:		March 30, 2004
//
//	This is the implementation of the Heap class, all in one file due to
//  it being templated and placing it in one file being recommended by teacher.
//  NOTE: Used zero indexes, so left/right/parent methods are slightly changed.
//  
//
//-----------------------------------------------------------------------------

#include <vector>

using namespace std;

class Underflow
{ };
class OutOfBounds
{ };

template<class T>
class Heap
{
	vector<T> data;
	bool valid_heap;

	inline int LeftChild(int node) { return (node+1)*2-1; }
	inline int RightChild(int node) { return (node+1)*2; }
	inline int Parent(int node) { return (node+1)/2-1; }

	inline bool WithinBounds(int node) const
	{
		if (node < 0 || node >= size())
			return false;
		return true;
	}

	void percolateDown(int node);
	void percolateUp(int node);
	
	public:

		Heap(void) { valid_heap = true; }
		
		inline bool isheap(void) const { return valid_heap; }
		inline bool empty(void) const { return (!size()); }

		inline int size(void) const { return data.size(); }

		inline void clear(void) { data.clear(); valid_heap = true; }

		void push(const T & obj);

		void insert(const T & obj);
		const T & top(void);
		void pop(void);

		const vector<T> sort(void);
		//added this to view the heap's layout
		inline const vector<T> & buffer(void) { return data; }

		void heapify(void);

};


template<class T>
void Heap<T>::percolateDown(int node)
{
	//just like the slides, basically
	int x = data[node], n;
	while (WithinBounds(LeftChild(node)))
	{
		n = LeftChild(node);
		if (WithinBounds(n+1)) //if there's a right child
			if (data[n+1] > data[n]) //and it's greater than the left child
				++n; //put the index at the right child;

		if (data[n] > x) //swap the values if the largest child is greater than x
			data[node] = data[n];
		else	// heap satisfied
			break;
		node = n;
	}
	//set the final position to x, since we never really did that in the algorithm
	data[node] = x;
}

template<class T>
void Heap<T>::percolateUp(int node)
{
	//just like the slides, basically
	int x = data[node];
	while (node != 0 && x > data[Parent(node)])
	{
  		// cannot put x in heap[i]
		data[node] = data[Parent(node)]; // move element down
		node = Parent(node); // move to parent
	}
	data[node] = x;
}

template<class T>
void Heap<T>::push(const T & obj) {
	/*
		the instructions said that this function maintains the heap property
		if it already has it, but doesnt say what to do otherwise.. so i
		made it simply insert otherwise... you can call heapify later.
	*/
	if (valid_heap)
	{
		//must still satisfy
		data.push_back(obj);
		percolateUp(size()-1);
	}
	else
	{
		//doesnt have to satisfy
		insert(obj);
	}
}

template<class T>
void Heap<T>::insert(const T & obj)
{
	//cheap insertion
	valid_heap = false;
	data.push_back(obj);
}

template<class T>
const T & Heap<T>::top(void)
{
	if (!valid_heap)
		heapify();

	if (size())
		return data[0];
	else
		throw Underflow();
}

template<class T>
void Heap<T>::pop(void)
{
	if (!valid_heap)
		heapify();

	if (!size()) //cant pop an empty heap
		throw Underflow();

	data[0] = data[size()-1];
	data.pop_back();

	if (!size()) //empty heap = valid
		valid_heap = true;
	else
		percolateDown(0);
}

template<class T>
const vector<T> Heap<T>::sort(void)
{
	if (!valid_heap)
		heapify();

	vector<T> tmp = data; // backup of our data.
	vector<T> output;
	
	while (size())
	{
		//inserting at beginning, because the numbers will be getting smaller each time
		//this ensures ascending order.
		output.insert(output.begin(),top());
		pop();
	}
	data = tmp;
	return output;
}

template<class T>
void Heap<T>::heapify(void)
{
	//like the slides
	int node = (size()-1)/2;
	while (node >= 0)
	{
		percolateDown(node);
		--node;
	}
	valid_heap = true;
}
