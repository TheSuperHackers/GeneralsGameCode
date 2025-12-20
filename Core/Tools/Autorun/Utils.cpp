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

/****************************************************************************
 ***   C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S        ***
 ****************************************************************************
 *																			*
 *         	   Project Name : Setup                         				*
 *																			*
 *                File Name : UTILS.C										*
 *																			*
 *               Programmers: Maria del Mar McCready Legg					*
 *																			*
 *               Start Date : December 12,  1992							*
 *																			*
 *              Last Update : March 16, 1998  [MML]             	  		*
 *																			*
 *--------------------------------------------------------------------------*
 * Functions:																*
 *																			*
 *	Clip_Line_To_Rect			--	Clips a line (two points) against a		*
 *										rectangle, using CS algorithm.		*
 *	Compute_Code				--	computes line clipping bit code for 	*
 *										point & rectangle.					*
 *	Copy_File 					--	Copies a file from one dir to another.	*
 *	Convert_Hex_To_Version		--	Converts a hex num obtained from the	*
 *										Registry, into a string				*
 *										representation of a version 		*
 *										number ( XX.XX ).					*
 *	Convert_Version_To_Hex		--	Converts a string to an unsigned long.	*
 *	Convert_To_Version_Format	--	Converts version string's "," to "."s   *
 *	Dialog_Box 					--	draws a dialog background box           *
 *	Draw_Box 					--	Displays a highlighted box.             *
 *	Fatal						--	General purpose fatal error handler.	*
 *	Get_Version					--	Retrieves a version string from a file. *
 *	Get_String 					--	Returns a pointer to the undipped text. *
 *	Is_File_Available 		  	--	Use both FindFirst to check that CD is	*
 *									    in drive & if File_Exists() to 		*
 *										determine if file is really there.	*
 *	Pad_With_Zeros            	--	Adds zeros to the beginning of string.	*
 *	String_Width 				--	Calculate with of the string.			*
 *	Strip_Newlines 			  	--	Remove '\r' from string passed in.		*
 *	TextPtr						--	Returns a pointer to the undipped text. *
 *	Path_Name_Valid			  	--	Validate that the path has the correct  *
 *										number of chars between '\' in the	*
 *										path.								*
 *	Path_Get_Next_Directory		--	Return the next dir path from string.	*
 *	Path_Add_Back_Slash			--	Add a "\\" to the end of the string.	*
 *	Path_Remove_Back_Slash		--	Remove a '\\' at the end of the string.	*
 *	Path_Trim_Blanks			--	Trim lead/trail white spaces off string	*
 *	Pad_With_Zeros				--	Adds zeros to the beginning of string.  *
 *	Remove_Ending_Spaces		--	Remove any blank spaces at end of string*
 *	Remove_Spaces				--	Remove spaces from string passed in.    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#include <io.h>
#include "ARGS.h"
#include "assert.h"
#include "Locale_API.h"
#include "resource.h"
#include "Utils.h"
#include "WinFix.h"
#include "Wnd_File.h"
#include <winver.h>
#include <shlwapi.h>
//#include "resources.h"



//----------------------------------------------------------------------------
//
// Function: Fix_Single_Ampersands()
//
//  Purpose: To replace each "&" with "&&" for display in a dialog.
//			 Some dialogs mistake a single "&" for an accelerator key.
//
//    Input: pszString - any NULL terminated string.
//
//  Returns: VOID (returns nothing)
//
// Comments: Modifies the characters in pszString.
//
//---------------------------------------------------------------------------

void Fix_Single_Ampersands ( LPSTR pszString, bool upper_case )
{
	char	pszTemp[ MAX_PATH ];		// variable to hold the string passed
	char	pszOld[ MAX_PATH ];			// variable to hold the string passed
	char *	letter;
	int		i = 0;

	lstrcpy((LPSTR)pszOld, (LPSTR)pszString );
	letter = pszOld;
	memset ( pszTemp, '\0', MAX_PATH );

	//----------------------------------------------------------------------
	// While our ptr has not passed the end of the string...
	//----------------------------------------------------------------------
	while (*letter != '\0') {

		if (*letter == '&') {

			pszTemp[i++] = '&';
			pszTemp[i++] = '&';
			letter++;

		} else {

			if ( upper_case ) {
				pszTemp[i++] = (char) toupper( *( letter++ ));
			} else {
				pszTemp[i++] = *(letter++);
			}
		}
	}
	strcpy((LPSTR)pszString, (LPSTR)pszTemp );
}

void Fix_Single_Ampersands ( wchar_t *pszString, bool upper_case )
{
	wchar_t	pszTemp[ MAX_PATH ];		// variable to hold the string passed
	wchar_t	pszOld[ MAX_PATH ];			// variable to hold the string passed
	wchar_t *letter;
	int		i = 0;

	wcscpy( pszOld, pszString );
	letter = pszOld;
	memset ( pszTemp, '\0', MAX_PATH );

	//----------------------------------------------------------------------
	// While our ptr has not passed the end of the string...
	//----------------------------------------------------------------------
	while (*letter != '\0') {

		if (*letter == '&') {

			pszTemp[i++] = '&';
			pszTemp[i++] = '&';
			letter++;

		} else {

			if ( upper_case ) {
				pszTemp[i++] = (char) toupper( *( letter++ ));
			} else {
				pszTemp[i++] = *(letter++);
			}
		}
	}
	wcscpy( pszString, pszTemp );
}

////////////////UnicodeString Fix_Single_Ampersands( UnicodeString string, bool upper_case)
////////////////{
////////////////	UnicodeString retval;
////////////////
////////////////	Int i = 0;
////////////////	while (i < string.getLength()) {
////////////////		if (upper_case) {
////////////////			retval.concat(toupper(string.getCharAt(i)));
////////////////		} else {
////////////////			retval.concat(string.getCharAt(i));
////////////////		}
////////////////		if (string.getCharAt(i) == L'&') {
////////////////			retval.concat(string.getCharAt(i));
////////////////		}
////////////////		++i;
////////////////	}
////////////////
////////////////	return retval;
////////////////}

//----------------------------------------------------------------------------
//
// Function: Fix_Double_Ampersands()
//
//  Purpose: To replace each "&&" with "&" for display in a dialog.
//			 Some dialogs mistake a single "&" for an accelerator key.
//
//    Input: pszString - any NULL terminated string.
//
//  Returns: VOID (returns nothing)
//
// Comments: Modifies the characters in pszString.
//
//---------------------------------------------------------------------------

void Fix_Double_Ampersands ( LPSTR pszString, bool upper_case )
{
	char pszTemp[ MAX_PATH ];  // variable to hold the string passed
	char pszOld[ MAX_PATH ];   // variable to hold the string passed
	char *letter;
	int  i = 0;

	lstrcpy( (LPSTR)pszOld, (LPSTR)pszString );
	letter = pszOld;
	memset ( pszTemp, '\0', MAX_PATH );

	//----------------------------------------------------------------------
	// While our ptr has not passed the end of the string...
	//----------------------------------------------------------------------
	while (*letter != '\0') {

		if ((*letter == '&') && (*( letter+1 ) == '&')) {

			pszTemp[i++] = '&';
			letter = letter + 2;

		} else {

			if ( upper_case ) {
				pszTemp[i++] = (char) toupper( *( letter++ ));
			} else {
				pszTemp[i++] = *(letter++);
			}
		}
	}
	strcpy((LPSTR)pszString, (LPSTR)pszTemp );
}


/******************************************************************************
 * Load_Alloc_Data -- Allocates a buffer and loads the file into it.          *
 *                                                                            *
 *	This is the C++ replacement for the Load_Alloc_Data function. It will     *
 *	allocate the memory big enough to hold the file & read the file into it.  *
 *                                                                            *
 * INPUT:   file  -- The file to read.                                        *
 *          mem   -- The memory system to use for allocation.                 *
 *                                                                            *
 * OUTPUT:  Returns with a pointer to the allocated and filled memory block.  *
 *                                                                            *
 * WARNINGS:   none                                                           *
 *                                                                            *
 * HISTORY:                                                                   *
 *   10/17/1994 JLB : Created.                                                *
 *============================================================================*/

