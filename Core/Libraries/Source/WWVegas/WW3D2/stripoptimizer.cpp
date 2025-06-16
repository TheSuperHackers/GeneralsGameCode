/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stripoptimizer.h"
#include "hashtemplate.h"
#include "wwdebug.h"

template <class T> inline void swap (T& a, T& b)
{
	T t(a);
	a = b;
	b = t;
}

//------------------------------------------------------------------------
// Prototypes for sort functions. Note that class T must have
// operators =, > and < in order for the functions to compile.
//------------------------------------------------------------------------

template <class T> inline void Insertion_Sort	(T* a, int N);
template <class T> inline void Quick_Sort		(T* a, int N);

template <class T> inline void Insertion_Sort (T* a, int N)
{
	for (int i = 1; i < N; i++)
	if (a[i-1] > a[i])
	{
		T		v	= a[i];
		int		j	= i;
		while (a[j-1]> v)
		{
			a[j] = a[j-1];						// copy data
			j--;
			if (!j)
				break;
		};
		a[j]	= v;
	}
}


template <class T> inline void Quick_Sort (T* a, int l, int r)
{
	if (r-l <= 16)
	{
		Insertion_Sort(a+l,r-l+1);
		return;
	}
	T			v = a[r];
	int			i = l-1;
	int			j = r;
	do 
	{
		do { i++; } while (i < r && a[i] < v);
		do { j--; } while (j > 0 && a[j] > v);
		swap(a[i],a[j]);
	} while (j > i);
	
	T t     = a[j];
	a[j]	= a[i];	  
	a[i]	= a[r];		
	a[r]	= t;
	if ((i-1) > l)
		Quick_Sort (a,l,i-1);
	if (r > (i+1))
		Quick_Sort (a,i+1,r);
}

template <class T> inline void Quick_Sort (T* a, int N)
{
	Quick_Sort(a,0,N-1);
}

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

/*****************************************************************************
 *
 * Function:		StripOptimizerClass::getStripIndexCount()
 *
 * Description:		Returns number of indices in a set of strips
 *
 * Parameters:
 *
 *****************************************************************************/

int StripOptimizerClass::Get_Strip_Index_Count (const int* strips, int strip_count)
{
	int cnt = 0;
	for (int i = 0; i < strip_count; i++)
	{
		int len = *strips++;		// read len
		cnt += len;
		strips += len;				// skip data
	}
	return cnt;
}

