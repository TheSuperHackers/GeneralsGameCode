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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Library/SEARCH.h                                  $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#if !defined(_USERENTRY)
#define	_USERENTRY
#endif

/*
**	This class is used to create and maintain an index. It does this by assigning a unique
**	identifier number to the type objects that it is indexing. The regular binary sort and search
**	function are used for speedy index retrieval. Typical use of this would be to index pointers to
**	normal data objects, but it can be used to hold the data objects themselves. Keep in mind that
**	the data object "T" is copied around by this routine in the internal tables so not only must
**	it have a valid copy constructor, it must also be efficient. The internal algorithm will
**	create an arbitrary number of default constructed objects of type "T". Make sure it has a
**	default constructor that is efficient and trivial. The constructor need not perform any actual
**	initialization since this class has prior knowledge about the legality of these temporary
**	objects and doesn't use them until after the copy constructor is used to initialize them.
**	This means that the default constructed object of type "T" can be technically in an unusable
**	state since it won't ever be used by this routine and won't ever be returned by any of
**	this template's methods.
**
**	This should properly be called a "map container class" since it is a container with a
**	mapping of an identifier with the element in the container.
*/

template<class T>
class IndexClass
{
	public:
		IndexClass();
		~IndexClass();

		/*
		**	Add element to index table.
		*/
		bool Add_Index(int id, T data);

		/*
		**	Removes an index entry from the index table.
		*/
		bool Remove_Index(int id);

		/*
		**	Check to see if index is present.
		*/
		bool Is_Present(int id) const;

		/*
		**	Fetch number of indexes in the table.
		*/
		int Count() const;

		/*
		**	Actually a fetch an index data element from the table.
		*/
		T Fetch_Index(int id) const;

		/*
		**	Clear out the index table to null (empty) state.
		*/
		void Clear();

	private:
		/*
		**	This node object is used to keep track of the connection between the data
		**	object and the index identifier number.
		*/
		struct NodeElement {
			int ID;			// ID number (must be first element in this structure).
			T Data;			// Data element assigned to this ID number.
		};

		/*
		**	This is the pointer to the allocated index table. It contains all valid nodes in
		**	a sorted order.
		*/
		NodeElement * IndexTable;

		/*
		**	This records the number of valid nodes within the index table.
		*/
		int IndexCount;

		/*
		**	The total size (in nodes) of the index table is recorded here. If adding a node
		**	would cause the index count to exceed this value, the index table must be resized
		**	to make room.
		*/
		int IndexSize;

		/*
		**	If the index table is sorted and ready for searching, this flag will be true. Sorting
		**	of the table only occurs when absolutely necessary.
		*/
		bool IsSorted;

		/*
		**	This records a pointer to the last element found by the Is_Present() function. Using
		**	this last recorded value can allow quick fetches of data whenever possible.
		*/
		NodeElement const * Archive;

		//-------------------------------------------------------------------------------------
		IndexClass(IndexClass const & rvalue);
		IndexClass * operator = (IndexClass const & rvalue);

		/*
		**	Increase size of internal index table by amount specified.
		*/
		bool Increase_Table_Size(int amount);

		/*
		**	Check if archive pointer is the same as that requested.
		*/
		bool Is_Archive_Same(int id) const;

		/*
		**	Invalidate the archive pointer.
		*/
		void Invalidate_Archive();

		/*
		**	Set archive to specified value.
		*/
		void Set_Archive(NodeElement const * node);

		/*
		**	Search for the node in the index table.
		*/
		NodeElement const * Search_For_Node(int id) const;

		static int _USERENTRY search_compfunc(void const * ptr, void const * ptr2);
};


template<class T>
IndexClass<T>::IndexClass() :
	IndexTable(0),
	IndexCount(0),
	IndexSize(0),
	IsSorted(false),
	Archive(0)
{
	Invalidate_Archive();
}


template<class T>
IndexClass<T>::~IndexClass()
{
	Clear();
}


template<class T>
void IndexClass<T>::Clear()
{
	delete [] IndexTable;
	IndexTable = 0;
	IndexCount = 0;
	IndexSize = 0;
	IsSorted = false;
	Invalidate_Archive();
}


// Does not alter the index data.
template<class T>
bool IndexClass<T>::Increase_Table_Size(int amount)
{
	/*
	**	Check size increase parameter for legality.
	*/
	if (amount < 0) return(false);

	NodeElement * table = W3DNEWARRAY NodeElement[IndexSize + amount];
	if (table != nullptr) {

		/*
		**	Copy all valid nodes into the new table.
		*/
		for (int index = 0; index < IndexCount; index++) {
			table[index] = IndexTable[index];
		}

		/*
		**	Make the new table the current one (and delete the old one).
		*/
		delete [] IndexTable;
		IndexTable = table;
		IndexSize += amount;
		Invalidate_Archive();

		/*
		**	Return with success flag.
		*/
		return(true);
	}

	/*
	**	Failure to allocate the memory results in a failure to increase
	**	the size of the index table.
	*/
	return(false);
}


template<class T>
int IndexClass<T>::Count() const
{
	return(IndexCount);
}


