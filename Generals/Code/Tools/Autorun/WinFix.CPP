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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Renegade Setup/Autorun/WinFix.CPP                          $*
 *                                                                                             *
 *                      $Author:: Maria_l                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/15/01 10:44a                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Make_Identifier -- Creates a temporary string identifer.                                  *
 *   WindowsVersionInfo::WindowsVersionInfo -- Windows Version Info constructor.               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define  STRICT
#include <windows.h>
#include <windowsx.h>
#pragma   hdrstop

//#include <commctrl.h>
//#include <winuser.h>
#include <assert.h>
#include <stdio.h>
#include "WinFix.H"
#include "Wnd_File.h"

/***************************************************************************
** Windows Version Info global object.
*/
WindowsVersionInfo WinVersion;



/***********************************************************************************************
 * WindowsVersionInfo::WindowsVersionInfo -- Windows Version Info constructor.                 *
 *                                                                                             *
 *    This routine will examine the system to determine the OS, version, and patch level of    *
 *    the current OS.                                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Don't try and use this class until after WinMain, because it won't be           *
 *             initialized until all the global objects have been constructed.                 *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *  04/09/98 jdl : Created.                                                                    *
 *=============================================================================================*/
WindowsVersionInfo::WindowsVersionInfo(void) :
	WindowsVersion(0),
	MajorVersionNumber(0),
	MinorVersionNumber(0),
	RunningOSR2(0),
	BuildNumber(0),
	IsWin9x(false),
	IsWin95(false),
	IsWin98(false),
	IsWin2000(false),
	IsWinNT(false),
	IsWinXP(false)
{
	OSVERSIONINFO version_info;

	VersionName[0] = '\0';
	AdditionalInfo[0] = '\0';

	//--------------------------------------------------------------------------
	// Start recording messages.
	//--------------------------------------------------------------------------
	Delete_Msg_File();
	Msg( __LINE__, __FILE__, "----------------------------------------------", NULL );
	Msg( __LINE__, __FILE__, "------------------ Setup -----------------", NULL );
	Msg( __LINE__, __FILE__, "----------------------------------------------", NULL );

	//--------------------------------------------------------------------------
	// Get the version info from the OS.
	//
	//	typedef struct _OSVERSIONINFO{ 
	//		DWORD	dwOSVersionInfoSize; 
	//		DWORD	dwMajorVersion; 
	//		DWORD	dwMinorVersion; 
	//		DWORD	dwBuildNumber; 
	//		DWORD	dwPlatformId; 
	//		TCHAR	szCSDVersion[ 128 ]; 
	// } OSVERSIONINFO; 
	//
	//	typedef struct _OSVERSIONINFOEX {
	//		DWORD	dwOSVersionInfoSize;
	//		DWORD	dwMajorVersion;
	//		DWORD	dwMinorVersion;
	//		DWORD	dwBuildNumber;
	//		DWORD	dwPlatformId;
	//		TCHAR	szCSDVersion[ 128 ];
	//		WORD	wServicePackMajor;
	//		WORD	wServicePackMinor;
	//		WORD	wSuiteMask;
	//		BYTE	wProductType;
	//		BYTE	wReserved;
	//	} OSVERSIONINFOEX, *POSVERSIONINFOEX, *LPOSVERSIONINFOEX;
	//--------------------------------------------------------------------------
	ZeroMemory( &version_info, sizeof( version_info ));	
	version_info.dwOSVersionInfoSize = sizeof( version_info );

	int result = GetVersionEx( &version_info );
	assert( result != 0 );

	//--------------------------------------------------------------------------
	// Save the major/minor version numbers
	//--------------------------------------------------------------------------
	MajorVersionNumber = (int)version_info.dwMajorVersion;
	MinorVersionNumber = (int)version_info.dwMinorVersion;
	WindowsVersion     = ( MajorVersionNumber * 100 ) + MinorVersionNumber;

	//--------------------------------------------------------------------------
	// Save the build number
	//--------------------------------------------------------------------------
	BuildNumber = (int)version_info.dwBuildNumber;

	//--------------------------------------------------------------------------
	// Check for Win9x
	//--------------------------------------------------------------------------
	if ( version_info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {

		IsWin9x = true;

		if ( MajorVersionNumber == 4 && MinorVersionNumber == 0 ) {
			IsWin95 = true;
		}
		if (( MajorVersionNumber > 4 ) || (( MajorVersionNumber == 4 ) && ( MinorVersionNumber > 0 ))) {
			IsWin98 = true;
		}

		if ( LOWORD( version_info.dwPlatformId ) > 1000 ) {
			RunningOSR2 = TRUE;
		}
	}

	//--------------------------------------------------------------------------
	// Check for WinNT
	//--------------------------------------------------------------------------
	if ( version_info.dwPlatformId == VER_PLATFORM_WIN32_NT ) {

		IsWinNT = true;
		
		if (( MajorVersionNumber >= 5 ) && ( MinorVersionNumber >= 1 )) {
			IsWinXP = true;
//			if ( version_info.wSuiteMask == VER_SUITE_PERSONAL ) {
//			}
		} else if (( MajorVersionNumber == 5 ) && ( MinorVersionNumber == 0 )) {
			IsWin2000 = true;
		}

//         if( bOsVersionInfoEx )
//         {
//            if ( osvi.wProductType == VER_NT_WORKSTATION )
//               printf ( "Professional " );
//
//            if ( osvi.wProductType == VER_NT_SERVER )
//               printf ( "Server " );

//         } else {

		#if( _DEBUG )
            HKEY hKey;
            char szProductType[80];
            DWORD dwBufLen;

            RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, KEY_QUERY_VALUE, &hKey );
            RegQueryValueEx( hKey, "ProductType", NULL, NULL, (LPBYTE) szProductType, &dwBufLen);
            RegCloseKey( hKey );

            if ( lstrcmpi( "WINNT", szProductType) == 0 )
				Msg( __LINE__, __FILE__, "WinNT Workstation." );
            if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
				Msg( __LINE__, __FILE__, "WinNT Server." );
		#endif
//         }
	}

#ifdef DEV_VERSION

	//--------------------------------------------------------------------------
	// For developmental versions, just use the major & minor version #'s
	//--------------------------------------------------------------------------
	sprintf( VersionName, "%x.%x", MajorVersionNumber, MinorVersionNumber );

#else

	//--------------------------------------------------------------------------
	// For final versions, trim 0's off the minor version
	//--------------------------------------------------------------------------
	unsigned short adjusted_minor;
	int i;

	adjusted_minor = MinorVersionNumber;
	for (i = 0; i < 4; i++) {
		if ((adjusted_minor & 0x000f) != 0) {
			break;
		}
		adjusted_minor >>= 4;
	}
	sprintf( VersionName, "%x.%x", MajorVersionNumber, adjusted_minor );

#endif

	//--------------------------------------------------------------------------
	// Save off the additional version information string
	// (used to indicated additional info or patch level, i.e. for NT 4.0 SP3
	// it would contain the string 'Service Pack 3')
	//--------------------------------------------------------------------------
	strncpy( AdditionalInfo, version_info.szCSDVersion, sizeof(AdditionalInfo) - 1 );
	AdditionalInfo[sizeof(AdditionalInfo) - 1] = '\x0';

	//--------------------------------------------------------------------------
	// Send all info found to the debug output file.
	//--------------------------------------------------------------------------
#if ( _DEBUG )
	Msg( __LINE__, __FILE__, "MajorVersionNumber	= %d", MajorVersionNumber );
	Msg( __LINE__, __FILE__, "MinorVersionNumber	= %d", MinorVersionNumber );
	Msg( __LINE__, __FILE__, "WindowsVersion	= %d", WindowsVersion );
	Msg( __LINE__, __FILE__, "BuildNumber	= %d", BuildNumber );
	Msg( __LINE__, __FILE__, "IsWin9x		= %d", IsWin9x );
	Msg( __LINE__, __FILE__, "IsWin95		= %d", IsWin95 );
	Msg( __LINE__, __FILE__, "IsWin98		= %d", IsWin98 );
	Msg( __LINE__, __FILE__, "IsWin2000		= %d", IsWin2000 );
	Msg( __LINE__, __FILE__, "RunningOSR2	= %d", RunningOSR2 );
	Msg( __LINE__, __FILE__, "IsWinNT		= %d", IsWinNT );
	Msg( __LINE__, __FILE__, "AdditionalInfo	= %s", AdditionalInfo );
	Msg( __LINE__, __FILE__, "VersionName	= %s", VersionName );
#endif
}