/*****************************************************************************
 *
 * Function:		StripOptimizerClass::optimizeStripOrder()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

// compares two strips and returns number of shared indices
static inline int Get_Strip_Similarity (const int* A, const int* B)
{
	int cnt = 0;
	int lenA = *A++;
	int lenB = *B++;

	for (int i = 0; i < lenA; i++)
	{
		int index = A[i];									// index of A
		for (int j = 0; j < lenB; j++)
		if (B[j] == index)									// match
		{
			cnt++;
			break;
		}
	}

	return cnt;
}

// copies a strip, returns new destination pointer
static inline int* Copy_Strip (int* d, const int* s)
{
	int len = *s++;
	*d++ = len;
	for (int i = 0; i < len; i++)
		*d++ = *s++;
	return d;
}

void StripOptimizerClass::Optimize_Strip_Order (int* strips, int strip_count)
{
	if (strip_count <= 0)
		return;
//	WWASSERT(strips);
	
	int**	ss = W3DNEWARRAY int*[strip_count];							// pointers to beginning of strips
	int* s = strips;
	int i = 0;
	for (; i < strip_count; i++)
	{
		ss[i] = s;
		int len = *s++;			// read len
		s+=len;					// skip
	}
	int outSize = Get_Strip_Index_Count(strips, strip_count)+strip_count;	// output memory alloc size
	int* out	= W3DNEWARRAY int[outSize];	
	int* o		= out;											// output pointer

	const int* prev = ss[0];								// previous strip
	o = Copy_Strip (o, ss[0]);								// output first strip
	ss[0] = 0;

	for (;;)
	{
		int		bestIndex		= -1;
		int		bestSimilarity	= -1;

		for (int j = 0; j < strip_count; j++)
		if (ss[j])
		{
			int sim = Get_Strip_Similarity (prev, ss[j]);		// get similarity
			if (sim > bestSimilarity)
			{
				bestSimilarity = sim;
				bestIndex	   = j;
			}
		}

		if (bestIndex==-1)									// we're done
			break;

		o = Copy_Strip(o, ss[bestIndex]);					// copy the strip
		prev = ss[bestIndex];								// set to prev
		ss[bestIndex] = NULL;								// mark as selected
	}

//	WWASSERT((out+outSize)==o);							// HUH?

	for (i = 0; i < outSize; i++)						// copy output
		strips[i] = out[i];

	delete[] out;
	delete[] ss;
}

/*****************************************************************************
 *
 * Function:		StripOptimizerClass::optimizeTriangleOrder()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

struct Tri
{
public:
	int a,b,c;

	bool operator< (const Tri& s) const { return a < s.a; }
	bool operator> (const Tri& s) const { return a > s.a; }
};

static inline int Get_Similarity (const Tri& A, const Tri& B)
{
	int sim = 0;

	if (A.a == B.a || A.a == B.b || A.a == B.c) sim++;
	if (A.b == B.a || A.b == B.b || A.b == B.c) sim++;
	if (A.c == B.a || A.c == B.b || A.c == B.c) sim++;

	return sim;
}

void StripOptimizerClass::Optimize_Triangle_Order (int *tris, int triangle_count)
{
	if (triangle_count<=0)
		return;
	WWASSERT(tris);

	Tri** t = W3DNEWARRAY Tri*[triangle_count];
	int i = 0;
	for (; i < triangle_count; i++)
	{
		t[i] = (Tri*)(tris + i*3);
	}

	Tri* out = W3DNEWARRAY Tri[triangle_count];
	Tri* o   = out;

	Tri* prev = t[0];

	*o++ = *prev;
	t[0] = NULL;

	for (;;)
	{
		// match best
		int bestIndex = -1;
		int bestSim   = -1;

		for (int j = 0; j < triangle_count; j++)
		if (t[j])
		{
			int sim = Get_Similarity (*prev, *t[j]);
			if (sim > bestSim)
			{
				bestSim   = sim;
				bestIndex = j;
				if (sim >= 2)										// that's the best we could get
					break;
			}
		}

		if (bestIndex == -1)										// we're done
			break;

		*o++ = *t[bestIndex];
		prev = t[bestIndex];								
		t[bestIndex] = NULL;
	}


	WWASSERT(o == (out+triangle_count));

	for (i = 0; i < triangle_count; i++)
	{
		Tri* d = (Tri*)(tris)+i;
		*d = out[i];
	}

	delete[] t;
	delete[] out;

}


/*****************************************************************************
 *
 * Function:		StripOptimizerClass::combineStrips()
 *
 * Description:		Combines a number of strips into one
 *
 * Parameters:
 *
 *****************************************************************************/

int* StripOptimizerClass::Combine_Strips (const int* strips, int strip_count)
{
	int check = Get_Strip_Index_Count(strips,strip_count) + (strip_count-1)*3;

	int* out	= W3DNEWARRAY int[check+1];
	int* o		= out + 1;

	int* tmp = W3DNEWARRAY int[check];	// DEBUG DEBUG

	bool prevEven = true;

	for (int i = 0; i < strip_count; i++)
	{
		int len = *strips++;

		if (i != 0)
		{
			*o++ = *strips;						// duplicate first
		
			if (!prevEven)
				*o++ = *strips;
		}

		for (int j = 0; j < len; j++)
			*o++ = *strips++;					// copy the strip

		if (i != (strip_count-1))
			*o++ = o[-1];

		prevEven = (!(len&1));
	}

	delete[] tmp;
//	WWASSERT(check == (o-out-1));
	*out = (o-out-1);						// set length
	return out;
}