template<class T>
bool IndexClass<T>::Is_Present(int id) const
{
	/*
	**	If there are no data elements in the index table, then it can
	**	never find the specified index. Check for and return failure
	**	in this case.
	*/
	if (IndexCount == 0) {
		return(false);
	}

	/*
	**	Check to see if this same index element was previously searched for. If
	**	so and it was previously found, then there is no need to search for it
	**	again -- just return true.
	*/
	if (Is_Archive_Same(id)) {
		return(true);
	}

	/*
	**	Perform a binary search on the index nodes in order to look for a
	**	matching index value.
	*/
	NodeElement const * nodeptr = Search_For_Node(id);

	/*
	**	If a matching index was found, then record it for future reference and return success.
	*/
	if (nodeptr != 0) {
		((IndexClass<T> *)this)->Set_Archive(nodeptr);
		return(true);
	}

	/*
	**	Could not find element so return failure condition.
	*/
	return(false);
}


// WARNING: This routine presumes that the index exists. If it doesn't exist, then the default
// constructed object "T" is returned instead. To avoid this problem, always verify the existence of
// the index by calling Is_Present() first.
template<class T>
T IndexClass<T>::Fetch_Index(int id) const
{
	if (Is_Present(id)) {

		/*
		**	Count on the fact that the archive pointer is always valid after a call to Is_Present
		**	that returns "true".
		*/
		return(Archive->Data);
	}
	static T x;
	return(x);
}


template<class T>
bool IndexClass<T>::Is_Archive_Same(int id) const
{
	if (Archive != 0 && Archive->ID == id) {
		return(true);
	}
	return(false);
}


// This must be performed if ever the archive pointer would become illegal (such as when the element
// it refers to is deleted).
template<class T>
void IndexClass<T>::Invalidate_Archive()
{
	Archive = 0;
}


// Use this routine when there is a good chance that the specified node will be requested in the
// immediate future.
template<class T>
void IndexClass<T>::Set_Archive(NodeElement const * node)
{
	Archive = node;
}


// WARNING: The data is COPIED to internal storage. This means that the data object must have a
// functional and efficient copy constructor and assignment operator.
template<class T>
bool IndexClass<T>::Add_Index(int id, T data)
{
	/*
	**	Ensure that there is enough room to add this index. If not, then increase the
	**	capacity of the internal index table.
	*/
	if (IndexCount + 1 > IndexSize) {
		if (!Increase_Table_Size(IndexSize == 0 ? 10 : IndexSize)) {

			/*
			**	Failure to increase the size of the index table means failure to add
			**	the index element.
			*/
			return(false);
		}
	}

	/*
	**	Add the data to the end of the index data and then sort the index table.
	*/
	IndexTable[IndexCount].ID = id;
	IndexTable[IndexCount].Data = data;
	IndexCount++;
	IsSorted = false;

	return(true);
}


template<class T>
bool IndexClass<T>::Remove_Index(int id)
{
	/*
	**	Find the array index into the table that matches the specified id value.
	*/
	int found_index = -1;
	for (int index = 0; index < IndexCount; index++) {
		if (IndexTable[index].ID == id) {
			found_index = index;
			break;
		}
	}

	/*
	**	If the array index was found, then copy all higher index entries
	**	downward to fill the vacated location. We cannot use memcpy here because the type
	**	object may not support raw copies. C++ defines the assignment operator to deal
	**	with this, so that is what we use.
	*/
	if (found_index != -1) {

		for (int index = found_index+1; index < IndexCount; index++) {
			IndexTable[index-1] = IndexTable[index];
		}
		IndexCount--;

		NodeElement fake;
		fake.ID = 0;
		fake.Data = T();
		IndexTable[IndexCount] = fake;		// zap last (now unused) element

		Invalidate_Archive();
		return(true);
	}

	return(false);
}


// Support function for bsearch and bsort. This compare function presumes that its parameters are
// pointing to NodeElements and that the first "int" in the node is the index ID number to be used
// for comparison.
// WARNING: This is highly dependent upon the layout of the NodeElement structure.
template<class T>
int _USERENTRY IndexClass<T>::search_compfunc(void const * ptr1, void const * ptr2)
{
	if (*(int const *)ptr1 == *(int const *)ptr2) {
		return(0);
	}
	if (*(int const *)ptr1 < *(int const *)ptr2) {
		return(-1);
	}
	return(1);
}


// Performs a binary search on the previously recorded index values.
template<class T>
IndexClass<T>::NodeElement const * IndexClass<T>::Search_For_Node(int id) const
{
	/*
	**	If there are no elements in the list, then it certainly can't find any matches.
	*/
	if (IndexCount == 0) {
		return(0);
	}

	/*
	**	If the list has not yet been sorted, then do so now. Binary searching requires
	**	the list to be sorted.
	*/
	if (!IsSorted) {
		qsort(&IndexTable[0], IndexCount, sizeof(IndexTable[0]), search_compfunc);
		((IndexClass<T> *)this)->Invalidate_Archive();
		((IndexClass<T> *)this)->IsSorted = true;
	}

	/*
	**	This list is sorted and ready to perform a binary search upon it.
	*/
	NodeElement node;
	node.ID = id;
	return((NodeElement const *)bsearch(&node, &IndexTable[0], IndexCount, sizeof(IndexTable[0]), search_compfunc));
}
