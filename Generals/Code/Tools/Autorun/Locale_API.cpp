/*
**	Command & Conquer Generals(tm)
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

/*****************************************************************************
*   C O N F I D E N T I A L --- W E S T W O O D   A S S O C I A T E S		 *
******************************************************************************
*
* FILE
*     $Archive: /Renegade Setup/Autorun/Locale_API.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Maria_l $
*
* VERSION INFO
*     $Modtime: 1/22/02 5:41p $
*     $Revision: 12 $
*
*****************************************************************************/

#include "locale.h"
#include "Locale_API.h"
#include "Utils.h"
#include "Wnd_File.h"
//#include "resources.h"

#include "GameText.h"

#define MISSING_STRING_HINTS_MAX (20)

wchar_t *localeStringsMissing[ MISSING_STRING_HINTS_MAX ] =
{
	{ L"0 MissingInstall"		},
	{ L"1 MissingExplore"		},
	{ L"2 MissingPreviews"		},
	{ L"3 MissingCancel"		},
	{ L"4 MissingAutoRunTitle"		},
	{ L"5 MissingNoExplorer"		},
	{ L"6 MissingWinVersionText"		},
	{ L"7 MissingWinVersionTitle"		},
	{ L"8 MissingCantFind"		},
	{ L"9 MissingUninstall"		},
	{ L"10 MissingWebsite"	},
	{ L"11 MissingCheckForUpdates"	},
	{ L"12 MissingWorldBuilder"	},
	{ L"13 MissingPlay"	},
	{ L"14 MissingGameTitle"	},
	{ L"15 MissingFullGameTitle"	},
	{ L"16 MissingRegistryKey"	},
	{ L"17 MissingMainWindow"	},
	{ L"18 MissingThing"	},
	{ L"19 UNKNOWN MESSAGE"	}
};









/****************************************************************************/
/* DEFINES			                                                        */
/****************************************************************************/

//----------------------------------------------------------------------------
// NOTE:	if USE_MULTI_FILE_FORMAT is "true", then a .lOC file must be in 
//			the same directory as this file.
//----------------------------------------------------------------------------
#define		USE_MULTI_FILE_FORMAT		FALSE

#define		LANGUAGE_IS_DBCS(l)	(((l)==IDL_JAPANESE)||((l)==IDL_KOREAN)||((l)==IDL_CHINESE))		// [OYO]
#define		CODEPAGE_IS_DBCS(C)	((C==932)||(C==949)||(C==950))										// [OYO]


/****************************************************************************/
/* GLOBAL VARIABLES                                                         */
/****************************************************************************/
char		LanguageFile[ _MAX_PATH ];
void *		LocaleFile		= NULL;
int			CodePage		= GetACP();
int			LanguageID		= 0;
int			PrimaryLanguage = LANG_NEUTRAL;
int			SubLanguage		= SUBLANG_DEFAULT;


/****************************************************************************/
/* LOCALE API                                                               */
/****************************************************************************/
wchar_t *	Remove_Quotes_Around_String ( wchar_t *old_string );


//=============================================================================
// These are wrapper functions around the LOCALE_ functions.  I made these to 
// make using the single vs. multi language files more transparent to the program.
//=============================================================================

bool Locale_Use_Multi_Language_Files ( void )
{
#if( USE_MULTI_FILE_FORMAT )
	return true;
#else
	return false;
#endif
}


/****************************************************************************/
/* initialization															*/
/****************************************************************************/

int Locale_Init	( int language, char *file )
{
	int	result = 0;

	TheGameText = CreateGameTextInterface();
	TheGameText->init();

/*
	//-------------------------------------------------------------------------
	// Check for valid language range.
	//-------------------------------------------------------------------------
	if( language < 0 || language >= LANG_NUM ) {
		language = 0;
	}

	//-------------------------------------------------------------------------
	// Check for a file passed in.
	//-------------------------------------------------------------------------
	if( file == NULL || file[0] == '/0' ) {
		return 0;
	}
	strcpy( LanguageFile, file );
	LanguageID = language;

	Msg( __LINE__, __FILE__, "LanguageID = %d", LanguageID );
	Msg( __LINE__, __FILE__, "CodePage   = %d.", CodePage );
	Msg( __LINE__, __FILE__, "LanguageFile = %s.", LanguageFile );

	//-------------------------------------------------------------------------
	// Initialize the lx object.
	//-------------------------------------------------------------------------
	LOCALE_init();

	#if( USE_MULTI_FILE_FORMAT )

		//---------------------------------------------------------------------
		// Set bank to use and load the appropriate table.
		//---------------------------------------------------------------------
		LOCALE_setbank(0);
		result = LOCALE_loadtable( LanguageFile, LanguageID );

	#else

		//---------------------------------------------------------------------
		// Create a file buffer that holds all the strings in the file.
		//---------------------------------------------------------------------
		long	filesize;
		HRSRC 	hRsrc;
		HGLOBAL hGlobal;

		HMODULE module = GetModuleHandle( NULL );

		//-------------------------------------------------------------------------
		// Find the string file in this program's resources.
		//-------------------------------------------------------------------------
		switch( CodePage ) {

			// Japanese
			case 932:
				PrimaryLanguage = LANG_JAPANESE;
				SubLanguage		= SUBLANG_DEFAULT;
				break;

			// Korean
			case 949:
				PrimaryLanguage = LANG_KOREAN;
				SubLanguage		= SUBLANG_DEFAULT;
				break;

			// Chinese
			case 950:
				PrimaryLanguage = LANG_CHINESE;
				SubLanguage		= SUBLANG_DEFAULT;
				break;

			// English, French, and German
			case 1252:

				switch( LanguageID ) {

					case LANG_GERMAN:
						PrimaryLanguage = LANG_GERMAN;
						SubLanguage		= SUBLANG_GERMAN;
						break;

					case LANG_FRENCH:
						PrimaryLanguage = LANG_FRENCH;
						SubLanguage		= SUBLANG_FRENCH;
						break;

					case LANG_ENGLISH:
						PrimaryLanguage = LANG_ENGLISH;
						SubLanguage		= SUBLANG_ENGLISH_US;
						break;
				}
				break;
		}

		hRsrc = FindResourceEx( module, RT_RCDATA, "STRINGS", MAKELANGID( PrimaryLanguage, SubLanguage ));
		if ( hRsrc == NULL ) {
			hRsrc = FindResourceEx( module, RT_RCDATA, "STRINGS", MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ));
		}
		if ( hRsrc ) {

			//---------------------------------------------------------------------
			// Load the resource, lock the memory, grab a DC.
			//---------------------------------------------------------------------
			hGlobal		= LoadResource( module, hRsrc );
			filesize	= SizeofResource( module, hRsrc );

			LocaleFile = (void*)malloc( filesize + 1 );
			memset( LocaleFile, '\0', filesize + 1 );
			memcpy( LocaleFile, (const void *)hGlobal, filesize );

			//---------------------------------------------------------------------
			// Free DS and memory used.
			//---------------------------------------------------------------------
			UnlockResource( hGlobal );
			FreeResource( hGlobal );
		}

		if( LocaleFile == NULL ) {
			LocaleFile = Load_File( LanguageFile, &filesize );
		}

		if( LocaleFile != NULL ) {
			result = 1;
		}

		//---------------------------------------------------------------------
		// Set the LanguageID because we may need this later.
		//---------------------------------------------------------------------
//		CHAR buffer[ _MAX_PATH ];

//		Locale_GetString( LANG_NUM, buffer );
//		LanguageID = atoi( buffer );

		LanguageID = 0;

#if(_DEBUG)
		switch( LanguageID ) {
			case 6:
				CodePage = 932;
				break;
			case 9:
				CodePage = 949;
				break;
			case 10:
				CodePage = 950;
				break;
		}
#endif

	#endif
*/
	return result;	
}