//------------------------------------------------------------------------
// New striping code
//------------------------------------------------------------------------

namespace Strip
{

/*****************************************************************************
 *
 * Struct:			Vector3i
 * 
 * Description:		Internal class for representing an edge
 *
 *****************************************************************************/

struct Vector3i
{
	int	v[3];
				Vector3i () {}
				Vector3i (int a, int b, int c) { v[0]=a; v[1]=b; v[2]=c; }

	int&		operator[](int i)		{ return v[i]; }
	const int&	operator[](int i) const { return v[i]; }

};

/*****************************************************************************
 *
 * Struct:			Edge
 * 
 * Description:		Internal class for representing an edge
 *
 *****************************************************************************/

struct Edge
{
				Edge		(void)					{}
				Edge		(int v0, int v1)		{ v[0] = v0; v[1] = v1;								}
	bool		operator==	(const Edge& s) const	{ return v[0]==s.v[0] && v[1] == s.v[1];			}
	void		sort		(void)					{ if (v[0]>v[1]) swap(v[0],v[1]);					}

	int			v[2];						// edge
};

/*****************************************************************************
 *
 * Struct:			Triangle
 * 
 * Description:		Internal class for representing a triangle and 
 *					associated connectivity information
 *
 *****************************************************************************/

struct Triangle
{
	Triangle (void)					
	{ 
		m_neighbors[0] = 0;
		m_neighbors[1] = 0;
		m_neighbors[2] = 0;
		m_vertices[0]  = 0;
		m_vertices[1]  = 0;
		m_vertices[2]  = 0;
		m_prev		   = 0;
		m_next		   = 0;
		m_bin		   = -1;
	}

	Triangle*	m_neighbors[3];				// pointers to neighbors of the triangle
	Vector3i	m_vertices;					// vertices of the triangle

	Triangle*	m_prev;						// previous triangle in same bin
	Triangle*	m_next;						// next triangle in same bin
	int			m_bin;						// current bin (-1 == not in any bin)

	int			getConnectivity (void) const	{ int cnt = 0; if (m_neighbors[0]) cnt++; if (m_neighbors[1]) cnt++; if (m_neighbors[2]) cnt++; return cnt;}
	const Edge	getEdge			(int i) const	{ WWASSERT(i>=0 && i<3); return Edge(m_vertices[i],i==2?m_vertices[0]:m_vertices[i+1]); }

};

/*****************************************************************************
 *
 * Class:			TriangleQueue
 * 
 * Description:		Internal class for maintaining triangles sorted by
 *					connectivity
 *
 *****************************************************************************/

class TriangleQueue
{
public:
					TriangleQueue			(Triangle* tris, int N);
					~TriangleQueue			(void);
	void			removeTriangle			(Triangle* t);
	Triangle*		getTop					(void) const;
	int				getVertexConnectivity	(int i)  const;
private:
					TriangleQueue			(const TriangleQueue&);
	TriangleQueue&	operator=				(const TriangleQueue&);
	void			reinsert				(Triangle* t);

	Triangle*		m_bin[4];				// bins (0 = triangles with zero neighbors, etc.)
	int*			m_nodeConnectivity;		// node connectivity count
	int				m_vertexCount;
};

/*****************************************************************************
 *
 * Class:			Stripify
 * 
 * Description:		Class for performing stripification
 *
 *****************************************************************************/

class Stripify
{
public:
	static int*			stripify	(const Vector3i* tris, int N);
private:
						Stripify							(void);	// not permitted
						Stripify	(const Stripify&);
	Stripify&			operator=	(const Stripify&);

	static Triangle*	generateTriangleList				(const Vector3i* tris, int N);
	static Vector3i		getTriangleNodeConnectivityWeights	(const TriangleQueue& queue, const Triangle& tri);
	static int			getMod3								(int i) { WWASSERT(i>=0 && i<6); return s_mod[i]; }
	static int			s_mod[6];						// small lookup table for mod3 operation (see getMod3())
};

int Stripify::s_mod[6] = {0,1,2,0,1,2};
} // Strip


