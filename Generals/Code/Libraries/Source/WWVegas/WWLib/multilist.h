/*
**	Command & Conquer Generals(tm)
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
 *                 Project Name : WWLib                                                        *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/multilist.h                            $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/09/01 8:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef MULTILIST_H
#define MULTILIST_H

#include "always.h"
#include "mempool.h"
#include <assert.h>

class MultiListNodeClass;
class GenericMultiListClass;


/******************************************************************************
	
	MultiLists

	MultiLists solve the problem of needing to have objects that can be placed
	into multiple lists at the same time.  MultiListNodes are all allocated from
	a pool which grows in chunks to avoid overhead from calling new and delete.
	All operations such as insertion, removal, checking if a list contains an
	object, are ether immediate or O(numlists) where numlists is the number of
	different lists that the object in question currently occupies.

******************************************************************************/


/**
** MultiListObjectClass
** This is an object that can be linked into a MultiList.  The only overhead
** for this is a single pointer to a MultiListNode.
** Objects that are linked into MulitLists must derive from this class.  
** If you delete an instance of one of these objects while it is in one or more
** Multi-Lists, it will automatically remove itself from the lists.
*/
class MultiListObjectClass 
{
public:

	MultiListObjectClass(void) : ListNode(NULL)								{ }
	virtual ~MultiListObjectClass(void);

	MultiListNodeClass *		Get_List_Node() const							{ return ListNode; }
	void							Set_List_Node(MultiListNodeClass *node)	{ ListNode = node; }

private:
	MultiListNodeClass *		ListNode;
};


/**
** MultiListNodeClass
** These nodes allow objects to be linked in multiple lists.  It is
** like a 2-D linked list where one dimension is the list of objects in a 
** given list and the other dimension is the list of lists that a given object
** is in.
*/
class MultiListNodeClass : public AutoPoolClass<MultiListNodeClass, 256>
{
public:
	MultiListNodeClass(void) { Prev = Next = NextList = 0; Object = 0; List = 0; }

	MultiListNodeClass		*Prev;					// prev object in list
	MultiListNodeClass		*Next;					// next object in list
	MultiListNodeClass		*NextList;				// next list this object is in
	MultiListObjectClass		*Object;					// pointer back to the object
	GenericMultiListClass	*List;					// pointer to list for this node
};


/**
** GenericMultiListClass
** This is an internal implementation class for Multi List.  It is not meant to be used as-is
** Instead use the MultiListClass template which performs type-casting and reference counting
** on the objects that you add to the list.
** This simply contains the head node for a list.  This is a doubly circularly linked list where
** our head node is a sentry.  To easily iterate the list use the iterator defined below.
*/
class GenericMultiListClass 
{
public:

	GenericMultiListClass(void)	{ Head.Next = Head.Prev = &Head; Head.Object = 0; Head.NextList = 0; }
	virtual ~GenericMultiListClass(void);
	
	bool							Is_In_List(MultiListObjectClass *obj);
	bool							Contains(MultiListObjectClass * obj);
	bool							Is_Empty(void);
	int							Count(void);

protected:
	
	bool							Internal_Add(MultiListObjectClass *obj,bool onlyonce = true);
	bool							Internal_Add_Tail(MultiListObjectClass * obj,bool onlyonce = true);
	bool							Internal_Add_After(MultiListObjectClass * obj,const MultiListObjectClass * existing_list_member,bool onlyonce = true);
	bool							Internal_Remove(MultiListObjectClass *obj);

	MultiListObjectClass	*	Internal_Get_List_Head(void);
	MultiListObjectClass	*	Internal_Remove_List_Head(void);

private:

	MultiListNodeClass		Head;
	friend class				GenericMultiListIterator;
	friend class				MultiListObjectClass;
};

inline bool GenericMultiListClass::Is_In_List(MultiListObjectClass * obj)
{
	return Contains(obj);
}

inline bool GenericMultiListClass::Is_Empty(void)
{
	return (Head.Next == &Head);
}

inline MultiListObjectClass * GenericMultiListClass::Internal_Get_List_Head(void)
{
	if (Head.Next == &Head) {
		return 0;					// no more objects
	} else {
		assert(Head.Next->Object != 0);
		return Head.Next->Object;
	}
}



/**
** GenericMultiListIterator
** This is the internal implementation of an iterator for a MultiList.  The user should
** use the templated MultiListIterator which will do typecasting and proper reference 
** counting rather than this class.
*/
class GenericMultiListIterator
{
public:
	GenericMultiListIterator(GenericMultiListClass *list)	{ assert(list); First(list); }