void * Load_Alloc_Data( char *filename, long *filesize )
{
	int					size, bytes_read;
	void				*ptr = NULL;
	StandardFileClass	file;

	//-------------------------------------------------------------------------
	// Open file in READ ONLY mode.  If fails, exit.
	//-------------------------------------------------------------------------
	file.Open( filename, MODE_READ_ONLY );
	if ( !file.Query_Open()) {
		return( NULL );
	}

	//-------------------------------------------------------------------------
	// Get filesize and create a buffer.
	//-------------------------------------------------------------------------
  	size = file.Query_Size();
	ptr = (void*)malloc(size + 1);
	if ( !ptr ) {
		return( NULL );
	}

	//-------------------------------------------------------------------------
	// Read data into the buffer, close the file.
	//-------------------------------------------------------------------------
	memset( ptr, '\0', size + 1 );
  	bytes_read = file.Read( ptr, size );
  	file.Close();

	//-------------------------------------------------------------------------
	// Check return bytes.  It should match the file size.
	//-------------------------------------------------------------------------
	assert( bytes_read == size );
	if ( bytes_read != size ) {
		free(ptr);
		return( NULL );
	}

	if ( filesize != NULL ) {
		*filesize = (long)size;
	}
	return( ptr );
}

/****************************************************************************
 * MIXFILECLASS::LOAD_FILE -- Returns a buffer loaded with file desired.	*
 *                                                                 			*
 * INPUT:   	none.														*																								  *
 *                                                                 			*
 * OUTPUT:		none.                        								*
 *																			*
 * WARNINGS:	Searches MixFile first, then local directory.				*
 *				Use free() to release buffer.								*																			*
 *																			*
 * HISTORY:                                                        			*
 *   04/13/1998  ML/MG : Created.                                    		*
 *==========================================================================*/

