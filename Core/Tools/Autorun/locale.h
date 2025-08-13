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

/* Copyright (C) Electronic Arts Canada Inc. 1998-1999.  All rights reserved. */

/* ABSTRACT
locale.h - REAL fonts
@ */

#ifndef __LOCALE_H
#define __LOCALE_H 1

//#ifdef __cplusplus
//extern "C" {
//#endif


/****************************************************************************/
/* LOCALE API                                                               */
/****************************************************************************/
const char *	LOCALE_getstr( const void *localefile, int stringid );
/* NOTE: this single function replaces the previous LOCALE API */

/****************************************************************************/
/* OBSOLETE LOCALE API                                                      */
/****************************************************************************/

/* This API is being retired in favor of the much simplier single LOCALE_getstr function. */

#define			LOCALE_BANK_COUNT   8

/* initialization/restore */
int				LOCALE_init( void );										/* call before calling any other LOCALE functions */
void			LOCALE_restore( void );										/* free all memory/resources allocated by LOCALE */

/* attributes */
void			LOCALE_setbank( int bankindex );							/* bankindex >= 0 && bankindex < LOCALE_BANK_COUNT */
int				LOCALE_getbank( void );										/* returns current bank id */
int				LOCALE_getbanklanguageid( void );							/* returns language id of the current bank */
int				LOCALE_getbankstringcount( void );							/* returns the string string count in the current bank */

/* operations */
int				LOCALE_loadtable( const char *pathname, int languageid );	/* load table into the current bank */
void			LOCALE_freetable( void );									/* free table in the current bank */
const char *	LOCALE_getstring( int stringid );							/* use values in the .h file created by locomoto */

/* maintain backwards compatibility */
#define			LOCALE_purgetable	LOCALE_freetable


//#ifdef __cplusplus
//}
//#endif

#endif /* __LOCALE_H */
/* END ABSTRACT */

