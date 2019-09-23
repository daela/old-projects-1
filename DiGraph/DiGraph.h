//----------------------------------------------------------------
//  
//	DiGraph.h
//  
//----------------------------------------------------------------

#ifndef  _DIGRAPH_H_
#define  _DIGRAPH_H_

#include  <iostream>
#include  <vector>
#include <list>
#include <queue>
using namespace std;



	/*	DEPTH FIRST SEARCH (int from, int to)

		vector of bool of visited nodes, visited
		size visited to the number of vertices and initialize it to false

		DFS(from)

		return visited[to];
	*/


	/*  DFS (int i)  (PRIVATE MEMBER)

		visited[i] = TRUE;
		for each entry in v[i]'s adjacency list
			w = entry
			if w hasn't been visited
				dfs(w);
		return
	*/		


	/*	SHORTEST PATH (int start, int destination)
		

		vector of predecessors, P	
		vector of bool of visited nodes, visited
		vector of int for output, path

		queue of nodes, Q
		int for starting node, j
		int for destination node, k

		size P to the number of vertices and initialize to -1
		size visited to the number of vertices and initialize to false

		push start into Q to begin
	
		while Q isn't empty and found is false
			j = front of Q
			set visited[j] to true
			pop front of Q

			for each entry in the adjacency list as long as found is false
				k = entry
				if k has not been visited
					if P[k] != -1  (no predecessor has been previously set)
						P[k] = j;
					if k == destination
						found = true;
					else
						push k onto Q

		if found is true
			j = destination
			while j != -1 (because the predecessor of start is -1)
				insert j at the beginning of path    I.E: path.insert(path.begin(),j);
				j = P[j]

		return path  (this will be empty if there is no path between the two points, so .size() == 0 for no path)
				


	*/

class BadIndex {};
template <class DataType>
class DiGraph
{
	public:
		
		DiGraph() { }

		void AddEdge(int origin, int destination);
		void RemoveEdge(int origin, int destination);
		void AddVertex(DataType item);
		bool HasEdge(int origin, int destination)  const;
		DataType GetData(int at) const;
		int NumVerticies(void);
		void Read(ifstream & inFile);
        void Print(ostream & out);
		bool DepthFirstSearch(int from, int to);
		vector<int> ShortestPath(int start, int destination);



 	private:
	     // "Head nodes" of adjacency lists
		struct VertexInfo
		{
			public:
			DataType data; //Vertext Data
			list<int> adjacencyList; //List Containing Vertex Numbers to which there is an edge from this vertex.
		};

		// Data Members
		vector<VertexInfo> v; //Vector containing all the verticies.
		vector<bool> visited;

		void dfs(int i);


		bool valid_index(int n) const
		{
			if (n < v.size() && n >= 0)
				return true;
			return false;
		}
};

template <class DataType>
void DiGraph<DataType>::AddEdge(int origin, int destination)
{
	--origin;
	--destination; //convert from one-based indexes
	if (valid_index(origin))
	{
		list<int>::iterator it, end;
		for (it = v[origin].adjacencyList.begin(), end = v[origin].adjacencyList.end();it != end;++it)
		{
			if (*it == destination)
				return;
		}
		v[origin].adjacencyList.push_back(destination);
	}
}


template <class DataType>
void DiGraph<DataType>::RemoveEdge(int origin, int destination)
{
	--origin;
	--destination; //convert from one-based indexes
	if (valid_index(origin))
	{
		bool found = false;
		list<int>::iterator it, end;
		for (it = v[origin].adjacencyList.begin(),end=v[origin].adjacencyList.end();it!=end;++it)
		{
			if (*it == destination)
			{
				v[origin].adjacencyList.erase(it);
				return;
			}
		}
	}
}

template <class DataType>
void DiGraph<DataType>::AddVertex(DataType item)
{
	VertexInfo vi;
	vi.data = item;
	vi.adjacencyList.clear();
	v.push_back(vi);
}

template <class DataType>
bool DiGraph<DataType>::HasEdge(int origin, int destination)  const
{
	--origin;
	--destination; //convert from one-based indexes
	if (valid_index(origin))
	{
		list<int>::const_iterator it, end;
		for (it = v[origin].adjacencyList.begin(),end=v[origin].adjacencyList.end();it!=end;++it)
		{
			if (*it == destination)
			{
				return true;
			}
		}
	}
	return false;
}

template <class DataType>
DataType DiGraph<DataType>::GetData(int at) const
{
	--at;
	if (valid_index(at))
		return v[at].data;
	else
		throw BadIndex();
}

template <class DataType>
int DiGraph<DataType>::NumVerticies(void)
{
	return v.size();
}

template <class DataType>
void DiGraph<DataType>::Read(ifstream & inFile)
{
	DataType dt;
	int n, i;
	VertexInfo vi;
	v.clear();
	while (inFile >> dt >> n)
	{
		AddVertex(dt);
		while (n-- && inFile >> i)
		{
			AddEdge(NumVerticies(),i);
		}
	}
}

template <class DataType>
void DiGraph<DataType>::Print(ostream & out)
{
	vector<VertexInfo>::iterator it, end;
	list<int>::iterator ait, aend;
	int i = 1;
	for (it = v.begin(),end = v.end();it!=end;++it)
	{
		out << i++ << " " << it->data << " " << it->adjacencyList.size();
		for (ait = it->adjacencyList.begin(),aend = it->adjacencyList.end();ait != aend;++ait)
			out << " " << *ait+1;
		out << endl;
	}


}
/* This member function prints all the vertices and their adjacency lists. Format is as follows.
   VertexNumber VertexData --- NumbersOfVerticiesForWhichThereIsAnEdgeFromThisVertex
*/

template <class DataType>
bool DiGraph<DataType>::DepthFirstSearch(int from, int to)
{
	--from;
	--to; //convert from one-based indexes
	int n = NumVerticies();
	visited.clear();
	while (n--)
		visited.push_back(false);

	dfs(from);
	
	return visited[to];
}

template <class DataType>
void DiGraph<DataType>::dfs(int i)
{
	list<int>::iterator it, end;
	visited[i] = true;

	for (it = v[i].adjacencyList.begin(), end = v[i].adjacencyList.end();it != end;++it)
	{
		if (!visited[*it])
			dfs(*it);
	}
}
template <class DataType>
vector<int> DiGraph<DataType>::ShortestPath(int start, int destination)
{
	--start;
	--destination; //convert from one-based indexes
	int n = NumVerticies();
	list<int>::iterator it, end;

	vector<int> predecessor;
	vector<int> path;
	bool found = false;
	visited.clear();
	
	while (n--)
	{
		visited.push_back(false);
		predecessor.push_back(-1);
	}


	queue<int> pBuf;
	pBuf.push(start);

	while (!pBuf.empty() && !found)
	{
		int j = pBuf.front();
		pBuf.pop();
		visited[j] = true;

		for (it = v[j].adjacencyList.begin(),end = v[j].adjacencyList.end();it!=end && !found;++it)
		{
			int k = *it;
			if (!visited[k])
			{
				if (predecessor[k] == -1)
					predecessor[k] = j;
				if (k == destination)
					found = true;
				else
					pBuf.push(k);
			}
		}
	}

	if (found)
	{
		int j = destination;
		while (j != -1)
		{
			path.insert(path.begin(),j+1);
			j = predecessor[j];
		}
	}

	return path;
}


#endif
