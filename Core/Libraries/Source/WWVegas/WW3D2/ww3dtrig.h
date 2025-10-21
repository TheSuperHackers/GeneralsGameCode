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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/ww3dtrig.h                             $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef WW3DTRIG_H
#define WW3DTRIG_H


/*
** The WW3D Library will check for debugging triggers using the following ID's
** Your application may install a trigger handler into the WWDebug library and then
** watch for these ID's coming through.  Then if you wish to "trigger" one, then
** assign a key or something to the trigger and when your trigger handler is called,
** check if the key is down.  The trigger can be enabled in any way you want but
** in practice, most things will probably be tied to keys.
*/
enum
{
	WW3D_TRIGGER_RENDER_STATS =			0x100,		// display render stats in the debug window
	WW3D_TRIGGER_SURFACE_CACHE_STATS =	0x101,		// display surface cache info in the debug window
	WW3D_TRIGGER_PROCESS_STATS =			0x102			// render stats for last frame only
};

#endif