/***********************************************************************************************
 * WindowsVersionInfo::Version_String -- Get the version number in human readable form         *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to string containing version info                                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/99 10:29PM ST : Created                                                              *
 *=============================================================================================*/
char *WindowsVersionInfo::Version_String(void)
{
	static char _ver95[]   	= {"Windows 95 "};
	static char _ver98[]   	= {"Windows 98 "};
	static char _verNT4[]	= {"Windows NT 4 "};
	static char _verNT5[]	= {"Windows 2000 "};
	static char _verXP[]	= {"Windows XP "};
	static char _unknown[]	= {"Unknown "};

	static char version[256];

	if (Is_Win95()) {
		strcpy (version, _ver95);
	}

	if (Is_Win98()) {
		strcpy (version, _ver98);
	}

	if (Is_WinNT()) {
		strcpy (version, _verNT4);
	}

	if (Is_WinNT5() || Is_Win_2000()) {
		strcpy (version, _verNT5);
	}

	if (Is_Win_XP()) {
		strcpy (version, _verXP);
	}

	strcat (version, AdditionalInfo);

	return (version);
}

/***************************************************************************
 * WindowsVersionClass::Version_Name -- returns version # as char string	*
 *																									*
 * INPUT:																						*
 *		none.																						*
 *																									*
 * OUTPUT:																						*
 *		ptr to name																				*
 *																									*
 * WARNINGS:																					*
 *		none.																						*
 *																									*
 * HISTORY:																						*
 *   10/30/1995 BRR : Created.															*
 *=========================================================================*/

char * WindowsVersionInfo::Version_Name(void)
{
	return ( VersionName );
}

/****************************************************************************
 * WindowsVersionClass::Meets_Minimum_Version_Requirements					*
 *																			*
 * INPUT:																	*
 *		none.																*
 *																			*
 * OUTPUT:																	*
 *		ptr to name															*
 *																			*
 * WARNINGS:																*
 *		none.																*
 *																			*
 * HISTORY:																	*
 *   10/30/1995 BRR : Created.												*
 *==========================================================================*/

bool WindowsVersionInfo::Meets_Minimum_Version_Requirements	( void )
{
//	return(( !IsWin95 && ( Version() >= 400 ))? true : false );
	return(( Version() >= 400 )? true : false );
}



