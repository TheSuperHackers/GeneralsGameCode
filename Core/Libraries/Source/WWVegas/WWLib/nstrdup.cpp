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

 /***************************************************************************
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     ***
 ***************************************************************************
 *                                                                         *
 *                 Project Name : G                                        *
 *                                                                         *
 *                     $Archive:: /G/wwlib/nstrdup.cpp                    $*
 *                                                                         *
 *                      $Author:: Neal_k                                  $*
 *                                                                         *
 *                     $Modtime:: 9/20/99 11:56a                          $*
 *                                                                         *
 *                    $Revision:: 6                                       $*
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   nstrdup -- duplicates a string using new[]                            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "always.h"
#include "nstrdup.h"
//#include "../app/main/gnew.h"

/**************************************************************************
 * nstrdup -- duplicates a string using new[]                             *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   02/03/1998 PWG : Created.                                            *
 *========================================================================*/
char * nstrdup(const char *str)
{
	if(str == 0) return 0;

	// eventually should be replaced with NEW when we go to the wwnew stuff.
	char *retval = W3DNEWARRAY char [strlen(str) + 1];
	strcpy(retval, str);
	return retval;
}


