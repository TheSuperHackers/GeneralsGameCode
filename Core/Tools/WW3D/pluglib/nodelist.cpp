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

/* $Header: /Commando/Code/Tools/pluglib/nodelist.cpp 8     1/02/01 6:31p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G                                                 *
 *                                                                                             *
 *                    File Name : NODELIST.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 06/09/97                                                     *
 *                                                                                             *
 *                  Last Update : June 9, 1997 [GH]                                            *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   INodeListClass::INodeListClass -- Create an INodeList                                     *
 *   INodeListClass::~INodeListClass -- Delete the INode List                                  *
 *   INode * INodeListClass::operator[] -- Array-like access to the list members               *
 *   INodeListClass::callback -- callback function for MAX                                     *
 *   INodeListClass::INodeListClass -- A "copy" contstructor with filtering...                 *
 *   INodeListClass::INodeListClass -- constructor                                             *
 *   INodeListClass::INodeListClass -- Constructor                                             *
 *   INodeListClass::Insert -- insert a list of nodes into this list                           *
 *   INodeListClass::Insert -- Inserts an INode into the list                                  *
 *   INodeListClass::Add_Tree -- Add a tree of INodes to the list                              *
 *   INodeListClass::Remove -- Remove the i'th element of the list                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "nodelist.h"


static AnyINodeFilter				_AnyFilter;


/*******************************************************************************
*	ListEntryClass
*
*	Used to implement a linked list of INodes.
*
*******************************************************************************/
class INodeListEntryClass
{
public:

	INodeListEntryClass(INode * n,TimeValue /*time*/) { Node = n; }
	~INodeListEntryClass(void) {}

	INode							* Node;
	INodeListEntryClass		* Next;
};

/***********************************************************************************************
 * INodeListClass::INodeListClass -- Constructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
INodeListClass::INodeListClass(TimeValue time,INodeFilterClass * inodefilter) :
	NumNodes(0),
	Time(time),
	ListHead(NULL),
	INodeFilter(inodefilter)
{
	if (INodeFilter == NULL) {
		INodeFilter = &_AnyFilter;
	}
}

/***********************************************************************************************
 * INodeListClass::INodeListClass -- Create an INodeList                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 * scene - 3dsMAX scene to enumerate																			  *
 * time - time at which to create the list of INodes														  *
 * inodefilter - object which will accept or reject each INode in the scene						  *
 * 																														  *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
INodeListClass::INodeListClass(IScene * scene,TimeValue time,INodeFilterClass * inodefilter) :
	NumNodes(0),
	Time(time),
	ListHead(NULL),
	INodeFilter(inodefilter)
{
	if (INodeFilter == NULL) {
		INodeFilter = &_AnyFilter;
	}
	scene->EnumTree(this);
}


/***********************************************************************************************
 * INodeListClass::INodeListClass -- Constructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/13/98    GTH : Created.                                                                 *
 *=============================================================================================*/
INodeListClass::INodeListClass(INode * root,TimeValue time,INodeFilterClass * nodefilter) :
	NumNodes(0),
	Time(time),
	ListHead(NULL),
	INodeFilter(nodefilter)
{
	if (INodeFilter == NULL) {
		INodeFilter = &_AnyFilter;
	}
	Add_Tree(root);
}


/***********************************************************************************************
 * INodeListClass::INodeListClass -- A "copy" contstructor with filtering...                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
INodeListClass::INodeListClass(INodeListClass & copyfrom,TimeValue time,INodeFilterClass * inodefilter) :
	NumNodes(0),
	Time(time),
	ListHead(NULL),
	INodeFilter(inodefilter)
{
	if (INodeFilter == NULL) {
		INodeFilter = &_AnyFilter;
	}
	for (unsigned i=0; i<copyfrom.Num_Nodes(); i++) {
		Insert(copyfrom[i]);
	}
}

/***********************************************************************************************
 * INodeListClass::~INodeListClass -- Delete the INode List                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
INodeListClass::~INodeListClass(void)
{
	while (ListHead)
	{
		INodeListEntryClass * next = ListHead->Next;
		delete ListHead;
		ListHead = next;
	}

	NumNodes = 0;
	ListHead = NULL;
}


/***********************************************************************************************
 * INode * INodeListClass::operator[] -- Array-like access to the list members                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 * 	index - index of the list entry																			  *
 * 																														  *
 * OUTPUT:                                                                                     *
 * 	pointer to an INode																							  *
 * 																														  *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
INode * INodeListClass::operator[] ( int index ) const
{
	INodeListEntryClass * entry = ListHead;
	while (index > 0 && entry != NULL )
	{
		entry = entry->Next;
		index--;
	}
	return entry->Node;
}


/***********************************************************************************************
 * INodeListClass::Insert -- insert a list of nodes into this list                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/14/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void INodeListClass::Insert(INodeListClass & insertlist)
{
	for (unsigned int i=0; i<insertlist.Num_Nodes(); i++) {
		Insert(insertlist[i]);
	}
}

/***********************************************************************************************
 * INodeListClass::Insert -- Inserts an INode into the list                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/02/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void INodeListClass::Insert(INode * node)
{
	if (INodeFilter->Accept_Node(node,Time))
	{
		INodeListEntryClass * newentry = new INodeListEntryClass(node, Time);
		newentry->Next = ListHead;
		ListHead = newentry;
		NumNodes++;
	}
}


/***********************************************************************************************
 * INodeListClass::Remove -- Remove the i'th element of the list                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/27/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void INodeListClass::Remove(int i)
{
	if ((i < 0) || (i > Num_Nodes())) {
		return;
	}

	INodeListEntryClass * prev = ListHead;
	while (i > 1) {
		prev = prev->Next;
	}

	INodeListEntryClass * deleteme = prev->Next;
	if (deleteme != NULL) {
		prev->Next = prev->Next->Next;
		delete deleteme;
	}
}


/***********************************************************************************************
 * INodeListClass::Add_Tree -- Add a tree of INodes to the list                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/13/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void INodeListClass::Add_Tree(INode * root)
{
	if (root == NULL) return;

	Insert(root);
	for (int i=0; i<root->NumberOfChildren(); i++) {
		Add_Tree(root->GetChildNode(i));
	}
}


/***********************************************************************************************
 * INodeListClass::callback -- callback function for MAX                                       *
 *                                                                                             *
 * 3dsMAX calls this function with a pointer to each INode in the scene.  We keep a pointer	  *
 * to the ones we like.																								  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int INodeListClass::callback(INode * node)
{
	Insert(node);

	return TREE_CONTINUE;	// Keep on enumerating....
}


void INodeListClass::Sort(const INodeCompareClass & node_compare)
{
	for (unsigned int i=0; i<Num_Nodes(); i++) {
		for (unsigned int j=0; j<Num_Nodes(); j++) {

			INodeListEntryClass * ni = get_nth_item(i);
			INodeListEntryClass * nj = get_nth_item(j);

			if (node_compare(ni->Node,nj->Node) > 0) {
				INode * tmp = ni->Node;
				ni->Node = nj->Node;
				nj->Node = tmp;
			}
		}
	}
}

INodeListEntryClass * INodeListClass::get_nth_item(int index)
{
	INodeListEntryClass * entry = ListHead;
	while (index > 0 && entry != NULL )
	{
		entry = entry->Next;
		index--;
	}
	return entry;
}

