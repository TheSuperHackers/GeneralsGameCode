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

/************************************************************************************************
*   C O N F I D E N T I A L --- W E S T W O O D   A S S O C I A T E S							*
*************************************************************************************************
*
* FILE
*     $Archive: /Renegade Setup/Autorun/Locale_API.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Maria_l $
*
* VERSION INFO
*     $Modtime: 1/15/02 2:16p $
*     $Revision: 5 $
*
*************************************************************************************************/

#pragma once

#ifndef  LOCALE_API_H
#define  LOCALE_API_H

#include <stdlib.h>

/****************************************************************************/
/* GLOBAL VARIABLES                                                         */
/****************************************************************************/
extern	int		CodePage;
extern	void *	LocaleFile;
extern	int		LanguageID;
extern	char	LanguageFile[];
extern	int		PrimaryLanguage;
extern	int		SubLanguage;

/****************************************************************************/
/* LOCALE API                                                               */
/****************************************************************************/
int				Locale_Init						( int language, char *file );
void			Locale_Restore					( void );
const wchar_t* Locale_GetString( const char *id, wchar_t *buffer = NULL, int size = _MAX_PATH );
/*
const char*		Locale_GetString				( int StringID, char *String );
const wchar_t*	Locale_GetString				( int StringID, wchar_t *String=NULL );
*/
bool			Locale_Use_Multi_Language_Files	( void );
//int				Locale_Get_Language_ID 			( void )	{ return LanguageID; };

#endif