void *Load_File ( char *filename, long *filesize )
{
	void *ptr = NULL;

	if ( filename == NULL || filename[0] == '\0' ) {
		return( NULL );
	}

	//-------------------------------------------------------------------------
	// Try loading from local directory.
	//-------------------------------------------------------------------------
	ptr = Load_Alloc_Data( filename, filesize );

	return( ptr );
}


/****************************************************************************
 * MAKE_CURRENT_PATH_TO -- Returns a buffer to path desired.				*
 *                                                                 			*
 * INPUT:   	none.														*																								  *
 *                                                                 			*
 * OUTPUT:		none.                        								*
 *																			*
 * WARNINGS:																*																			*
 *																			*
 * HISTORY:                                                        			*
 *   10/08/2001  MML : Created.                                    			*
 *==========================================================================*/

char *Make_Current_Path_To ( const char *filename, char *path )
{
	char	szPath	[ _MAX_PATH ];
	char	drive	[ _MAX_DRIVE];
	char	dir	 	[ _MAX_DIR  ];

	strcpy( szPath, Args->Get_argv(0));
	_splitpath( szPath, drive, dir, NULL, NULL );
	_makepath( szPath, drive, dir, NULL, NULL );
	Path_Add_Back_Slash( szPath );
	strcat( szPath, filename );

	if( path != NULL ) {
		strcpy( path, szPath );
	}
	return( path );
}

wchar_t *Make_Current_Path_To ( const wchar_t *filename, wchar_t *path )
{
	wchar_t	szPath	[ _MAX_PATH ];
	wchar_t	drive	[ _MAX_DRIVE];
	wchar_t	dir	 	[ _MAX_DIR  ];

	wcscpy( szPath, (wchar_t *)Args->Get_argv(0));
	_wsplitpath( szPath, drive, dir, NULL, NULL );
	_wmakepath( szPath, drive, dir, NULL, NULL );
	Path_Add_Back_Slash( szPath );
	wcscat( szPath, filename );

	if( path != NULL ) {
		wcscpy( path, szPath );
	}
	return( path );
}


/******************************************************************************
 * Path_Add_Back_Slash -- Add a '\\' to the end of the path.
 *
 * INPUT:		char * path -- Pointer to the string to be modified.
 *
 * OUTPUT:     char * path
 *
 * WARNINGS:   none
 *
 * HISTORY:
 *   08/14/1998 MML : Created.
 *============================================================================*/

char *Path_Add_Back_Slash ( char *path )
{
	if ( path != NULL && *path != '\0' ) {
		if ( path[ strlen( path )-1 ] != '\\' ) {
			 strcat( path, "\\" );
		}
	}
	return( path );
}

