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

/***************************************************************************
 *                                                                         *
 *                 Project Name : G                                        *
 *                                                                         *
 *                    File Name : SLIST.h                                  *
 *                                                                         *
 *                   Programmer : Philip W. Gorrow                         *
 *                                                                         *
 *                   Start Date : 03/11/97                                 *
 *                                                                         *
 *                  Last Update : March 11, 1997 [PWG]                     *
 *                                                                         *
 *  The Single List Class is responsible for all management functions that *
 *  can be performed on a singular linked list.  It uses the SLNode class  *
 *  to track the links and maintain a pointer to the object being tracked. *
 *                                                                         *
 *  The singlely linked list class is non destructive.  That is only       *
 *  pointers to the actual data being stored in the nodes are kept.  The   *
 *  users is responsible for creating the objects to be added and deleting *
 *  the objects once they are removed from the list if they need to be.    *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   *SList<T>::Remove_Head -- Removes the head of the list                *
 *   *SList<T>::Remove_Tail -- Removes the tail element from the list      *
 *   SList<T>::Get_Count -- Returns a count of the entries in the list     *
 *   *SList<T>::Head -- Returns the head node of the list                  *
 *   *SList<T>::Tail -- Returns the tail node of the list                  *
 *   SList<T>::Is_Empty -- Returns true if the list is empty               *
 *   SList<T>::Add_Head -- Adds a node to the head of the list             *
 *   SList<T>::Add_Head -- Adds a list to to the head of the list          *
 *   SList<T>::Add_Tail -- Adds a node to the tail of the list             *
 *   SList<T>::Add_Tail -- Adds a list to the tail of the list             *
 *   *SList<T>::Find_Node -- returns first node in list matching the input *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "SLNODE.h"

template <class T>
class SList {
  private:
		SLNode<T> *HeadNode;    // Note: Constructor not called for pointer.
		SLNode<T> *TailNode;

	public:
		//
		// This constructor is inlined because G++ will not create the
		// constructor correctly if it is not defined within the class
		// definition.
		//
		SList(void)
		{
			HeadNode = nullptr;
			TailNode = nullptr;
		};

		virtual ~SList(void) { Remove_All(); };

		//
		// Returns the current head of the list.  Used to walk the list.
		//
		SLNode<T> *Head(void) const;
		SLNode<T> *Tail(void) const;

		SLNode<T> *Find_Node(T * data) const;

		virtual bool Add_Head(T *data);           // Add   object to head of list
		virtual bool Add_Head(SList<T>& list);    // Add a list  to head of list
		virtual bool Add_Tail(T *data);           // Add   object to tail of list
		virtual bool Add_Tail(SList<T>& list);    // Add a list  to tail of list

		virtual T *Remove_Head(void);             // Remove head node  from list
		virtual T *Remove_Tail(void);             // Remove tail node  from list
		virtual bool Remove(T *element);          // remove an individual element
		virtual void Remove_All(void);            // Remove all  nodes from list

		// Insert before oldnode, if oldnode is null then before head node
		virtual bool Insert_Before(T *newnode, T *oldnode =   nullptr);

		// Could possibly implement an InsertBefore that operates on a whole list

		// Insert after oldnode, if oldnode is null then insert at head
		virtual bool Insert_After(T   *newnode, T *oldnode = nullptr);

		// Could possibly implement an InsertAfter that operates on a whole list
		virtual bool Is_Empty(void)   const;      // True if list is empty
		virtual long Get_Count(void) const;    // Returns number of nodes in list
};


/**************************************************************************
 * SList<T>::Insert_Before -- Inserts entry prior to specified entry      *
 *                                                                        *
 * Inserts an entry prior to the specified entry in the  list.  If the    *
 * specified   entry is null, it will add it prior to the head of the     *
 * list.  Returns true if sucessfully added, false otherwise.             *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
bool SList<T>::Insert_Before(T *newnode, T   *oldnode)
{
	// if not adding anything then just skip the add.
	if (newnode == nullptr)
		return false;

	// if there is no head to the list then add it to head
	if (oldnode == nullptr || HeadNode == nullptr || HeadNode->Data() == oldnode) {
		return(Add_Head(newnode));
	}

	// now we need to walk the list in an attempt to add the
	//   node.  since we are a singlely linked list we need
	//   to stop one prior to the entry.
	SLNode<T> *cur;
	for (cur=HeadNode; cur->Next() && cur->Next()->Data() != oldnode; cur=cur->Next()) {}

	// Verify that we found the entry as it might not have been in the list.
	// Note: Cur will be valid because it wont be assigned unless Next is
	//  valid.
	if (cur->Next() != nullptr && cur->Next()->Data() == oldnode) {
		SLNode<T> *temp	= new SLNode<T> (newnode);
		temp->Set_Next(cur->Next());
		cur->Set_Next(temp);
		return(true);
	}
	return(false);
}


/**************************************************************************
 * SList<T>::Insert_After -- Inserts an entry after specified entry       *
 *                                                                        *
 * Inserts an entry after to the specified entry in the list.  If the     *
 * specified entry is null, it will add it prior to the head of the list. *
 * Returns true if sucessfully added, false otherwise.                    *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
bool SList<T>::Insert_After(T *newnode, T *oldnode)
{
	if (newnode == nullptr)
		return false;

	if (oldnode == nullptr || HeadNode == nullptr)  {
		return(Add_Head(newnode));
	}

	// Search for oldnode in list
	SLNode<T> *cur;
	for (cur = HeadNode; cur && cur->Data() != oldnode; cur = cur->Next()) {}

	// Did we find the data we want to insert after?
	if (cur != nullptr  && cur->Data() == oldnode) {
		if (cur == TailNode) {        // Inserting after tail
			return(Add_Tail(newnode));
		}

		SLNode<T> *temp		= new SLNode<T>(newnode);
		temp->Set_Next(cur->Next());
		cur->Set_Next(temp);
		return true;
	}
	return false;
}
/**************************************************************************
 * SList<T>::Remove_All -- Removes all of the entries in the current list *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
void SList<T>::Remove_All(void)
{
	SLNode<T> *next;
	for (SLNode<T> *cur = HeadNode; cur; cur = next) {
		next = cur->Next();
		delete cur;
	}
	HeadNode = TailNode = nullptr;
}

/**************************************************************************
 * SList<T>::Remove -- Removes an element in the list.                    *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:  true if element could be removed, false if it could not be    *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
bool SList<T>::Remove(T *element)
{
	// if not adding anything then just skip the add.
	if (element == nullptr || HeadNode == nullptr)
		return false;

	// if the head is the element in question remove it
	if (HeadNode->Data() == element) {
		return(Remove_Head() != nullptr ? true : false);
	}

	// now we need to walk the list in an attempt to add the
	//   node.  since we are a singlely linked list we need
	//   to stop one prior to the entry.
	SLNode<T> *cur;
	for (cur = HeadNode; cur->Next() && cur->Next()->Data() != element; cur=cur->Next()) { }

	// Verify that we found the entry as it might not have been in the list.
	// Note: Cur will be valid because it wont be assigned unless Next is
	//  valid.
	if (cur->Next() != nullptr && cur->Next()->Data() == element) {
		SLNode<T> *temp	= cur->Next();
		cur->Set_Next(temp->Next());
		if (temp == TailNode) TailNode = cur;
		delete temp;
		return true;
	}
	return(false);
}

/**************************************************************************
 * *SList<T>::Remove_Head -- Removes the head of the list                 *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
T *SList<T>::Remove_Head(void)
{
	if (HeadNode == nullptr)      // Should make an assertion here instead!
		return ((T* )nullptr);

	SLNode<T> *temp = HeadNode;
	HeadNode = HeadNode->Next();

	if (HeadNode == nullptr)     // Do we have empty list now?
		TailNode = nullptr;

	T *data = temp->Data();
	delete temp;
	return data;
}

//
// Removes the tail of the list and returns a data pointer to the
// element removed.  Warning!  On a singlely linked list it is
// slow as hell to remove a tail!  If you need to frequently remove
// tails, then you should consider a doubly linked list.
//

/**************************************************************************
 * *SList<T>::Remove_Tail -- Removes the tail element from the list       *
 *                                                                        *
 * INPUT:		none																		  *
 *                                                                        *
 * OUTPUT:		returns a pointer to the element removed						  *
 *                                                                        *
 * WARNINGS:	On a singlely linked list it is slow as hell to remove a   *
 *					tail!  If you need to frequently remove tails, then you 	  *
 *					should consider a doubly linked list.                      *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
T *SList<T>::Remove_Tail(void)
{
	if (HeadNode == nullptr)     // Should make an assertion here instead!
		return ((T *)nullptr);

	T* data = TailNode->Data();
	return (Remove(data) ? data : (T*)nullptr);
}


/**************************************************************************
 * SList<T>::Get_Count -- Returns a count of the entries in the list      *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
inline long SList<T>::Get_Count(void) const
{
	long count = 0;
	for (SLNode<T> *cur = HeadNode; cur; cur = cur->Next())
		count++;
	return count;
}


/**************************************************************************
 * *SList<T>::Head -- Returns the head node of the list                   *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
inline SLNode<T> *SList<T>::Head(void) const
{
	return(HeadNode);
}

/**************************************************************************
 * *SList<T>::Tail -- Returns the tail node of the list                   *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
inline SLNode<T> *SList<T>::Tail(void) const
{
	return(TailNode);
}

/**************************************************************************
 * SList<T>::Is_Empty -- Returns true if the list is empty                *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
inline bool SList<T>::Is_Empty(void) const
{
	return( HeadNode == nullptr ? true : false);
}


/**************************************************************************
 * SList<T>::Add_Head -- Adds a node to the head of the list              *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
bool SList<T>::Add_Head(T *data)
{
	// we don't deal with lists that point to nothing
	if (!data) return false;

	SLNode<T> *temp			= new SLNode<T>(data);
	temp->Set_Next(HeadNode);
	HeadNode						= temp;
	if (!TailNode) TailNode	= temp;
	return true;
}


/**************************************************************************
 * SList<T>::Add_Head -- Adds a list to to the head of the list           *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
bool SList<T>::Add_Head(SList<T>& list)
{
	if (list.HeadNode == nullptr) return false;

	// Save point for initial add of element.
	SLNode<T> *addpoint = nullptr;

	// We traverse list backwards so nodes are added in right order.
	for (SLNode<T> *cur = list.HeadNode; cur; cur = cur->Next())
	if (addpoint) {
		SLNode<T> *temp   = new SLNode<T>(cur->Data());
		temp->Set_Next(addpoint->Next());
		addpoint->Set_Next(temp);
		addpoint = temp;
	} else {
		Add_Head(cur->Data());
		addpoint = HeadNode;
	}
	return true;
}

/**************************************************************************
 * SList<T>::Add_Tail -- Adds a node to the tail of the list              *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
bool SList<T>::Add_Tail(T *data)
{
	if (data == nullptr) return false;

	SLNode<T> *temp = new SLNode<T> (data);

	if (HeadNode == nullptr) {				// empty list
		HeadNode = TailNode	= temp;
	} else {									// non-empty list
		TailNode->Set_Next(temp);
		TailNode					= temp;
	}
	return true;
}

/**************************************************************************
 * SList<T>::Add_Tail -- Adds a list to the tail of the list              *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   03/11/1997 PWG : Created.                                            *
 *========================================================================*/
template<class T>
bool SList<T>::Add_Tail(SList<T>& list)
{
	if (list.HeadNode == nullptr) return false;

	for (SLNode<T> *cur = list.HeadNode; cur; cur = cur->Next())
		Add_Tail(cur->Data());
	return true;
}


/**************************************************************************
 * *SList<T>::Find_Node -- returns first node in list matching the input  *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   08/22/1997 GH  : Created.                                            *
 *========================================================================*/
template<class T>
inline SLNode<T> *SList<T>::Find_Node(T * data) const
{
	SLNode<T> * cur;

	for (cur = HeadNode; cur && cur->Data() != data; cur = cur->Next());

	return cur;
}
