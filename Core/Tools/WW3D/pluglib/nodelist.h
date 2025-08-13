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

/* $Header: /Commando/Code/Tools/pluglib/nodelist.h 7     1/02/01 6:31p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G                                                 *
 *                                                                                             *
 *                    File Name : NODELIST.H                                                   *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 06/09/97                                                     *
 *                                                                                             *
 *                  Last Update : June 9, 1997 [GH]                                            *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef NODELIST_H
#define NODELIST_H

#include "always.h"
#include <max.h>

#ifndef NODEFILT_H
#include "nodefilt.h"
#endif


class INodeListEntryClass;
class INodeCompareClass;


/*******************************************************************************
*	INodeListClass
*
*	This is a class that can enumerate a 3dsMax scene and build a list of
*	all of the INodes that meet your desired criteria.
*
*******************************************************************************/
class INodeListClass : public ITreeEnumProc
{
public:

	INodeListClass(TimeValue time,INodeFilterClass * nodefilter = NULL);
	INodeListClass(IScene * scene,TimeValue time,INodeFilterClass * nodefilter = NULL);
	INodeListClass(INode * root,TimeValue time,INodeFilterClass * nodefilter = NULL);
	INodeListClass(INodeListClass & copyfrom,TimeValue time,INodeFilterClass * inodefilter = NULL);
	~INodeListClass();

	void			Set_Filter(INodeFilterClass * inodefilter) { INodeFilter = inodefilter; }
	void			Insert(INodeListClass & insertlist);
	void			Insert(INode * node);
	void			Remove(int i);
	unsigned		Num_Nodes(void) const { return NumNodes; }
	INode *		operator[] (int index) const;
	void			Sort(const INodeCompareClass & node_compare);
	void			Add_Tree(INode * root);

private:

	unsigned						NumNodes;
	TimeValue					Time;
	INodeListEntryClass *	ListHead;
	INodeFilterClass *		INodeFilter;

	INodeListEntryClass * get_nth_item(int index);
	int callback(INode * node);
};


class INodeCompareClass
{
public:
	// returns <0 if nodea < node b.
	// returns =0 if nodea = node b.
	// returns >0 if nodea > node b.
	virtual int operator() (INode * nodea,INode * nodeb) const = 0;
};


#endif /*NODELIST_H*/
