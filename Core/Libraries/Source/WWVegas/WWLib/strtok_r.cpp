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
 *                     $Archive:: /G/wwlib/strtok_r.cpp                   $*
 *                                                                         *
 *                      $Author:: Neal_k                                  $*
 *                                                                         *
 *                     $Modtime:: 4/03/00 11:43a                          $*
 *                                                                         *
 *                    $Revision:: 1                                       $*
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   strtok_r -- POSIX replacement for strtok (no internal static)         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "strtok_r.h"
#include "stringex.h"

//
// Replacement for strtok() that doesn't use a static to
//   store the current position.  The name comes from the
//   POSIX threadsafe version of strtok (r = reentrant).  The
//   user provided var lasts is used in place of the static.
//
// Yes the Windows version of strtok is already threadsafe,
//   but the fact that you can't call a function that uses strtok()
//   during a series of strtok() calls is really annoying.
//
#ifndef _UNIX
char *strtok_r(char *strptr, const char *delimiters, char **lasts)
{
	if (strptr)
		*lasts=strptr;

	if ((*lasts)[0]==0)  // 0 length string?
		return(NULL);

	//
	// Note: strcspn & strspn are both called, they're opposites
	//
	int dstart=strcspn(*lasts, delimiters);  // find first char of string in delimiters

	if (dstart == 0)  // string starts with a delimiter
	{
		int dend=strspn(*lasts, delimiters);     // find last char of string NOT in delimiters
		*lasts+=dend;

		if ((*lasts)[0]==0)  // 0 length string?
			return(NULL);

		dstart=strcspn(*lasts, delimiters);
	}
	char *retval=*lasts;

	if ((*lasts)[dstart]==0)  // is this the last token?
		*lasts+=dstart;
	else	// at least one more token to go...
	{
		(*lasts)[dstart]=0;  // null out the end
		*lasts+=(dstart+1);  // advance pointer
	}
	return(retval);
}
#endif
