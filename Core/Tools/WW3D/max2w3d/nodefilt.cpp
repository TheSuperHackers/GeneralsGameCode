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
 *   VisibleSelectedINodeFilter::Accept_Node -- Accepts Visible and selected inodes            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "nodefilt.h"


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

