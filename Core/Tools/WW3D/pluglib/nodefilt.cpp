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

/* $Header: /Commando/Code/Tools/max2w3d/nodefilt.cpp 9     1/16/98 10:34a Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G                                                 *
 *                                                                                             *
 *                    File Name : NODEFILT.cpp                                                 *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 06/09/97                                                     *
 *                                                                                             *
 *                  Last Update : June 9, 1997 [GH]                                            *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   VisibleMeshINodeFilter::Accept_Node -- Accepts visible meshes                             *
 *   AnimatedINodeFilter::Accept_Node -- Accepts animated INodes                               *
 *   RootINodeFilter::Accept_Node -- Accepts root INodes                                       *
 *   VisibleHelperINodeFilter::Accept_Node -- Accepts visible helper objects                   *
 *   VisibleMeshOrHelperINodeFilter::Accept_Node -- Accepts visible helper or mesh objects     *
 *   HelperINodeFilter::Accept_Node -- Accepts all helper inodes (including hidden)            *
 *   MeshINodeFilter::Accept_Node -- Accepts all mesh inodes (including hidden)                *
 *   VisibleSelectedINodeFilter::Accept_Node -- Accepts Visible and selected inodes            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "nodefilt.h"
#include <istdplug.h>


/*
**	The default node filter:
*/
VisibleMeshINodeFilter DefaultINodeFilter;


/***********************************************************************************************
 * HelperINodeFilter::Accept_Node -- Accepts all helper inodes (including hidden)              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
BOOL HelperINodeFilter::Accept_Node(INode * node,TimeValue time)
{
	Object * obj = node->EvalWorldState(time).obj;

	if (obj && obj->SuperClassID() == HELPER_CLASS_ID) {

		return TRUE;

	}

	return FALSE;
}


/***********************************************************************************************
 * MeshINodeFilter::Accept_Node -- Accepts all mesh inodes (including hidden)                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
BOOL MeshINodeFilter::Accept_Node(INode * node,TimeValue time)
{
	Object * obj = node->EvalWorldState(time).obj;

	if (obj &&
		obj->CanConvertToType(triObjectClassID) &&
		obj->SuperClassID() == GEOMOBJECT_CLASS_ID)
	{

		return TRUE;

	} else {

		return FALSE;

	}
}


/***********************************************************************************************
 * VisibleMeshINodeFilter::Accept_Node -- Accepts visible meshes                               *
 *                                                                                             *
 * Accepts nodes which: 																							  *
 * - can be converted to tri-meshes 																			  *
 * - are not hidden 																									  *
 * - whose visibility > 0.0																						  *
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
BOOL VisibleMeshINodeFilter::Accept_Node(INode * node, TimeValue time)
{
	Object * obj = node->EvalWorldState(time).obj;

	if
	(
		obj
		&& !node->IsHidden ()
		&& obj->CanConvertToType(triObjectClassID)
		&& obj->SuperClassID() == GEOMOBJECT_CLASS_ID
//		&& node->GetVisibility (time) > 0.0f
	)
	{

		return TRUE;

	} else {

		return FALSE;

	}
}

/***********************************************************************************************
 * VisibleHelperINodeFilter::Accept_Node -- Accepts visible helper objects                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
BOOL VisibleHelperINodeFilter::Accept_Node(INode * node, TimeValue time)
{
	Object * obj = node->EvalWorldState(time).obj;

	if ((!obj) || (node->IsHidden()) /*|| (node->GetVisibility(time) <= 0.0f)*/) {

		return FALSE;

	}

	if (obj->SuperClassID() == HELPER_CLASS_ID) {

		return TRUE;

	}

	return FALSE;
}


/***********************************************************************************************
 * VisibleMeshOrHelperINodeFilter::Accept_Node -- Accepts visible helper or mesh objects       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
BOOL VisibleMeshOrHelperINodeFilter::Accept_Node(INode * node, TimeValue time)
{
	Object * obj = node->EvalWorldState(time).obj;

	if ((!obj) || (node->IsHidden()) /*|| (node->GetVisibility(time) <= 0.0f)*/) {

		return FALSE;

	}

	if (obj->CanConvertToType(triObjectClassID) && obj->SuperClassID() == GEOMOBJECT_CLASS_ID) {

		return TRUE;

	}

	if (obj->SuperClassID() == HELPER_CLASS_ID) {

		return TRUE;

	}

	return FALSE;
}

/***********************************************************************************************
 * AnimatedINodeFilter::Accept_Node -- Accepts animated INodes                                 *
 *                                                                                             *
 * Accepts nodes which: 																							  *
 * - can be converted to tri-meshes 																			  *
 * - are not hidden 																									  *
 * - whose visibility > 0.0 																						  *
 * - have animation keys!																							  *
 * 																														  *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
BOOL AnimatedINodeFilter::Accept_Node(INode * node, TimeValue time)
{
	Object			* obj = node->EvalWorldState(time).obj;
	Control			* poscon = node->GetTMController()->GetPositionController();
	Control			* rotcon = node->GetTMController()->GetRotationController();

	int numkeys = 0;
	if (poscon != NULL) {
		IKeyControl	* poskeys = GetKeyControlInterface(poscon);
		if (poskeys != NULL) numkeys += poskeys->GetNumKeys();
	}

	if (rotcon != NULL) {
		IKeyControl	* rotkeys = GetKeyControlInterface(rotcon);
		if (rotkeys != NULL) numkeys += rotkeys->GetNumKeys();
	}

	if (obj && !node->IsHidden() && numkeys > 0) {
		return TRUE;
	}

	return FALSE;
}


/***********************************************************************************************
 * VisibleSelectedINodeFilter::Accept_Node -- Accepts Visible and selected inodes              *
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
BOOL VisibleSelectedINodeFilter::Accept_Node(INode * node, TimeValue time)
{
	if (!node->IsHidden() && node->Selected()) {
		return TRUE;
	} else {
		return FALSE;
	}
}