/************************************************************************/
/* restore	 															*/
/************************************************************************/

void Locale_Restore ( void )
{
	if (TheGameText)
	{
		delete TheGameText;
		TheGameText = NULL;
	}

	#if( USE_MULTI_FILE_FORMAT )
		LOCALE_freetable();
		LOCALE_restore();
	#else
		if( LocaleFile ) {
			free( LocaleFile );
			LocaleFile = NULL;
		}
	#endif
}

/****************************************************************************/
/* retreiving strings												 		*/
/****************************************************************************/

const char* Locale_GetString( int StringID, char *String )
{
	static	char		buffer[ _MAX_PATH ];
	static	wchar_t		wide_buffer[ _MAX_PATH ];

	memset( buffer, '\0', _MAX_PATH );
	memset(	wide_buffer, '\0', _MAX_PATH );

	#if( USE_MULTI_FILE_FORMAT )
		wcscpy( wide_buffer, (wchar_t *)LOCALE_getstring( StringID ));
	#else									  
		wcscpy( wide_buffer, (wchar_t *)LOCALE_getstr( LocaleFile, StringID ));
	#endif

	Remove_Quotes_Around_String( wide_buffer );
	WideCharToMultiByte( CodePage, 0, wide_buffer, _MAX_PATH, buffer, _MAX_PATH, NULL, NULL );

	if( String != NULL ) {
		strncpy( String, buffer, _MAX_PATH );
	}

	return buffer;
}

const wchar_t* Locale_GetString( const char *id, wchar_t *buffer, int size )
{
	if (TheGameText)
	{
		const wchar_t *fetched = TheGameText->fetch(id);
		if (buffer)
		{
			wcsncpy(buffer, fetched, size);
			buffer[size-1] = 0;
		}
		return fetched;
	}
	return L"No String Manager";
}

/*
const wchar_t* Locale_GetString( int StringID, wchar_t *String )
{
	static wchar_t wide_buffer[ _MAX_PATH ];

	memset(	wide_buffer, '\0', _MAX_PATH );

	#if( USE_MULTI_FILE_FORMAT )
		wcscpy( wide_buffer, (wchar_t *)LOCALE_getstring( StringID ));
	#else		
		
		wchar_t *localeStr = NULL;
		
		if (TheGameText != NULL)
			localeStr = (wchar_t *)TheGameText->fetch( s_stringLabels[StringID] );
		
		if (localeStr == NULL) 
		{
			return localeStringsMissing[ min( MISSING_STRING_HINTS_MAX - 1, StringID ) ];
		} 
		else 
		{
			wcscpy( wide_buffer, localeStr);
		}
	#endif 

	Remove_Quotes_Around_String( wide_buffer );

	if( String != NULL ) {
		wcsncpy( String, wide_buffer, _MAX_PATH );
	}

	return wide_buffer;
}
*/

/****************************************************************************/
/* formating strings   														*/
/****************************************************************************/

wchar_t *Remove_Quotes_Around_String ( wchar_t *old_string )
{
	wchar_t	wide_buffer[ _MAX_PATH ];
	wchar_t *letter = old_string;
	int		length;

	//----------------------------------------------------------------------
	// If string is not NULL...
	//----------------------------------------------------------------------
	if ( *letter == '"' ) {

		letter++;
		wcscpy( wide_buffer, letter );

		length = wcslen( wide_buffer );

		if ( wide_buffer[ wcslen( wide_buffer )-1 ] == '"' ) {
			wide_buffer[ wcslen( wide_buffer )-1 ] = '\0';
		}
		wcscpy( old_string, wide_buffer );
	}

	return( old_string );
}



