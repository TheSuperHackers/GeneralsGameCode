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
 *** C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S 		 ***
 ***************************************************************************
 *                                                                         *
 *     	          Project Name : Setup                   				   *
 *                                                                         *
 *                    File Name : UTILS.H                                  *
 *                                                                         *
 *                   Programmers: Maria del Mar McCready Legg              *
 *                                                                         *
 *                   Start Date : December 20, 1994                        *
 *                                                                         *
 * 	              Last Update : April 06, 1998  [MML]               	   *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <windows.h>


/******************************************************************************
**	Swaps two objects.
*/
template<class T>
void swap( T & left, T & right )
{
	T temp;

	temp  = left;
	left  = right;
	right = temp;
}


void		Fix_Single_Ampersands  			( LPSTR pszString, bool upper_case );
void		Fix_Single_Ampersands  			( wchar_t *pszString, bool upper_case );
//UnicodeString Fix_Single_Ampersands ( UnicodeString string, bool upper_case);
void		Fix_Double_Ampersands  			( LPSTR string, bool upper_case );
void *		Load_Alloc_Data					( char *filename, long *filesize=0 );
void *		Load_File						( char *filename, long *filesize=0 );
char *		Make_Current_Path_To			( const char *filename, char *path );
wchar_t *	Make_Current_Path_To			( const wchar_t *filename, wchar_t *path );
char *		Path_Add_Back_Slash				( char *path );
char *		Path_Remove_Back_Slash			( char *path );
wchar_t *	Path_Add_Back_Slash				( wchar_t *path );
wchar_t *	Path_Remove_Back_Slash			( wchar_t *path );
void		PlugInProductName				( char *szString, int nName );
void		PlugInProductName				( char *szString, char *szName );
void		PlugInProductName				( wchar_t *szString, const wchar_t *szName );