template <> inline unsigned int HashTemplateKeyClass<Strip::Edge>::Get_Hash_Value(const Strip::Edge& s)
{
	return (s.v[0]*139) + (s.v[1]*7);
}

namespace Strip
{
/*****************************************************************************
 *
 * Function:		TriangleQueue::getTop()
 *
 * Description:		Returns pointer to triangle with smallest connectivity
 *
 * Returns:			pointer to triangle with smallest connectivity or NULL
 *					if the queue is empty
 *
 *****************************************************************************/

inline Triangle* TriangleQueue::getTop	(void) const
{
	for (int i = 0; i < 4; i++)
	if (m_bin[i])
		return m_bin[i];				// return head
	return 0;							// end
}

/*****************************************************************************
 *
 * Function:		TriangleQueue::getVertexConnectivity()
 *
 * Description:		Returns current connectivity count of specified vertex
 *
 * Parameters:		i = vertex index
 *
 * Returns:			connectivity count
 *
 *****************************************************************************/

inline int	TriangleQueue::getVertexConnectivity (int i)  const  
{ 
	WWASSERT(i>=0 && i< m_vertexCount); 
	return m_nodeConnectivity[i]; 
}

/*****************************************************************************
 *
 * Function:		TriangleQueue::~TriangleQueue()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

inline TriangleQueue::~TriangleQueue ()
{
	delete[] m_nodeConnectivity;
}

/*****************************************************************************
 *
 * Function:		TriangleQueue::reinsert()
 *
 * Description:		Internal function for recalculating a triangle's
 *					connectivity
 *
 * Parameters:		t = pointer to triangle (non-NULL)
 *
 *****************************************************************************/

inline void TriangleQueue::reinsert (Triangle* t)
{
	WWASSERT (t && t->m_bin!=-1);					// must be in some bin
	int w = t->getConnectivity();

	// remove from bin
	if (t->m_prev)
		t->m_prev->m_next = t->m_next;
	else
	{
		WWASSERT(t->m_bin != -1);
		WWASSERT(t == m_bin[t->m_bin]);			// must be head
		m_bin[t->m_bin] = t->m_next;			// new head
	}

	if (t->m_next)
		t->m_next->m_prev = t->m_prev;			

	t->m_prev = 0;
	t->m_next = m_bin[w];
	if (t->m_next)
		t->m_next->m_prev = t;

	m_bin[w] = t;								// head of bin
	t->m_bin = w;								// set bin
}

/*****************************************************************************
 *
 * Function:		TriangleQueue::removeTriangle()
 *
 * Description:		Removes a triangle from the queue
 *
 * Parameters:		t = pointer to triangle (non-NULL)
 *
 *****************************************************************************/

inline void TriangleQueue::removeTriangle	(Triangle* t)
{
	WWASSERT(t);
	if (t->m_prev)
		t->m_prev->m_next = t->m_next;
	else
	{
		WWASSERT(t->m_bin != -1);
		WWASSERT(t == m_bin[t->m_bin]);			// must be head
		m_bin[t->m_bin] = t->m_next;			// new head
	}

	if (t->m_next)
		t->m_next->m_prev = t->m_prev;			
	t->m_bin = -1;

	// update connectivity of t's neighbors

	Triangle* update[3];
	int			i;

	for (i = 0; i < 3; i++)
	{
		update[i]  = 0;
		if (t->m_neighbors[i])
		{
			Triangle* n = t->m_neighbors[i];
			int k=0;
			for (k = 0; k < 3; k++)
			if (n->m_neighbors[k]==t)
				break;
			WWASSERT (k!=3);							// WASS??
			n->m_neighbors[k] = 0;					// reduce connection
			t->m_neighbors[i] = 0;
			update[i] = n;
		}
	}

	// update connectivity count of t's vertices

	for (i = 0; i < 3; i++)
	{
		m_nodeConnectivity[t->m_vertices[i]]--;
		WWASSERT(m_nodeConnectivity[t->m_vertices[i]] >= 0);		// WASS?
	}

	for (i = 0; i < 3; i++)							// perform reinsertions now...
	if (update[i])
		reinsert(update[i]);

}

/*****************************************************************************
 *
 * Function:		TriangleQueue::TriangleQueue()
 *
 * Description:		Constructor
 *
 * Parameters:		tris = array of triangles
 *					N	 = number of triangles in the array
 *
 *****************************************************************************/

inline TriangleQueue::TriangleQueue	(Triangle* tris, int N)
{
	int i;
	for (i = 0; i < 4; i++)
		m_bin[i] = 0;							// initialize to zero

	int largestIndex = 0;

	for (i = 0; i < N; i++)
	for (int j = 0; j < 3; j++)
	{
		WWASSERT(tris[i].m_vertices[j]>=0);
		if (tris[i].m_vertices[j] > largestIndex)
			largestIndex = tris[i].m_vertices[j];
	}

	m_vertexCount	   = largestIndex+1;
	m_nodeConnectivity = W3DNEWARRAY int[m_vertexCount];			// 
	for (i = 0; i < m_vertexCount; i++)
		m_nodeConnectivity[i]  = 0;


	for (i = 0; i < N; i++)
	{
		Triangle* t = tris+i;
		int w = t->getConnectivity();			
		WWASSERT(w>=0 && w <=3);
		WWASSERT(!t->m_prev && !t->m_next && t->m_bin==-1);	// must not be in a bin
		t->m_prev = 0;
		t->m_next = m_bin[w];
		if (t->m_next)
			t->m_next->m_prev = t;
		m_bin[w] = t;							// head of bin
		t->m_bin = w;							// set bin
		for (int j = 0; j < 3; j++)
			m_nodeConnectivity[t->m_vertices[j]]++;
	}
}


/*****************************************************************************
 *
 * Function:		Stripify::getTriangleConnectivityWeights()
 *
 * Description:		Returns "node connectivity" weights for each triangle vertex
 *
 * Parameters:		queue	= reference to triangle queue
 *					tri		= reference to triangle
 *
 * Returns:			Vector structure containing three weights
 *
 *****************************************************************************/

inline Vector3i Stripify::getTriangleNodeConnectivityWeights (const TriangleQueue& queue, const Triangle& tri)
{
	int weight[3];
	int i = 0;
	for (; i < 3; i++)
	{
		weight[i] = queue.getVertexConnectivity(tri.m_vertices[i]);
	}
	int highestVal = weight[0];
	if (weight[1] > highestVal) highestVal = weight[1];
	if (weight[2] > highestVal) highestVal = weight[2];

	Vector3i v(-1,-1,-1);

	for (i = 0; i < 3; i++) {
		if (weight[0] == highestVal) v[i] = +1;
	}
		
	return v;
}

/*****************************************************************************
 *
 * Function:		Stripify::generateTriangleList()
 *
 * Description:		Converts input triangles into internal Triangle structure
 *
 * Parameters:		inTris	= input triangles
 *					N		= number of input triangles
 *
 * Returns:			pointer to Triangle array
 *
 * Notes:			The function sets up the initial connectivity information
 *
 *****************************************************************************/

Triangle* Stripify::generateTriangleList (const Vector3i* inTris, int N)
{
	WWASSERT (inTris && N>=0);

	Triangle*	tris = W3DNEWARRAY Triangle[N];			// allocate triangles
	int			i;

	//--------------------------------------------------------------------
	// Copy triangle vertex data 
	//--------------------------------------------------------------------

	for (i = 0; i < N; i++)				
	{
		//--------------------------------------------------------------------
		// We could perform random rotation here (this way we don't need random
		// comparisons later in the algo in equality cases).
		//--------------------------------------------------------------------

		tris[i].m_vertices[0] = inTris[i][0];
		tris[i].m_vertices[1] = inTris[i][1];
		tris[i].m_vertices[2] = inTris[i][2];
	}

	//--------------------------------------------------------------------
	// Build connectivity information using a hash table
	//--------------------------------------------------------------------

	HashTemplateClass<Edge,Triangle*> hash;
	Triangle*			 t = tris;

	for (i = 0; i < N; i++, t++)
	{
		for (int j = 0; j < 3; j++)
		if (!t->m_neighbors[j])											// if neighbor not defined yet
		{
			Edge	edge	= tris[i].getEdge(j);
			Edge	e		= edge;
			e.sort();													// sort vertices (smaller first)
			
			Triangle* n = hash.Get(e);
			if (n)														// if edge is already in the hash...
			{
				int k=0;
				for (k = 0; k < 3; k++)									// find matching edge
				if (!n->m_neighbors[k])									// this neighbor not located yet
				{
					Edge ek = n->getEdge(k);							// get edge
					if (ek.v[0] == edge.v[1] && ek.v[1] == edge.v[0])	// if matching edge (note that order must be different)
						break;											// .. we found the edge
				}

				if (k < 3)												// (k==3) -> no match
				{
					t->m_neighbors[j] = n;								// set neighbor
					n->m_neighbors[k] = t;								// set neighbor
					hash.Remove(e);										// remove from hash (this speeds up the hash queries considerably)
				}
			} else
				hash.Insert(e, t);										// insert triangle into hash
		}
	}

	return tris;														// return pointer to output data
}

/*****************************************************************************
 *
 * Function:		Stripify::stripify()
 *
 * Description:		Builds a set of triangle strips out of a triangle array
 *
 * Parameters:		inTris	= input triangles (three vertices per triangle)
 *					N		= number of input triangles
 *
 * Returns:			pointer to strip data array
 *
 * Notes:			The strip data array layout is as follows:
 *
 *					[int]		number of strips
 *					[int]		length of first strip (in vertices)
 *					[int,..]	vertex indices of the first strip
 *					[int]		length of second strip
 *					...
 *
 *					The routine assumes that degenerate triangles have been
 *					remove and input vertices have been welded
 *
 *****************************************************************************/

int* Stripify::stripify  (const Vector3i* inTris, int N)
{
	if (!inTris || N<=0)												// boo!
		return 0;

	//--------------------------------------------------------------------
	// Initial setup
	//--------------------------------------------------------------------

	Triangle*		tris	= generateTriangleList(inTris,N);			// build connectivity info
	int*			out		= W3DNEWARRAY int[N*5];								// absolute worst case situation
	int*			o		= out;										// internal ptr (save entry[0] for later use)
	int				strip_count	= 0;										// number of output strips
	TriangleQueue	queue (tris, N);									// insert triangles into priority queue

	int nSwaps = 0;
	int nLen   = 0;

	//--------------------------------------------------------------------
	// Main loop. Always select triangle with smallest weight.
	//--------------------------------------------------------------------

	for (;;)
	{
		Triangle* t = queue.getTop();									// get triangle with smallest weight
		if (!t)															// ok, we ran out of triangles (we're done)
			break;

		//--------------------------------------------------------------------
		// Choose initial direction by selecting neighbor with smallest
		// weight
		//--------------------------------------------------------------------

		int*	pLen		= o;										// get current pointer (as we need to take care of it later)
		int		len			= 3;										// initial length
		int		best		= 0;										// best edge
		int		bestWeight	= 0x7fffffff;								// initialize to maximum width

		Vector3i nodeWeights = getTriangleNodeConnectivityWeights(queue, *t);
		
		for (int i = 0; i < 3; i++)
		if (t->m_neighbors[i])											// if triangle has a neighbor in this direction
		{
			int	weight = t->m_neighbors[i]->getConnectivity();			// get connectivity of neighbor

			weight += nodeWeights[i];									// add node weights
			weight += nodeWeights[getMod3(i+1)];

			// DEBUG DEBUG ADD SWAP COST HERE?
			if (weight <= bestWeight)
			{
				bestWeight = weight;
				best       = i;
			}
		}

		*o++ = len;														// output the first three vertices
		*o++ = t->m_vertices[getMod3(best+2)];
		*o++ = t->m_vertices[getMod3(best+0)];
		*o++ = t->m_vertices[getMod3(best+1)];

		for (;;)
		{
			Triangle* next = 0;											// find next triangle
		
			int i;
			for (i = 0; i < 3; i++)
			if (t->m_neighbors[i])
			{
				Triangle* n = t->m_neighbors[i];
				for (int j = 0; j < 3; j++)
				{
					Edge e = n->getEdge(j);
					if (!(len&1))
						swap(e.v[0],e.v[1]);							// swap
					if (e.v[0] == o[-1] && e.v[1] == o[-2])
					{
						next = n;
						break;
					}
				}
			}

			queue.removeTriangle(t);									// get rid of the old triangle

			if (!next)													// we're done
				break;

			//--------------------------------------------------------------------
			// Find out where we want to continue...
			//--------------------------------------------------------------------
	
			int		bestEdge	= -1;
			int		bestWeight	= 0x7fffffff;
			bool	bestSwap	= false;

			Vector3i nodeWeights = getTriangleNodeConnectivityWeights(queue, *next);
			
			for (i = 0; i < 3; i++)
			if (next->m_neighbors[i])									// is there a neighbor?
			{
				Edge e			= next->getEdge(i);						// a swap happens if it contains the prevprev

				bool swap = (e.v[0] == o[-2] || e.v[1] == o[-2]);

				Triangle* n = next->m_neighbors[i];						// get pointer to neighbor
				int       w = n->getConnectivity();


				w += nodeWeights[i];									// add vertex weight
				w += nodeWeights[getMod3(i+1)];							// add vertex weight

				w += (swap) ? 1 : -1;									// add swap penalty
							
				if (w <= bestWeight)
				{
					bestWeight = w;
					bestEdge   = i;
					bestSwap   = swap;
				}
			}

			if (bestEdge != -1 && bestSwap)								// if we're going to continue...
			{
				*o = o[-2];												// introduce swap vertex
				o++;
				len++;													// adjust length
				nSwaps++;												// update statistics
			}

			//--------------------------------------------------------------------
			// Find out which was the new vertex (store it to vIndex)
			//--------------------------------------------------------------------

			int vIndex = 0;

			if (next->m_vertices[1] != o[-1] && next->m_vertices[1] != o[-2]) vIndex = 1; else
			if (next->m_vertices[2] != o[-1] && next->m_vertices[2] != o[-2]) vIndex = 2; else
				WWASSERT (next->m_vertices[0] != o[-1] && next->m_vertices[0] != o[-2]);

			//--------------------------------------------------------------------
			// Output the vertex and move to next triangle
			//--------------------------------------------------------------------

			*o++ = next->m_vertices[vIndex];							// output the vertex
			len++;														// increase strip length
			t = next;													// move to next triangle
		}
		
		*pLen = len;													// patch final length
		strip_count++;														// increase strip count

		nLen += len;
//		printf ("strip len = %d\n",len);
	}

	//--------------------------------------------------------------------
	// Allocate new optimal-size array and copy output there. Then release
	// all temporary memory allocations.
	//--------------------------------------------------------------------

//	printf ("total indices = %d\n",nLen);
//	printf ("total swaps   = %d\n",nSwaps);

	int		len		= o-out;											// allocation length
	int*	rOut	= W3DNEWARRAY int[len+1];

	*rOut = strip_count;													// first entry is number of strips
	for (int i = 0; i < len; i++)
	{
		WWASSERT(out[i] >= 0);											// would be nice to test for len as well
		rOut[i+1] = out[i];
	}

	delete[] out;														// delete internal output
	delete[] tris;														// delete internal triangle list

	return rOut;
}
} // Strip
int* StripOptimizerClass::Stripify(const int* tris, int N)
{
	return Strip::Stripify::stripify((const Strip::Vector3i*)tris,N);
}

//------------------------------------------------------------------------