wchar_t *Path_Add_Back_Slash ( wchar_t *path )
{
	if ( path != NULL && *path != '\0' ) {
		if ( path[ wcslen( path )-1 ] != '\\' ) {
			 wcscat( path, L"\\" );
		}
	}
	return( path );
}


/******************************************************************************
 * Path_Remove_Back_Slash -- Remove a '\\' from the end of the path.
 *
 * INPUT:		char * path -- Pointer to the string to be modified.
 *
 * OUTPUT:     char * path
 *
 * WARNINGS:   none
 *
 * HISTORY:
 *   08/14/1998 MML : Created.
 *============================================================================*/

char *Path_Remove_Back_Slash ( char *path )
{
	if ( path != NULL && *path != '\0' ) {
		if ( path[ strlen( path )-1 ] == '\\' ) {
			 path[ strlen( path )-1 ] = '\0';
		}
	}
	return( path );
}

wchar_t *Path_Remove_Back_Slash ( wchar_t *path )
{
	if ( path != NULL && *path != '\0' ) {
		if ( path[ wcslen( path )-1 ] == L'\\' ) {
			 path[ wcslen( path )-1 ] = L'\0';
		}
	}
	return( path );
}

/*--------------------------------------------------------------------------*/
/* Function: PlugInProductName												*/
/*																			*/
/* Descrip:  The function plugs the product name defined in					*/
/*           SdProductName() into %P found in the static message.			*/
/*           It will search for the first nMax controls only.				*/
/* Misc:															   		*/
/*																	   		*/
/*--------------------------------------------------------------------------*/

void PlugInProductName ( char *szString, char *szName )
{
	int		nCount, nMsgLength;
	char	szTextBuf[ MAX_PATH ];
	char	szOut[ MAX_PATH ];
	char	szProduct[ MAX_PATH ];
	char *	temp = NULL;
	char *	next = NULL;

	if ( szName == NULL || szName[0] == '\0' ) {
		return;
	}

	//--------------------------------------------------------------------------
	// Find the first appearance of "%P".
	//--------------------------------------------------------------------------
	strcpy( szProduct, szName );
	strcpy( szTextBuf, szString );
	nMsgLength	= strlen( szTextBuf );
	nCount		= 0;
	temp 		= strstr( szTextBuf, "%s" );

	//-------------------------------------------------------------
	// Substitute each "%P" with "%s".  nStrReturn is the index
	// into the buffer where "%P" was found.
	//-------------------------------------------------------------
	while ( temp != NULL && nCount < 6) {
		next	= temp+1;
		nCount	= nCount + 1;
		temp	= strstr( next, "%s" );
	}

	//-------------------------------------------------------------
	// Only support up to 5 product name per message.
	// Do the substitution of the product name and store in szOut.
	//-------------------------------------------------------------
	switch( nCount ) {
		case 1:
			sprintf( szOut, szTextBuf, szProduct );
			break;
		case 2:
			sprintf( szOut, szTextBuf, szProduct, szProduct );
			break;
		case 3:
			sprintf( szOut, szTextBuf, szProduct, szProduct, szProduct );
			break;
		case 4:
			sprintf( szOut, szTextBuf, szProduct, szProduct, szProduct, szProduct );
			break;
		case 5:
			sprintf( szOut, szTextBuf, szProduct, szProduct, szProduct, szProduct, szProduct, szProduct );
			break;
	}

	//-------------------------------------------------------------
	// Replace szTextBuf with szOut.
	//-------------------------------------------------------------
	if ( nCount >= 1 ) {
		strcpy( szString, szOut );
	}
}

/*--------------------------------------------------------------------------*/
/* Function: PlugInProductName												*/
/*																			*/
/* Descrip:  The function plugs the product name defined in					*/
/*           SdProductName() into %P found in the static message.			*/
/*           It will search for the first nMax controls only.				*/
/* Misc:															   		*/
/*																	   		*/
/*--------------------------------------------------------------------------*/

