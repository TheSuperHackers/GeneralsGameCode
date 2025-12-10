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
 *****	C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S	*****
 ****************************************************************************
 *                 Project Name:: Command & Conquer							*
 *                     $Archive:: /Renegade Setup/Autorun/GETCD.cpp              $Author:: Steve_t									*
 *                     $Modtime:: 1/28/02 10:54a                  $Revision:: 20										*
 *--------------------------------------------------------------------------*
 * Functions:																*
 *		GetCDClass::GetCDClass	--	default constructor						*
 *		GetCDClass::~GetCDClass	--	destructor								*
 *		GetCDClass::GetCDDrive	--	returns the logical CD drive			*
 *		CD_Volume_Verification	--	Check label of the CDRom.				*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#include	<windows.h>
#include	<string.h>
//#include	"always.h"
#include	"GetCD.h"
//#include	"timer.h"
#include	"Wnd_File.h"
//#include	"missiondisk.h"
#include	"WinFix.h"

#ifndef ROR_NOT_READY
#define ROR_NOT_READY		21
#endif

/**********************************************************************
**	This macro serves as a general way to determine the number of elements
**	within an array.
*/
#define	ARRAY_SIZE(x)		int(sizeof(x)/sizeof(x[0]))
#define	size_of(typ,id)		sizeof(((typ*)0)->id)

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
#if ( MISSION_DISK )

static const char * _CD_Volume_Label[] = {
	"Renegade Mission",		// Yuri's Revenge (mission disk)
};

#else

static const char * _CD_Volume_Label[] = {
	"Renegade Game",		// Red Alert 2
	"Renegade Data",		// Red Alert 2
};

#endif


static int _Num_Volumes = ARRAY_SIZE( _CD_Volume_Label );

GetCDClass	CDList;


/****************************************************************************
 * GetCDClass -- default constructor										*
 *																			*
 * INPUT:																	*
 *			none 															*
 * OUTPUT:																	*
 *			none															*
 * WARNINGS:																*
 *																			*
 * HISTORY:																	*
 *   05/26/1994 SW	:	Created.											*
 *   12/04/1995 ST	:	fixed for Win95										*
 *==========================================================================*/
GetCDClass::GetCDClass( void )
{
	char path[]={ "a:\\" };

	CDCount = 0;
	CDIndex = 0;

	Msg( __LINE__, __FILE__, "GetCDClass constructor\n" );

	/*--------------------------------------------------------------------------
	** Set all CD drive placeholders to empty
	*/
	for( int j = 0; j < MAX_CD_DRIVES; j++ ) {
		CDDrives[j] = NO_CD_DRIVE;
	}

	for( char i = 'c'; i <= 'z'; i++ ) {

		path[0] = i;

		if ( GetDriveType( path ) == DRIVE_CDROM ) {
			CDDrives[ CDCount++ ] = (int)( i-'a' );
			Msg( __LINE__, __FILE__, "CD drive found - %c:\n", i +'A'-'a' );
		}
	}

	/*--------------------------------------------------------------------------
	**	Catch the case when there are NO CD-ROM drives available
	*/
	if ( CDCount == 0 ) {
		for ( char i = 'a'; i <= 'b'; i++ ) {
			path[0] = i;
			if ( GetDriveType( path ) == DRIVE_CDROM ) {
				CDDrives[ CDCount++ ] = (int)( i-'a' );
			}
		}
	}
	if ( CDCount == 0 ) {
		Msg( __LINE__, __FILE__, "No CD drives found\n");
	}
}

/********************************************************************************
 * GetCDClass -- destructor														*
 *																				*
 * INPUT:		none															*
 *																				*
 * OUTPUT:		none															*
 *																				*
 * WARNINGS:																	*
 *																				*
 * HISTORY:																		*
 *   05/26/1994 SW: Created.													*
 *   12/4/95    ST: fixed for Win95												*
 *==============================================================================*/
GetCDClass::~GetCDClass(void)
{
//	if(cdDrive_addrp.seg)
//		DPMI_real_free(cdDrive_addrp);		// free up those conventional buffers
}

/********************************************************************************
 * GetCDClass -- Get_CD_Drive_For_This_Volume									*
 *																				*
 * INPUT:		char *volume_name												*
 *																				*
 * OUTPUT:		int																*
 *																				*
 * WARNINGS:																	*
 *																				*
 * HISTORY:																		*
 *   10/18/2000 MML: Created.													*
 *==============================================================================*/

int	GetCDClass::Get_CD_Drive_For_This_Volume ( const char *volume_label )
{
	char		volume_name[128] = "";
	int			count = 0;
	char		buffer[128];
	unsigned	misc_dword;
	unsigned	filename_length;
	int			cd_drive;

	CDIndex = 0;

	while( CDIndex < CDCount ) {

		//---------------------------------------------------------------------
		// NOTE: A=0, B=1, C=2, D=3, etc...
		//---------------------------------------------------------------------
		cd_drive = CDDrives[ CDIndex++ ];
		wsprintf( buffer, "%c:\\", 'A' + cd_drive );

		if ( GetVolumeInformation(
				(char const *)buffer,
				&volume_name[0],
				(unsigned long)sizeof(volume_name)-1,
				(unsigned long *)NULL,
				(unsigned long *)&filename_length,
				(unsigned long *)&misc_dword,
				(char *)NULL,
				(unsigned long)0 )) {

			//---------------------------------------------------------------------
			// Windows '95 appears to have a volume name limit of 11 characters.
			// I cannot find a Win32 call that will return the maximum volume name
			// length so the value '11' is hard-coded here and the assumption made
			// that all OS's have this length or better.
			//---------------------------------------------------------------------
			if( WinVersion.Is_Win95()) {
				volume_name[11] = '\0';
			}

			if ( _stricmp( volume_label, volume_name ) == 0) {
				return( cd_drive );
			}

		} else {

#if RTS_DEBUG
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL
			);
			Msg( __LINE__, __FILE__, (LPTSTR)lpMsgBuf );
			LocalFree( lpMsgBuf );
#endif

		}
	}
	return( Get_First_CD_Drive());
}

