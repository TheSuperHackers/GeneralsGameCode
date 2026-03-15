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
 *                     $Archive:: /Renegade Setup/Autorun/CDCNTRL.h                           $*
 *                                                                                             *
 *                      $Author:: Maria_l                                                     $*
 *                                                                                             *
 *                     $Modtime:: 4/20/01 2:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

//#include	"always.h"
//#include	<winmin.h>
#define  STRICT
#include <windows.h>
#include <windowsx.h>
#include	<ctype.h>



class CDControlClass {

	public:
		CDControlClass(void);
		~CDControlClass(void);

		/*
		** Force the CD tray to open on the specified drive.
		*/
		void Force_CD_Eject(int drive);

		/*
		** Prevent the user from ejecting the CD in the specified drive.
		*/
		bool Lock_CD_Tray(int drive);

		/*
		** Allow the user to eject the CD in the specified drive.
		*/
		bool Unlock_CD_Tray(int drive);


	private:
		/*
		** NT functions.
		*/
		bool Eject_CD(char drive);
		HANDLE Open_Removable_Volume(char drive);
		bool Lock_Volume(HANDLE volume);
		bool Unlock_Volume(HANDLE volume);
		bool Dismount_Volume(HANDLE volume);
		bool Prevent_Removal_Of_Volume(HANDLE volume, bool prevent);
		bool Auto_Eject_Volume(HANDLE volume);
		bool Close_Removable_Volume(HANDLE volume);
		bool Lock_CD_Drive(char drive);
		bool Unlock_CD_Drive(char drive);

		/*
		** Win9x functions.
		*/
		HANDLE WINAPI Open_VWin32 (void);
		bool WINAPI Close_VWin32 (HANDLE vwin32);
		bool WINAPI Unlock_Logical_Volume (HANDLE vwin32, char drive);
		bool WINAPI Lock_Logical_Volume (HANDLE vwin32, char drive, char lock_level, short permissions);
		bool Lock_Volume_95 (HANDLE vwin32, char drive);
		bool Unlock_Volume_95 (HANDLE vwin32, char drive);
		bool Auto_Eject_Volume_95 (HANDLE vwin32, char drive);
		void Eject_CD_Win95 (char drive);
		bool Lock_CD_Drive_95(char drive);
		bool Unlock_CD_Drive_95(char drive);

};

extern CDControlClass CDControl;


#define LOCK_TIMEOUT        2000		// 2 Seconds
#define LOCK_RETRIES        2			// 2 times

/*
** Low level structures for Win9x.
**
** DeviceIoControl infrastructure
*/
#if !defined (VWIN32_DIOC_DOS_IOCTL)
#define VWIN32_DIOC_DOS_IOCTL      1

typedef struct _DIOC_REGISTERS {
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} DIOC_REGISTERS, *PDIOC_REGISTERS;

#endif

/*
** Intel x86 processor status flags
*/
#define CARRY_FLAG             0x0001

/*
** DOS IOCTL function support
*/

#pragma pack(1)

/*
** Parameters for locking/unlocking removable media
*/
typedef struct _PARAMBLOCK {
   BYTE bOperation;
   BYTE bNumLocks;
} PARAMBLOCK, *PPARAMBLOCK;

#pragma pack()