void PlugInProductName( char *szString, int nName )
{
/*
	int		nCount, nMsgLength;
	char	szTextBuf[ MAX_PATH ];
	char	szOut[ MAX_PATH ];
	char	szProduct[ MAX_PATH ];
	char *	temp = NULL;
	char *	next = NULL;

	if ( nName <= STRNONE ) {
		nName = STRNONE;
	}

	//--------------------------------------------------------------------------
	// Find the first appearance of "%P".
	//-------------------------------------------------------------
//	LoadString( Main::hInstance, nName, szProduct, MAX_PATH );
	Locale_GetString( nName, szProduct );

	strcpy( szTextBuf, szString );
	nMsgLength	= strlen( szTextBuf );
	nCount		= 0;
	temp 		= strstr( szTextBuf, "%s" );

	//-------------------------------------------------------------
	// Substitute each "%P" with "%s".  nStrReturn is the index
	// into the buffer where "%P" was found.
	//-------------------------------------------------------------
	while ( temp != NULL && nCount < 6) {
		next	= temp+1;
		nCount	= nCount + 1;
		temp	= strstr( next, "%s" );
	}

	//-------------------------------------------------------------
	// Only support up to 5 product name per message.
	// Do the substitution of the product name and store in szOut.
	//-------------------------------------------------------------
	switch( nCount ) {
		case 1:
			sprintf( szOut, szTextBuf, szProduct );
			break;
		case 2:
			sprintf( szOut, szTextBuf, szProduct, szProduct );
			break;
		case 3:
			sprintf( szOut, szTextBuf, szProduct, szProduct, szProduct );
			break;
		case 4:
			sprintf( szOut, szTextBuf, szProduct, szProduct, szProduct, szProduct );
			break;
		case 5:
			sprintf( szOut, szTextBuf, szProduct, szProduct, szProduct, szProduct, szProduct, szProduct );
			break;
	}

	//-------------------------------------------------------------
	// Replace szTextBuf with szOut.
	//-------------------------------------------------------------
	if ( nCount >= 1 ) {
		strcpy( szString, szOut );
	}
*/
}

/*--------------------------------------------------------------------------*/
/* Function: PlugInProductName												*/
/*																			*/
/* Descrip:  The function plugs the product name defined in					*/
/*           SdProductName() into %P found in the static message.			*/
/*           It will search for the first nMax controls only.				*/
/* Misc:															   		*/
/*																	   		*/
/*--------------------------------------------------------------------------*/

void PlugInProductName ( wchar_t *szString, const wchar_t *szName )
{
	int		nCount, nMsgLength;
	wchar_t	szTextBuf[ MAX_PATH ];
	wchar_t	szOut[ MAX_PATH ];
	wchar_t	szProduct[ MAX_PATH ];
	wchar_t *temp = NULL;
	wchar_t *next = NULL;

	if ( szName == NULL || szName[0] == '\0' ) {
		return;
	}

	//--------------------------------------------------------------------------
	// Find the first appearance of "%P".
	//--------------------------------------------------------------------------
	wcscpy( szProduct, szName );
	wcscpy( szTextBuf, szString );
	nMsgLength	= wcslen( szTextBuf );
	nCount		= 0;
	temp 		= wcsstr( szTextBuf, L"%s" );

	//-------------------------------------------------------------
	// Substitute each "%P" with "%s".  nStrReturn is the index
	// into the buffer where "%P" was found.
	//-------------------------------------------------------------
	while ( temp != NULL && nCount < 6) {
		next	= temp+1;
		nCount	= nCount + 1;
		temp	= wcsstr( next, L"%s" );
	}

	//-------------------------------------------------------------
	// Only support up to 5 product name per message.
	// Do the substitution of the product name and store in szOut.
	//-------------------------------------------------------------
	switch( nCount ) {
		case 1:
			swprintf( szOut, szTextBuf, szProduct );
			break;
		case 2:
			swprintf( szOut, szTextBuf, szProduct, szProduct );
			break;
		case 3:
			swprintf( szOut, szTextBuf, szProduct, szProduct, szProduct );
			break;
		case 4:
			swprintf( szOut, szTextBuf, szProduct, szProduct, szProduct, szProduct );
			break;
		case 5:
			swprintf( szOut, szTextBuf, szProduct, szProduct, szProduct, szProduct, szProduct, szProduct );
			break;
	}

	//-------------------------------------------------------------
	// Replace szTextBuf with szOut.
	//-------------------------------------------------------------
	if ( nCount >= 1 ) {
		wcscpy( szString, szOut );
	}
}




