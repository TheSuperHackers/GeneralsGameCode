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
 *                 Project Name : WWSaveLoad                                                   *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwsaveload/definitionclassids.h              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/03/00 5:44p                                               $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef __DEFINITION_CLASS_ID_H
#define __DEFINITION_CLASS_ID_H

#include "always.h"
#include "bittype.h"

//////////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
//////////////////////////////////////////////////////////////////////////////////
const int	DEF_CLASSID_START			= 0x00001000;
const int	DEF_CLASSID_RANGE			= 0x00001000;
#define		NEXT_SUPER_CLASSID(n)	DEF_CLASSID_START + (n * DEF_CLASSID_RANGE)


//////////////////////////////////////////////////////////////////////////////////
//
//	DefinitionClassID
//
//	Note:  The following enum should contain ALL of the class ids for
// definitions in the entire system (to guarantee they are unique).  Each
// super-class is allocated a range of class ids.  Use the
// SuperClassID_From_ClassID function to determine which super class
//	a particular class id belongs to.
//
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CLASSID_TERRAIN			= NEXT_SUPER_CLASSID(0),
	CLASSID_TILE				= NEXT_SUPER_CLASSID(1),
	CLASSID_GAME_OBJECTS		= NEXT_SUPER_CLASSID(2),
	CLASSID_LIGHT				= NEXT_SUPER_CLASSID(3),
	CLASSID_SOUND				= NEXT_SUPER_CLASSID(4),
	CLASSID_WAYPATH			= NEXT_SUPER_CLASSID(5),
	CLASSID_ZONE				= NEXT_SUPER_CLASSID(6),
	CLASSID_TRANSITION		= NEXT_SUPER_CLASSID(7),
	CLASSID_PHYSICS			= NEXT_SUPER_CLASSID(8),
	CLASSID_EDITOR_OBJECTS	= NEXT_SUPER_CLASSID(9),
	CLASSID_MUNITIONS			= NEXT_SUPER_CLASSID(10),
	CLASSID_DUMMY_OBJECTS	= NEXT_SUPER_CLASSID(11),
	CLASSID_BUILDINGS			= NEXT_SUPER_CLASSID(12),
	CLASSID_TWIDDLERS			= NEXT_SUPER_CLASSID(13),
	CLASSID_GLOBAL_SETTINGS	= NEXT_SUPER_CLASSID(14),
};


//////////////////////////////////////////////////////////////////////////////////
//
//	SuperClassID_From_ClassID
//
//////////////////////////////////////////////////////////////////////////////////
inline uint32
SuperClassID_From_ClassID (uint32 class_id)
{
	//
	//	Which id-range does it fall under?
	//
	int delta			= class_id - DEF_CLASSID_START;
	int num_ranges		= delta / DEF_CLASSID_RANGE;

	return DEF_CLASSID_START + (num_ranges * DEF_CLASSID_RANGE);
}


#endif //__DEFINITION_CLASS_ID_H

