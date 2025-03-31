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
 *                 Project Name : WWSHADE                                                         *
 *                                                                                             *
 *                     $Archive:: wwshade/shdclassids.h                           $*
 *                                                                                             *
 *                  $Org Author:: Jani_p
 *																																	
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 8/01/02 3:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SHDCLASSIDS_H
#define SHDCLASSIDS_H


/*
** ClassID's for Shader Definitions
*/
enum
{
	SHDDEF_CLASSID_DUMMY = 0,
	SHDDEF_CLASSID_SIMPLE,
	SHDDEF_CLASSID_GLOSSMASK,
	SHDDEF_CLASSID_BUMPSPEC,
	SHDDEF_CLASSID_BUMPDIFF,
	SHDDEF_CLASSID_CUBEMAP,
	SHDDEF_CLASSID_LEGACYW3D,
	SHDDEF_CLASSID_LAST,
};


/*
** ClassID's for actual Shader Implementations (typically there will be several for each "type", one
** for each hardware configuration...)
*/
enum 
{
	SHD_CLASSID_DUMMY = 0,
	SHD_CLASSID_LAST,
};


#endif //SHDCLASSIDS_H
