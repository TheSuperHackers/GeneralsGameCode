/*
**	Command & Conquer Renegade(tm)
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
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/LODDefs.h        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/03/99 7:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef __LODDEFS_H
#define __LODDEFS_H


////////////////////////////////////////////////////////////////////////////
//
//	Data types
//
typedef enum
{
	TYPE_COMMANDO = 0,
	TYPE_G,
	TYPE_COUNT
} LOD_NAMING_TYPE;


////////////////////////////////////////////////////////////////////////////
//
//	Inline functions
//
__inline bool Is_LOD_Name_Valid (LPCTSTR name, LOD_NAMING_TYPE &type)
{
	bool retval = false;

	// Does this name fit with the format expected?
	LPCTSTR last_2_chars = name + (::lstrlen (name)-2);
	if (((last_2_chars[0] == 'L') || (last_2_chars[0] == 'l')) &&
		 ((last_2_chars[1] >= '0') && (last_2_chars[1] <= '9'))) {
		type = TYPE_COMMANDO;
		retval = true;
	} else if ((last_2_chars[1] >= 'a' && last_2_chars[1] <= 'z') ||
				  (last_2_chars[1] >= 'A' && last_2_chars[1] <= 'Z')) {
		type = TYPE_G;
		retval = true;
	}

	return retval;
}

__inline bool Is_Model_Part_of_LOD (LPCTSTR name, LPCTSTR base, LOD_NAMING_TYPE &type)
{
	// Assume failure
	bool retval = false;

	// Does the name start with the base?
	if (::strstr (name, base) == name) {

		// Get the remaining characters in the name (after the base)
		LPCTSTR extension = name + ::lstrlen (base);

		// What naming convention are we using?
		if (type == TYPE_COMMANDO) {
			if (((extension[0] == 'L') || (extension[0] == 'l')) &&
				 ((extension[1] >= '0') && (extension[1] <= '9'))) {
				retval = (extension[2] == 0);
			}
		} else if (type == TYPE_G) {
			if ((extension[0] >= 'a' && extension[0] <= 'z') ||
				 (extension[0] >= 'A' && extension[0] <= 'Z')) {
				retval = (extension[1] == 0);
			}
		}
	}

	// Return the true/false result ccde
	return retval;
}


#endif //__LODDEFS_H

