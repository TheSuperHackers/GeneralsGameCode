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
 *                 Project Name : WWLib                                                        *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/multilist.cpp                          $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/12/01 9:51a                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "multilist.h"
#include "wwmemlog.h"

/*
** Declare the pool for ListNodes
*/
DEFINE_AUTO_POOL(MultiListNodeClass, 256);


/***********************************************************************************************

  MultiListObjectClass Implementation.

***********************************************************************************************/

MultiListObjectClass::~MultiListObjectClass(void)
{
	while (ListNode) {
		ListNode->List->Internal_Remove(this);
	}
}


/***********************************************************************************************

  GenericMultiListClass Implementation.
  This class has all of the real implementation.  Templates are provided in the header file
  which create typed lists by simply type-casting the void pointers that GenericMultiList uses.

***********************************************************************************************/

GenericMultiListClass::~GenericMultiListClass(void)
{
	assert(Is_Empty());
}

bool GenericMultiListClass::Contains(MultiListObjectClass * obj)
{
	assert(obj);

	MultiListNodeClass* lnode = obj->Get_List_Node();
	while (lnode) {
		if (lnode->List == this) return true;
		lnode = lnode->NextList;
	}
	return false;
}

int GenericMultiListClass::Count(void)
{
	int counter = 0;
	GenericMultiListIterator it(this);
	for (it.First(); !it.Is_Done(); it.Next()) {
		counter++;
	}
	return counter;
}

bool GenericMultiListClass::Internal_Add(MultiListObjectClass *obj, bool onlyonce)
{
	WWMEMLOG(MEM_GAMEDATA);
	assert(obj);

	if (onlyonce && Is_In_List(obj)) {
		return false;
	}

	// allocate a list node for this object
	MultiListNodeClass *node = new MultiListNodeClass;
	node->Object = obj;

	// link the list node into the list of list nodes for the object
	node->NextList = obj->Get_List_Node();
	obj->Set_List_Node(node);

	// link the node to the objects in *this* list
	node->Prev = &(Head);
	node->Next = Head.Next;
	node->Next->Prev = node;
	node->Prev->Next = node;

	// identify this node as being part of this list
	node->List = this;

	return true;
}

bool GenericMultiListClass::Internal_Add_Tail(MultiListObjectClass * obj,bool onlyonce)
{
	WWMEMLOG(MEM_GAMEDATA);
	assert(obj);

	if (onlyonce && Is_In_List(obj)) {
		return false;
	}

	// allocate a list node for this object
	MultiListNodeClass *node = new MultiListNodeClass;
	node->Object = obj;

	// link the list node into the list of list nodes for the object
	node->NextList = obj->Get_List_Node();
	obj->Set_List_Node(node);

	// link the node to the tail of this list (next should be head, prev should be head.Prev)
	node->Prev = Head.Prev;
	node->Next = &(Head);
	node->Next->Prev = node;
	node->Prev->Next = node;

	// identify this node as being part of this list
	node->List = this;

	return true;
}

bool GenericMultiListClass::Internal_Add_After(MultiListObjectClass * obj,const MultiListObjectClass * existing_list_member,bool onlyonce)
{
	WWMEMLOG(MEM_GAMEDATA);
	assert(obj);
	assert(existing_list_member);

	if (onlyonce && Is_In_List(obj)) {
		return false;
	}

	// find the node hanging off 'existing_list_member' that corresponds to this list (O(numlists))
	MultiListNodeClass * existing_node = existing_list_member->Get_List_Node();
	while ((existing_node->List != this) && (existing_node)) {
		existing_node = existing_node->NextList;
	}

	if (existing_node == nullptr) {
		return false;	// he's not in this list!
	}

	// allocate a node
	MultiListNodeClass * node = new MultiListNodeClass;
	node->Object = obj;

	// link the node into the list of list nodes for the object
	node->NextList = obj->Get_List_Node();
	obj->Set_List_Node(node);

	// now, link the new node after existing_node
	node->Prev = existing_node;
	node->Next = existing_node->Next;
	node->Next->Prev = node;
	node->Prev->Next = node;
	node->List = this;

	return true;
}

bool GenericMultiListClass::Internal_Remove(MultiListObjectClass *obj)
{
	// find the list node in this object that belongs to this list
	MultiListNodeClass * lnode = obj->Get_List_Node();
	MultiListNodeClass * prevlnode = nullptr;

	while ((lnode) && (lnode->List != this)) {
		prevlnode = lnode;
		lnode = lnode->NextList;
	}

	if (lnode == nullptr) {
		return false;
	}

	// now we've found the node which corresponds to this list,
	// unlink from the list of objects
	lnode->Prev->Next = lnode->Next;
	lnode->Next->Prev = lnode->Prev;

	// unlink from the list of list nodes
	if (prevlnode) {
		prevlnode->NextList = lnode->NextList;
	} else {
		assert(obj->Get_List_Node() == lnode);	// must be first list obj is in...
		obj->Set_List_Node(lnode->NextList);
	}

	// delete the link
	delete lnode;

	return true;
}

MultiListObjectClass * GenericMultiListClass::Internal_Remove_List_Head(void)
{
	if (Head.Next == &Head) {
		return nullptr;					// no more objects
	}

	MultiListNodeClass * node = Head.Next;
	MultiListObjectClass * obj = node->Object;

	// remove the object from our list
	Internal_Remove(obj);

	// here you go.
	return obj;
}