	void				First(GenericMultiListClass *list)		{ List = list; CurNode = List->Head.Next; }
	void				First(void)										{ CurNode = List->Head.Next; }
	void				Last(GenericMultiListClass *list)		{ List = list; CurNode = List->Head.Prev; }
	void				Last(void)										{ CurNode = List->Head.Prev; }

	void				Next(void)										{ CurNode = CurNode->Next; }
	void				Prev(void)										{ CurNode = CurNode->Prev; }
	bool				Is_Done(void)									{ return (CurNode == &(List->Head)); }
	
protected:
	
	MultiListObjectClass	*		Current_Object(void)			{ return CurNode->Object; }

	GenericMultiListClass *		List;				// list we're working in
	MultiListNodeClass *			CurNode;			// node we're currently at.

};



/**************************************************************************************

  MultiListClass, MultiListIterator

  These templates inherits from GenericMultiListClass and GenericMultiListIterator
  and handle all typecasting to your object type.  These classes require that
  your 'ObjectType' be derived from MultiListObjectClass.

**************************************************************************************/

/**
** MultiListClass
** This is a template derived from GenericMultiListClass which allows you
** to create specialized lists.  All this template does is perform type-checking
** and internal type-casting of all pointers to your object type.  Your
** object must be derived from MultiListObjectClass in order to be used
** with this template.
*/
template <class ObjectType>
class MultiListClass : public GenericMultiListClass
{
public:

	MultiListClass(void) { }
			
	virtual ~MultiListClass(void)
	{
		while (!Is_Empty()) {
			Remove_Head();
		}
	}

	bool				Add(ObjectType * obj,bool onlyonce = true)
	{
		return Internal_Add(obj,onlyonce);
	}	

	bool				Add_Tail(ObjectType * obj,bool onlyonce = true)
	{
		return Internal_Add_Tail(obj,onlyonce);
	}

	bool				Add_After(ObjectType * obj,const ObjectType * existing_list_member,bool onlyonce = true)
	{
		return Internal_Add_After(obj,existing_list_member,onlyonce);
	}

	bool				Remove(ObjectType *obj)
	{
		return Internal_Remove(obj);
	}

	ObjectType *	Get_Head()
	{
		return ((ObjectType*)Internal_Get_List_Head());
	}

	ObjectType *	Peek_Head()
	{
		return ((ObjectType*)Internal_Get_List_Head());
	}

	ObjectType *	Remove_Head()
	{
		return ((ObjectType*)Internal_Remove_List_Head());
	}

	void				Reset_List()
	{
		while (Get_Head() != NULL) {
			Remove_Head();
		}
	}

private:

	// not implemented
	MultiListClass(const MultiListClass & that);
	MultiListClass & operator = (const MultiListClass & that);
	
};

/**
** MultiListIterator
** This is a template derived from GenericMultiListIterator which allows you
** to iterate through specialized MultiListClass's.  Again, it basically
** just type-casts all of the pointers for you (hopefully compiles away to
** nothing...)
**
** WARNING: If you need to remove an object from a MultiList while you are iterating, use the
** Remove_Current_Object function (don't modify the list directly while iterating it).
*/
template <class ObjectType>
class MultiListIterator : public GenericMultiListIterator
{
protected:
	using GenericMultiListIterator::CurNode;
public:
	using GenericMultiListIterator::First;

public:

	MultiListIterator(MultiListClass<ObjectType> *list) : GenericMultiListIterator(list)	{}

	ObjectType *	Get_Obj(void)
	{
		return (ObjectType*)Current_Object();
	}

	ObjectType *	Peek_Obj(void)
	{
		return (ObjectType*)Current_Object();
	}

	void				Remove_Current_Object(void)
	{
		ObjectType * obj = Peek_Obj();
		if (obj != NULL) {
			Next();
			((MultiListClass<ObjectType> *)List)->Remove(obj);
		}
	}

};


/**************************************************************************************

  RefMultiListClass, RefMultiListIterator

  These templates inherits from GenericMultiListClass and GenericMultiListIterator
  and handle all typecasting to your object type and also do proper reference tracking.
  These classes require that your 'ObjectType' be derived from MultiListObjectClass and
  RefCountClass.

**************************************************************************************/

/**
** RefMultiListClass
** This is a template derived from GenericMultiListClass which handles ref-counted
** objects.  It assumes that 'ObjectType' is derived from MultiListObjectClass and
** RefCountClass.   It adds type-checking and reference counting to 
** GenericMultiListClass.
*/
template <class ObjectType>
class RefMultiListClass : public GenericMultiListClass
{
public:

	virtual			~RefMultiListClass(void)
	{
		while (!Is_Empty()) {
			Release_Head();
		}
	}

