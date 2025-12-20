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

/* $Header: /Commando/Code/Tools/max2w3d/nodefilt.h 6     1/14/98 10:23a Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G                                                 *
 *                                                                                             *
 *                    File Name : NODEFILT.h                                                   *
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

#pragma once

#include "always.h"
#include <max.h>

/***************************************************************
*
*	INodeFilterClass
*
*	This is simply an object used to accept or reject INodes
*	based on whatever criteria you desire.  There are some
*	default node filters defined in this module or you can
*	create your own by inheriting the Abstract Base Class
*	INodeFilterClass and implementing the Accept_Node method.
*
***************************************************************/
class INodeFilterClass
{
public:
	virtual BOOL Accept_Node(INode * node, TimeValue time) = 0;
};


/***************************************************************
*
*	AnyINodeFilter
*
*	Accepts all INodes...
*
***************************************************************/
class AnyINodeFilter	: public INodeFilterClass
{
public:
	virtual BOOL Accept_Node(INode * node, TimeValue time) { return TRUE; }
};


/***************************************************************
*
*	HelperINodeFilter
*
*	Accepts INodes which are Helper objects
*
***************************************************************/
class HelperINodeFilter : public INodeFilterClass
{
public:
	virtual BOOL Accept_Node(INode * node, TimeValue time);
};


/***************************************************************
*
*	MeshINodeFilter
*
*	Only accepts INodes which are Triangle meshes
*
***************************************************************/
class MeshINodeFilter : public INodeFilterClass
{
public:
	virtual BOOL Accept_Node(INode * node, TimeValue time);
};

/***************************************************************
*
*	VisibleMeshINodeFilter
*
*	Only accepts INodes which are Triangle meshes and are
*	currently visible
*
***************************************************************/
class VisibleMeshINodeFilter : public INodeFilterClass
{
public:
	virtual BOOL Accept_Node(INode * node, TimeValue time);
};

/***************************************************************
*
*	VisibleHelperINodeFilter
*
*	Only accepts INodes which are Helper objects and are
*	currently visible
*
***************************************************************/
class VisibleHelperINodeFilter : public INodeFilterClass
{
public:
	virtual BOOL Accept_Node(INode * node, TimeValue time);
};


/***************************************************************
*
*	VisibleMeshOrHelperINodeFilter
*
*	Only accepts INodes which are Triangle meshes or helper
*	objects and are currently visible
*
***************************************************************/
class VisibleMeshOrHelperINodeFilter : public INodeFilterClass
{
public:
	virtual BOOL Accept_Node(INode * node, TimeValue time);
};


/***************************************************************
*
*	AnimatedINodeFilter
*
*	Only accepts INodes which contain at least on animation
*	key.
*
***************************************************************/
class AnimatedINodeFilter : public INodeFilterClass
{
public:
	virtual BOOL Accept_Node(INode * node, TimeValue time);
};


/***************************************************************
*
*	VisibleSelectedINodeFilter
*
*	Only accepts INodes which are Visible and Selected
*
***************************************************************/
class VisibleSelectedINodeFilter : public INodeFilterClass
{
public:
	virtual BOOL Accept_Node(INode * node, TimeValue time);
};
