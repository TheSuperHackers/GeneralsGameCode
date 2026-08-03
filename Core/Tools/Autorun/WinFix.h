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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Renegade Setup/Autorun/WinFix.h                            $*
 *                                                                                             *
 *                      $Author:: Maria_l                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/07/01 5:57p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   WindowsVersionInfo::Major -- Get the major version of the OS							   *
 *   WindowsVersionInfo::Minor -- Get the minor version of the OS							   *
 *   WindowsVersionInfo::Build -- Get the build level of the OS								   *
 *   WindowsVersionInfo::Info -- Get additional system information							   *
 *   WindowsVersionInfo::Is_Win9x -- Determine if we are running on a non-NT system.		   *
 *   WindowsVersionInfo::Is_Win95 -- Determine if we are running on a Win95 system.			   *
 *   WindowsVersionInfo::Is_Win98 -- Determine if we are running on a Win98 system.			   *
 *   WindowsVersionInfo::Is_WinNT -- Determine if we are running on an NT system.			   *
 *   WindowsVersionInfo::Is_WinNT5 -- Determine if we are running on an NT 5 system.		   *
 *   WindowsVersionInfo::Version	--														   *
 *   WindowsVersionInfo::IsOSR2Release --													   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

/*-----------------------------------------------------------------------------
** Windows Version Information class.  This is a global object that is used to
** store information about the specific OS that we are running under.  This can
** be used to make special allowances for differences between OS's, such as when
** using the registry, or trying to work around a limitaion of a particular OS
** (their APIs are slightly different...)
**-----------------------------------------------------------------------------*/
class WindowsVersionInfo
{
	public:
		WindowsVersionInfo	();
		~WindowsVersionInfo	() {}

		int 	Major								() const { return( MajorVersionNumber ); }
		int 	Minor								() const { return( MinorVersionNumber ); }
		int 	Build								() const { return( BuildNumber ); }
		bool 	Is_Win9x							() const { return( IsWin9x ); }									// Win 9x
		bool 	Is_Win95							() const { return( IsWin95 ); }									// Win 95
		bool 	Is_Win98							() const { return( IsWin98 ); }									// Win 98
		bool 	Is_WinNT							() const { return( IsWinNT ); }									// Win NT
		bool 	Is_WinNT4							() const { return( IsWinNT && MajorVersionNumber == 4 ); }		// Win NT
		bool	Is_WinNT5							() const { return( IsWinNT && MajorVersionNumber == 5 ); }		// Win NT
		bool	Is_Win_2000							() const { return( IsWin2000 ); }								// Win 2000
		bool	Is_Win_XP							() const { return( IsWinXP ); }									// Win XP
		int		Version								() const { return( WindowsVersion );	}
		int		IsOSR2Release						() const { return( RunningOSR2 );		}
		const char * Info							() const { return( &AdditionalInfo[0] ); }
		char *	Version_String						();
		char *	Version_Name						();
		bool	Meets_Minimum_Version_Requirements	();

	private:
		/*-----------------------------------------------------------------------
		** Major version number; i.e. for 4.10.1721 this would be '4'
		*/
		int MajorVersionNumber;

		/*-----------------------------------------------------------------------
		** Minor version number; i.e. for 4.10.1721 this would be '10'
		*/
		int MinorVersionNumber;

		/*-----------------------------------------------------------------------
		** Version number expressed as a DWORD; i.e. for 4.10 this would be '410'
		*/
		int WindowsVersion;

		/*-----------------------------------------------------------------------
		** Build number; i.e. for 4.10.1721 this would be '1721'
		*/
		int BuildNumber;

		/*-----------------------------------------------------------------------
		** Is the system running OSR 2 or later release of Windows95.
		*/
		int RunningOSR2;

		/*-----------------------------------------------------------------------
		** Additional Info; i.e. for NT 4.0 with SP3, this would be
		** the string 'Service Pack 3'
		*/
		char AdditionalInfo[128];

		/*-----------------------------------------------------------------------
		** Windows 9x flag; true if running on non-NT system
		*/
		bool IsWin9x;
		bool IsWin95;
		bool IsWin98;

		/*-----------------------------------------------------------------------
		** Windows NT flag; true if running on Windows NT system
		*/
		bool IsWinNT;

		/*-----------------------------------------------------------------------
		** Windows 2000 (Formerly Windows NT 5.0)
		** As you've no doubt heard by now, Windows NT 5.0 has been officially
		** christened "Windows 2000."
		*/
		bool IsWin2000;

		/*-----------------------------------------------------------------------
		** Windows XP flag; true if running on Windows NT system
		*/
		bool IsWinXP;

		/*-----------------------------------------------------------------------
		** This array is used for formatting the version # as a string
		*/
		char VersionName[30];
};

extern WindowsVersionInfo WinVersion;