	bool				Add(ObjectType * obj,bool onlyonce = true)
	{
		// if we add the object from our list, add a reference to it
		bool result = Internal_Add(obj,onlyonce);
		if (result == true) {
			obj->Add_Ref();
		}
		return result;
	}	

	bool				Add_Tail(ObjectType * obj,bool onlyonce = true)
	{
		// if we add the object from our list, add a reference to it
		bool result = Internal_Add_Tail(obj,onlyonce);
		if (result == true) {
			obj->Add_Ref();
		}
		return result;
	}

	bool				Add_After(ObjectType * obj,const ObjectType * existing_list_member,bool onlyonce = true)
	{
		// if we add the object from our list, add a reference to it
		bool result = Internal_Add_After(obj,existing_list_member,onlyonce);
		if (result == true) {
			obj->Add_Ref();
		}
		return result;
	}

	bool				Remove(ObjectType *obj)
	{
		// if we remove the object from our list, release our reference to it
		bool result = Internal_Remove(obj);
		if (result) {
			obj->Release_Ref();
		}
		return result;
	}

	bool				Release_Head(void)
	{
		// remove the head from the list and release our reference to it
		ObjectType * obj = ((ObjectType*)Internal_Remove_List_Head());
		if (obj) {
			obj->Release_Ref();
			return true;
		} else {
			return false;
		}
	}

	ObjectType *	Get_Head()
	{
		// if we have a head, add a reference for the caller of this function
		ObjectType * obj = ((ObjectType*)Internal_Get_List_Head());
		if (obj) {
			obj->Add_Ref();
		}
		return obj;
	}

	ObjectType *	Peek_Head()
	{
		// no need to add-ref since the caller is 'peek'ing
		return ((ObjectType*)Internal_Get_List_Head());
	}

	ObjectType *	Remove_Head()
	{
		// our reference is transferred to the caller of this function
		return ((ObjectType*)Internal_Remove_List_Head());
	}

	void				Reset_List()
	{
		while (Peek_Head() != NULL) {
			Release_Head();
		}
	}
};

/**
** RefMultiListIterator
** This is a template derived from GenericMultiListIterator which can iterate
** through RefMultiListClass's.  It adds type-checking and reference counting
** to GenericMultiListIterator.
**
** WARNING: If you need to remove an object from a MultiList while you are iterating, use the
** Remove_Current_Object function (don't modify the list directly while iterating it).  Also
** note that this function will cause the list to release its reference to the object.
*/
template <class ObjectType>
class RefMultiListIterator : public GenericMultiListIterator
{
public:

	RefMultiListIterator(RefMultiListClass<ObjectType> *list) : GenericMultiListIterator(list) {}

	ObjectType *	Get_Obj(void)
	{
		ObjectType * obj = (ObjectType*)Current_Object();
		if (obj != NULL) {
			obj->Add_Ref();
		}
		return obj;
	}

	ObjectType *	Peek_Obj(void)
	{
		return ((ObjectType*)Current_Object());
	}

	void				Remove_Current_Object(void)
	{
		ObjectType * obj = Peek_Obj();
		if (obj != NULL) {
			Next();
			((RefMultiListClass<ObjectType> *)List)->Remove(obj);
		}
	}
};



/**************************************************************************************

  PriorityMultiListIterator

  This template inherits from MultiListIterator and is used to implement a simple
  priority queue where nodes are popped off the head of the list and added to the tail.

**************************************************************************************/

template <class ObjectType>
class PriorityMultiListIterator : public MultiListIterator<ObjectType>
{
protected:
	using MultiListIterator<ObjectType>::CurNode;
public:
	using MultiListIterator<ObjectType>::First;
	using MultiListIterator<ObjectType>::Remove_Current_Object;

public:
	PriorityMultiListIterator(MultiListClass<ObjectType> *list)
		:	OriginalHead (NULL),
			MultiListIterator<ObjectType>(list)			{ First (); }

	bool
	Process_Head (ObjectType **object)
	{
		bool retval = false;

		//	Check to ensure we don't wrap around the list (stop after iterating
		// the list once).
		if (CurNode != NULL && CurNode->Object != NULL && OriginalHead != CurNode) {
			OriginalHead		= (OriginalHead == NULL) ? CurNode : OriginalHead;
			(*object)			= (ObjectType *)CurNode->Object;


			// Remove the node from the head of the list and
			// add it to the tail of the list
			Remove_Current_Object();
			((MultiListClass<ObjectType> *)PriorityMultiListIterator::List)->Add_Tail ((*object));

			retval = true;
		}

		return retval;
	}
	
protected:
	
	MultiListNodeClass *		OriginalHead;
};


#endif //LIST_CLASS_H