/********************************************************************************
 * GetCDClass -- Get_Volume_Label												*
 *																				*
 * INPUT:		int  index														*
 *																				*
 * OUTPUT:		char *															*
 *																				*
 * WARNINGS:																	*
 *																				*
 * HISTORY:																		*
 *   10/31/2000 MML: Created.	* Happy Halloween! *							*
 *==============================================================================*/

const char * GetCDClass::Get_Volume_Label	( int index )
{
	if( index >= 0 && index < _Num_Volumes ) {
		return( _CD_Volume_Label[ index ]);
	}
	return( _CD_Volume_Label[0]);
}

/********************************************************************************
 * GetCDClass -- Get_Volume_For_This_CD_Drive									*
 *																				*
 * INPUT:		char *volume_name												*
 *				char *path														*
 *																				*
 * OUTPUT:		int																*
 *																				*
 * WARNINGS:																	*
 *																				*
 * HISTORY:																		*
 *   10/31/2000 MML: Created.	* Happy Halloween! *							*
 *==============================================================================*/

const char *GetCDClass::Get_Volume_For_This_CD_Drive ( const char *path, char *volume_name )
{
	char			buffer[128];
	unsigned		misc_dword;
	unsigned		filename_length;
	static char		volume_label[ MAX_PATH ] = "";	// [OYO] add static

	if ( path == NULL || volume_name == NULL ) {
		return( NULL );
	}

	memset( volume_name, '\0', sizeof( volume_name ));
	wsprintf( buffer, "%c:\\", path[0] );

	if ( GetVolumeInformation(
			(char const *)buffer,
			&volume_label[0],
			(unsigned long)sizeof(volume_label)-1,
			(unsigned long *)NULL,
			(unsigned long *)&filename_length,
			(unsigned long *)&misc_dword,
			(char *)NULL,
			(unsigned long)0 )) {

		strcpy( volume_name, volume_label );

	} else {

		LPVOID lpMsgBuf;
		FormatMessage(
		    FORMAT_MESSAGE_ALLOCATE_BUFFER |
		    FORMAT_MESSAGE_FROM_SYSTEM |
		    FORMAT_MESSAGE_IGNORE_INSERTS,
		    NULL,
		    GetLastError(),
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    (LPTSTR) &lpMsgBuf,
		    0,
		    NULL
		);
		Msg( __LINE__, __FILE__, (LPTSTR)lpMsgBuf );
		LocalFree( lpMsgBuf );

		strcpy( volume_name, _CD_Volume_Label[0] );
	}

	//-------------------------------------------------------------------------
	// Windows '95 appears to have a volume name limit of 11 characters.
	// I cannot find a Win32 call that will return the maximum volume name
	// length so the value '11' is hard-coded here and the assumption made
	// that all OS's have this length or better.
	//-------------------------------------------------------------------------
	bool winversion = WinVersion.Is_Win95();

	if( winversion ) {
		volume_name[11] = '\0';
	}

	return( volume_name );
}

/********************************************************************************
 * CD_VOLUME_VERIFICATION -- Check volume label of the CD in the given drive.	*
 *																				*
 * INPUT:		int drive_number												*
 *	  			char *lable														*
 *																				*
 * OUTPUT:		TRUE/FALSE														*
 *																				*
 * WARNINGS:	None															*
 *																				*
 * HISTORY:																		*
 *    01/11/99 4:20PM MML : Created												*
 *==============================================================================*/

bool CD_Volume_Verification ( int cd_drive, char *volume_label, char *volume_to_find )
{
	char		volume_name[128] = "";
	int			count = 0;
	char		buffer[128];
	unsigned	misc_dword;
	unsigned	filename_length;

	/***************************************************************************
	** Get the volume label. If we get a 'not ready' error then retry for the
	** timeout period.
	*/
	for (;;) {

		//---------------------------------------------------------------------
		// NOTE: A=0, B=1, C=2, D=3, etc...
		//---------------------------------------------------------------------
		wsprintf( buffer, "%c:\\", 'A' + cd_drive );

		if ( GetVolumeInformation(
				(char const *)buffer,
				&volume_name[0],
				(unsigned long)sizeof(volume_name)-1,
				(unsigned long *)NULL,
				(unsigned long *)&filename_length,
				(unsigned long *)&misc_dword,
				(char *)NULL,
				(unsigned long)0 )) {

			/******************************************************************
			** Match the volume label to the list of known volume labels.
			*/

			//---------------------------------------------------------------------
			// Windows '95 appears to have a volume name limit of 11 characters.
			// I cannot find a Win32 call that will return the maximum volume name
			// length so the value '11' is hard-coded here and the assumption made
			// that all OS's have this length or better.
			//---------------------------------------------------------------------
			if( WinVersion.Is_Win95()) {
				volume_name[11] = '\0';
			}

			if ( volume_label != NULL ) {
				strncpy( volume_label, volume_name, 128 );
			}

			if ( _stricmp( volume_to_find, volume_name ) == 0) {
				return TRUE;
			}

			if ( !count ) {
				count++;
			} else {
				return FALSE;
			}

		} else {

			/*********************************************************************
			** Failed to get the volume label on a known CD drive.  If this is a
			** CD changer it may require time to swap the disks so don't return
			** immediately if the error is ROR_NOT_READY
			*/
			return FALSE;
		}
	}
}




/* ==================================================================== */

