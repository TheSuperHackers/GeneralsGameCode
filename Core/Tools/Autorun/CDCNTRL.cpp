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
 *                     $Archive:: /Renegade Setup/Autorun/CDCNTRL.cpp                         $*
 *                                                                                             *
 *                      $Author:: Maria_l                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/18/01 5:33p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"assert.h"
#include	"CDCNTRL.h"
#include	"WinFix.h"
#include	"Wnd_File.h"
#pragma	warning(disable : 4201)
#include	<winioctl.h>
#include	<tchar.h>
#include	<stdio.h>


CDControlClass CDControl;


void Last_Error_Text ( LPCTSTR szPrefix, HRESULT hr );


CDControlClass::CDControlClass()
{
}


CDControlClass::~CDControlClass()
{
}

// The drive number is 0 for a, 1 for b and so on.
void CDControlClass::Force_CD_Eject(int drive)
{
	if ( WinVersion.Is_Win9x() ) {
		Eject_CD_Win95(drive);
	}else{
		Eject_CD(drive);
	}
}

// The drive number is 0 for a, 1 for b and so on.
bool CDControlClass::Lock_CD_Tray (int drive)
{
	if ( WinVersion.Is_Win9x() ) {
		return (Lock_CD_Drive_95(drive));
	}else{
		return (Lock_CD_Drive(drive));
	}
}

// The drive number is 0 for a, 1 for b and so on.
bool CDControlClass::Unlock_CD_Tray (int drive)
{
	if ( WinVersion.Is_Win9x() ) {
		return (Unlock_CD_Drive_95(drive));
	}else{
		return (Unlock_CD_Drive(drive));
	}
}

HANDLE CDControlClass::Open_Removable_Volume( char drive )
{
	assert (WinVersion.Is_WinNT());

	HANDLE		volume;
	unsigned	drivetype;
	char		volume_name[8];
	char		rootname[5];
	unsigned long	access_flags;

	/*----------------------------------------------------------------------------------------
	** Get the drive type to ensure that this is a removable volume.
	*/
	_stprintf (rootname, _TEXT( "%c:\\" ), drive + 'A');
	drivetype = GetDriveType( rootname );

	switch( drivetype ) {

		case DRIVE_REMOVABLE:
			access_flags = GENERIC_READ | GENERIC_WRITE;
			break;

		case DRIVE_CDROM:
			access_flags = GENERIC_READ;
			break;

		default:
//			DebugString ("Attempt to open non-removable volume for locking or ejection\n");
			Msg( __LINE__, TEXT(__FILE__), TEXT("Attempt to open non-removable volume for locking or ejection" ));
			return( INVALID_HANDLE_VALUE );
	}

	/*----------------------------------------------------------------------------------------
	** Get a handle to the volume.
	*/
	_stprintf( volume_name, _TEXT( "\\\\.\\%c:" ), drive + 'A' );
	volume = CreateFile( volume_name, access_flags, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr );

//	assert (volume != INVALID_HANDLE_VALUE);

	if ( volume == INVALID_HANDLE_VALUE ) {
//		DebugString ("Unable to open drive %c: for ejection\n", drive + 'A');
		Msg( __LINE__, TEXT(__FILE__), TEXT("Unable to open drive %c: for ejection"), drive + 'A' - 1 );
	}
	return( volume );
}

bool CDControlClass::Close_Removable_Volume(HANDLE volume)
{
	assert (WinVersion.Is_WinNT());
	return ((CloseHandle(volume)) ? true : false);
}

// Prevent access by other threads to a given volume.
bool CDControlClass::Lock_Volume( HANDLE volume )
{
	assert( WinVersion.Is_WinNT( ));

	unsigned long bytes_returned	= 0;
//	unsigned long sleep_amount		= LOCK_TIMEOUT / LOCK_RETRIES;

	/*
	** Do this in a loop until a timeout period has expired
	*/
	for ( int trycount = 0; trycount < LOCK_RETRIES; trycount++ ) {

		if ( DeviceIoControl( volume, FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &bytes_returned, nullptr )) {
			return( true );
		}
//		Msg( __LINE__, TEXT(__FILE__), TEXT("DeviceIoControl failed to lock volume. Error %d - %s"), GetLastError(), Last_Error_Text());
		Last_Error_Text( _TEXT( "DeviceIoControl failed to lock volume." ), GetLastError());
//		Sleep( sleep_amount );
		Sleep( LOCK_TIMEOUT );
	}
	return( false );
}

// Allow access by other threads to a given volume.
bool CDControlClass::Unlock_Volume(HANDLE volume)
{
	assert( WinVersion.Is_WinNT());

	unsigned long bytes_returned;
//	unsigned long sleep_amount	= LOCK_TIMEOUT / LOCK_RETRIES;

	/*
	** Do this in a loop until a timeout period has expired
	*/
   for ( int trycount = 0; trycount < LOCK_RETRIES; trycount++ ) {

		if ( DeviceIoControl( volume, FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0, &bytes_returned, nullptr )) return( true );
//		DebugString ("DeviceIoControl failed to unlock volume. Error %d - %s\n", GetLastError(), Last_Error_Text());
//		Msg( __LINE__, __FILE__, "DeviceIoControl failed to unlock volume. Error %d - %s", GetLastError(), Last_Error_Text());
		Last_Error_Text( _TEXT( "DeviceIoControl failed to unlock volume." ), GetLastError());
//		Sleep( sleep_amount );
		Sleep( LOCK_TIMEOUT );
	}
	return( false );
}

bool CDControlClass::Dismount_Volume(HANDLE volume)
{
	assert( WinVersion.Is_WinNT());

	unsigned long bytes_returned;
	bool result = ((DeviceIoControl( volume, FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0, &bytes_returned, nullptr )) ? true : false );

	if (result == false) {
//		DebugString ("DeviceIoControl failed to dismount volume. Error %d - %s\n", GetLastError(), Last_Error_Text());
//		Msg( __LINE__, __FILE__, "DeviceIoControl failed to dismount volume. Error %d - %s", GetLastError(), Last_Error_Text());
		Last_Error_Text( _TEXT( "DeviceIoControl failed to dismount volume." ), GetLastError());
	}
	return( result );
}

// Disable the eject button on the given drive.
bool CDControlClass::Prevent_Removal_Of_Volume( HANDLE volume, bool prevent )
{
	assert( WinVersion.Is_WinNT());

	unsigned long bytes_returned;
	PREVENT_MEDIA_REMOVAL pmrbuffer;

	pmrbuffer.PreventMediaRemoval = prevent;

	bool result = ((DeviceIoControl( volume, IOCTL_STORAGE_MEDIA_REMOVAL, &pmrbuffer, sizeof(PREVENT_MEDIA_REMOVAL), nullptr, 0, &bytes_returned, nullptr)) ? true : false);

	if (result == false) {
//		DebugString ("DeviceIoControl failed to prevent media removal. Error %d - %s\n", GetLastError(), Last_Error_Text());
//		Msg( __LINE__, __FILE__, "DeviceIoControl failed to prevent media removal. Error %d - %s", GetLastError(), Last_Error_Text());
		Last_Error_Text( _TEXT( "DeviceIoControl failed to prevent media removal." ), GetLastError());
	}
	return( result );
}

bool CDControlClass::Auto_Eject_Volume(HANDLE volume)
{
	assert (WinVersion.Is_WinNT());
	unsigned long bytes_returned;
	bool result = ((DeviceIoControl( volume, IOCTL_STORAGE_EJECT_MEDIA, nullptr, 0, nullptr, 0, &bytes_returned, nullptr )) ? true : false);

	if (result == false) {
//		DebugString ("DeviceIoControl failed to eject media. Error %d - %s\n", GetLastError(), Last_Error_Text());
//		Msg( __LINE__, __FILE__, "DeviceIoControl failed to eject media. Error %d - %s", GetLastError(), Last_Error_Text());
		Last_Error_Text( TEXT("DeviceIoControl failed to eject media."), GetLastError());
	}
	return (result);
}

bool CDControlClass::Eject_CD(char drive)
{
	assert (WinVersion.Is_WinNT());
	HANDLE volume;
	bool ejected = false;

	volume = Open_Removable_Volume(drive);

	if (volume == INVALID_HANDLE_VALUE) return (false);

	/*
	** Lock and dismount the volume.
	*/
    if (Lock_Volume(volume) && Dismount_Volume(volume)) {

		/*
		** Set prevent removal to false and eject the volume.
		*/
       if (Prevent_Removal_Of_Volume(volume, false) && Auto_Eject_Volume(volume)) {
			ejected = true;
		}
	}

	/*
	** Close the volume so other processes can use the drive.
	*/
	Close_Removable_Volume(volume);

	return (ejected);
}

bool CDControlClass::Lock_CD_Drive(char drive)
{
	assert (WinVersion.Is_WinNT());
	HANDLE volume;
	bool retval = false;

	volume = Open_Removable_Volume(drive);

	if (volume == INVALID_HANDLE_VALUE) return (false);

	/*
	** Lock the volume.
	*/
    if (Lock_Volume(volume)) {

		/*
		** Set prevent removal to false
		*/
		if (Prevent_Removal_Of_Volume(volume, true)) {
			retval = true;
		}
	}

	/*
	** Close the volume so other processes can use the drive.
	*/
	Unlock_Volume(volume);
	Close_Removable_Volume(volume);

	return (true);
}

bool CDControlClass::Unlock_CD_Drive(char drive)
{
	assert (WinVersion.Is_WinNT());
	HANDLE volume;
	bool retval = false;

	volume = Open_Removable_Volume(drive);

	if (volume == INVALID_HANDLE_VALUE) return (false);

	/*
	** Lock the volume.
	*/
    if (Lock_Volume(volume)) {

		/*
		** Set prevent removal to false
		*/
		if (Prevent_Removal_Of_Volume(volume, false)) {
			retval = true;
		}
	}

	/*
	** Close the volume so other processes can use the drive.
	*/
	Unlock_Volume(volume);
	Close_Removable_Volume(volume);

	return (true);
}

// The drive number is 0 for a, 1 for b and so on.
void CDControlClass::Eject_CD_Win95 (char drive)
{
	assert (WinVersion.Is_Win9x());
	HANDLE vwin32 = INVALID_HANDLE_VALUE;

	drive++;
	vwin32 = Open_VWin32 ();
	assert (vwin32 != INVALID_HANDLE_VALUE);

	/*
	** Make sure no other applications are using the drive.
	*/
	bool drive_locked = Lock_Logical_Volume (vwin32, drive, 0, 0);
//	assert (drive_locked);
	if (!drive_locked) {
//		DebugString("Unable to lock volume %c:\n", 'A' + drive - 1);
		Msg( __LINE__, TEXT(__FILE__), TEXT("Unable to lock volume %c: "), 'A' + drive - 1 );
		return;
	}

	/*
	** Make sure there is no software lock keeping the media in the drive.
	*/
	if (!Unlock_Volume_95 (vwin32, drive)) {
//		DebugString("Could not unlock media from drive %c:\n", 'A' + drive - 1);
		Msg( __LINE__, TEXT(__FILE__), TEXT("Could not unlock media from drive %c: "), 'A' + drive - 1 );
		Unlock_Logical_Volume (vwin32, drive);
		return;
	}

	/*
	** Eject!
	*/
	if (!Auto_Eject_Volume_95 (vwin32, drive)) {
//		DebugString("Could not eject media from drive %c:\n", 'A' + drive - 1);
		Msg( __LINE__, TEXT(__FILE__), TEXT("Could not eject media from drive %c: "), 'A' + drive - 1 );
	}

	Unlock_Logical_Volume (vwin32, drive);
	Close_VWin32 (vwin32);
}

// The drive number is 0 for a, 1 for b and so on.
bool CDControlClass::Lock_CD_Drive_95 (char drive)
{
//	DebugString ("CDControlClass::Lock_CD_Drive_95\n");
	Msg( __LINE__, TEXT(__FILE__), TEXT("CDControlClass::Lock_CD_Drive_95." ));

	bool retval = true;
	assert (WinVersion.Is_Win9x());
	HANDLE vwin32 = INVALID_HANDLE_VALUE;

//	DebugString ("Preventing ejection on CD drive %c\n", drive + 'A');
	Msg( __LINE__, TEXT(__FILE__), TEXT("Preventing ejection on CD drive %c: "), 'A' + drive -1 );

	drive++;
	vwin32 = Open_VWin32();
	assert (vwin32 != INVALID_HANDLE_VALUE);

	/*
	** Make sure no other applications are using the drive.
	*/
//	DebugString ("About to lock logical volume to enable exclusive access\n");
	Msg( __LINE__, TEXT(__FILE__), TEXT("About to lock logical volume to enable exclusive access." ));

	bool drive_locked = Lock_Logical_Volume (vwin32, drive, 0, 0);
//	assert (drive_locked);
	if (!drive_locked) {
//		DebugString("Unable to lock volume %c:\n", 'A' + drive - 1);
		Msg( __LINE__, TEXT(__FILE__), TEXT("Unable to lock volume %c:"), 'A' + drive - 1 );
		return(false);
	}
//	DebugString ("Volume locked OK\n");
	Msg( __LINE__, TEXT(__FILE__), TEXT("Volume locked OK." ));

	/*
	** Lock the tray in the closed position.
	*/
//	DebugString ("About to prevent CD tray ejection\n");
	Msg( __LINE__, TEXT(__FILE__), TEXT("About to prevent CD tray ejection." ));

	if (!Lock_Volume_95 (vwin32, drive)) {
//		DebugString("Could not lock CD tray in drive %c:\n", 'A' + drive - 1);
		Msg( __LINE__, TEXT(__FILE__), TEXT("Could not lock CD tray in drive %c:"), 'A' + drive - 1 );
		retval = false;
	}else{
//		DebugString ("CD tray ejection disabled OK\n");
		Msg( __LINE__, TEXT(__FILE__), TEXT("CD tray ejection disabled OK." ));
	}

	Unlock_Logical_Volume (vwin32, drive);
	Close_VWin32 (vwin32);

//	DebugString ("CDControlClass::Lock_CD_Drive_95 returning %s\n", retval ? "true" : "false");
	Msg( __LINE__, TEXT(__FILE__), TEXT("CDControlClass::Lock_CD_Drive_95 returning %s."), retval ? "true" : "false" );

	return (retval);
}

// The drive number is 0 for a, 1 for b and so on.
bool CDControlClass::Unlock_CD_Drive_95 (char drive)
{
//	DebugString ("CDControlClass::Unlock_CD_Drive_95\n");
	Msg( __LINE__, TEXT(__FILE__), TEXT("CDControlClass::Unlock_CD_Drive_95 returning %s." ));

	bool retval = true;
	assert (WinVersion.Is_Win9x());
	HANDLE vwin32 = INVALID_HANDLE_VALUE;

//	DebugString ("Allowing ejection on CD drive %c\n", drive + 'A');
	Msg( __LINE__, TEXT(__FILE__), TEXT("Allowing ejection on CD drive %c."), drive + 'A' - 1 );

	drive++;
	vwin32 = Open_VWin32();
	assert (vwin32 != INVALID_HANDLE_VALUE);

	/*
	** Make sure no other applications are using the drive.
	*/
//	DebugString ("About to lock logical volume to enable exclusive access\n");
	Msg( __LINE__, TEXT(__FILE__), TEXT("About to lock logical volume to enable exclusive access." ));

	bool drive_locked = Lock_Logical_Volume (vwin32, drive, 0, 0);
//	assert (drive_locked);
	if (!drive_locked) {
//		DebugString("Unable to lock volume %c:\n", 'A' + drive - 1);
		Msg( __LINE__, TEXT(__FILE__), TEXT("Unable to lock volume %c:."), 'A' + drive - 1 );
		return(false);
	}
//	DebugString ("Volume locked OK\n");
	Msg( __LINE__, TEXT(__FILE__), TEXT("Volume locked OK." ));

	/*
	** Unlock the tray to allow ejection.
	*/
//	DebugString ("About to allow CD tray ejection\n");
	Msg( __LINE__, TEXT(__FILE__), TEXT("About to allow CD tray ejection." ));

	if (!Unlock_Volume_95 (vwin32, drive)) {
//		DebugString("Could not unlock CD tray in drive %c:\n", 'A' + drive - 1);
		Msg( __LINE__, TEXT(__FILE__), TEXT("Could not unlock CD tray in drive %c:"), 'A' + drive - 1 );
		retval = false;
	}else{
//		DebugString ("CD tray ejection enabled OK\n");
		Msg( __LINE__, TEXT(__FILE__), TEXT("CD tray ejection enabled OK." ));
	}

	Unlock_Logical_Volume (vwin32, drive);
	Close_VWin32 (vwin32);

//	DebugString ("CDControlClass::Unlock_CD_Drive_95 returning %s\n", retval ? "true" : "false");
	Msg( __LINE__, TEXT(__FILE__), TEXT("CDControlClass::Unlock_CD_Drive_95 returning %s."), retval ? "true" : "false" );
	return (retval);
}

bool CDControlClass::Unlock_Volume_95 (HANDLE vwin32, char drive)
{
	assert (WinVersion.Is_Win9x());
	DIOC_REGISTERS regs = {0};
	PARAMBLOCK     unlock_params = {0};
	bool  result;
	unsigned long cb;

	/*
	** First, check the lock status. This way, we'll know the number of pending locks we must unlock.
	*/
	unlock_params.bOperation = 2;   // return lock/unlock status

	regs.reg_EAX = 0x440D;
	regs.reg_EBX = drive;
	regs.reg_ECX = MAKEWORD(0x48, 0x08);
	regs.reg_EDX = (unsigned long)&unlock_params;

	result = (DeviceIoControl (vwin32, VWIN32_DIOC_DOS_IOCTL, &regs, sizeof(regs), &regs, sizeof(regs), &cb, 0)) ? true : false;

	if (result) {

		/*
		** DeviceIoControl succeeded. Now see if the unlock succeeded. It
		** succeeded if the carry flag is not set, or if the carry flag is
		** set but EAX is 0x01 or 0xB0.
		**
		** It failed if the carry flag is set and EAX is not 0x01 or 0xB0.
		**
		** If the carry flag is clear, then unlock succeeded. However, you
		** don't need to set fResult because it is already TRUE when you get
		** in here.
		*/
		if (regs.reg_Flags & CARRY_FLAG) {
			result = (regs.reg_EAX == 0xB0) || (regs.reg_EAX == 0x01);
		}

	} else {
//		DebugString ("DeviceIoControl failed to perform DOS IO control function. Error %d - %s\n", GetLastError(), Last_Error_Text());
//		Msg( __LINE__, __FILE__, "DeviceIoControl failed to perform DOS IO control function. Error %d - %s.", GetLastError(), Last_Error_Text() );
		Last_Error_Text( TEXT("DeviceIoControl failed to perform DOS IO control function.."), GetLastError());
	}

	if (!result) return (false);

	/*
	** Now we have to unlock the media for every time it was locked. This gets us a lock count of
	** 0 and totally unlocked media.
	*/
	for (int i = 0; i < unlock_params.bNumLocks; ++i) {
		unlock_params.bOperation = 1;   // unlock the media
		regs.reg_EAX = 0x440D;
		regs.reg_EBX = drive;
		regs.reg_ECX = MAKEWORD(0x48, 0x08);  // LOCK/UNLOCK
		regs.reg_EDX = (unsigned long)&unlock_params;

		result = (DeviceIoControl (vwin32, VWIN32_DIOC_DOS_IOCTL, &regs, sizeof(regs), &regs, sizeof(regs), &cb, 0)) ? true : false;
		if (result == false) {
//			DebugString ("DeviceIoControl failed to perform DOS IO control function. Error %d - %s\n", GetLastError(), Last_Error_Text());
//			Msg( __LINE__, __FILE__, "DeviceIoControl failed to perform DOS IO control function. Error %d - %s.", GetLastError(), Last_Error_Text() );
			Last_Error_Text( TEXT("DeviceIoControl failed to perform DOS IO control function.."), GetLastError());
		}

		/*
		** See if DeviceIoControl and the lock succeeded
		*/
		result = result && !(regs.reg_Flags & CARRY_FLAG);
		if (!result) break;
	}
	return (result);
}

bool CDControlClass::Lock_Volume_95 (HANDLE vwin32, char drive)
{
	assert (WinVersion.Is_Win9x());
	DIOC_REGISTERS regs = {0};
	PARAMBLOCK     unlock_params = {0};
	bool  result;
	unsigned long cb;

	/*
	** Bring the lock count down to 0.
	*/
	Unlock_Volume_95(vwin32, drive);

	/*
	** Increment the lock count.
	*/
    unlock_params.bOperation = 0;   // lock the media
    regs.reg_EAX = 0x440D;
    regs.reg_EBX = drive;
    regs.reg_ECX = MAKEWORD(0x48, 0x08);  // LOCK/UNLOCK
    regs.reg_EDX = (unsigned long)&unlock_params;

    result = (DeviceIoControl (vwin32, VWIN32_DIOC_DOS_IOCTL, &regs, sizeof(regs), &regs, sizeof(regs), &cb, 0)) ? true : false;
	if (result == false) {
//		DebugString ("DeviceIoControl failed to perform DOS IO control function. Error %d - %s\n", GetLastError(), Last_Error_Text());
//		Msg( __LINE__, __FILE__, "DeviceIoControl failed to perform DOS IO control function. Error %d - %s.", GetLastError(), Last_Error_Text() );
		Last_Error_Text( TEXT("DeviceIoControl failed to perform DOS IO control function."), GetLastError());
	}

	/*
	** See if DeviceIoControl and the lock succeeded
	*/
    result = result && !(regs.reg_Flags & CARRY_FLAG);
	return (result);
}

bool CDControlClass::Auto_Eject_Volume_95 (HANDLE vwin32, char drive)
{
	assert (WinVersion.Is_Win9x());
	DIOC_REGISTERS regs = {0};
	bool  result;
	unsigned long cb;

	regs.reg_EAX = 0x440D;
	regs.reg_EBX = drive;
	regs.reg_ECX = MAKEWORD(0x49, 0x08);		//EJECT

	result = (DeviceIoControl (vwin32, VWIN32_DIOC_DOS_IOCTL, &regs, sizeof(regs), &regs, sizeof(regs), &cb, 0)) ? true : false;
	if (result == false) {
//		DebugString ("DeviceIoControl failed to perform DOS IO control function. Error %d - %s\n", GetLastError(), Last_Error_Text());
//		Msg( __LINE__, __FILE__, "DeviceIoControl failed to perform DOS IO control function. Error %d - %s.", GetLastError(), Last_Error_Text() );
		Last_Error_Text( TEXT("DeviceIoControl failed to perform DOS IO control function."), GetLastError());
	}

	/*
	** See if we ejected OK
	*/
	result = result && !(regs.reg_Flags & CARRY_FLAG);

	return (result);
}

// Opens a handle to VWIN32 to issue low-level disk I/O.
HANDLE WINAPI CDControlClass::Open_VWin32 ()
{
	assert (WinVersion.Is_Win9x());
	HANDLE result = CreateFile ( TEXT("\\\\.\\vwin32"), 0, 0, nullptr, 0, FILE_FLAG_DELETE_ON_CLOSE, nullptr);
	assert (result != INVALID_HANDLE_VALUE);
	return (result);
}

bool WINAPI CDControlClass::Close_VWin32 (HANDLE vwin32)
{
	assert (WinVersion.Is_Win9x());
	return ((CloseHandle (vwin32)) ? true : false);
}

bool WINAPI CDControlClass::Lock_Logical_Volume (HANDLE vwin32, char drive, char lock_level, short permissions)
{
	assert (WinVersion.Is_Win9x());
	bool           result;
	DIOC_REGISTERS regs = {0};
	char           device_cat;  // can be either 0x48 or 0x08
	unsigned long  cb;

	/*
	** lock_level
	**    Can be 0, 1, 2, or 3. Level 0 is an exclusive lock that can only
	**    be taken when there are no open files on the specified drive.
	**    Levels 1 through 3 form a hierarchy where 1 must be taken before
	**    2, which must be taken before 3.
	**
	** permissions
	**    Specifies how the lock will affect file operations when lock levels
	**    1 through 3 are taken. Also specifies whether a formatting lock
	**    should be taken after a level 0 lock.
	*/

	/*
	** Try first with device category 0x48 for FAT32 volumes. If it
	** doesn't work, try again with device category 0x08. If that
	** doesn't work, then the lock failed.
	*/
	device_cat = 0;

	do {
		if (device_cat == 0) {
			device_cat = 0x48;
		}else{
			device_cat = 0x08;
		}

		/*
		** Set up the parameters for the call.
		*/
		regs.reg_EAX = 0x440D;
		regs.reg_EBX = MAKEWORD(drive, lock_level);
		regs.reg_ECX = MAKEWORD(0x4A, device_cat);
		regs.reg_EDX = permissions;

		result = (DeviceIoControl (vwin32, VWIN32_DIOC_DOS_IOCTL, &regs, sizeof(regs), &regs, sizeof(regs), &cb, 0)) ? true : false;
		if (result == false) {
//			DebugString ("DeviceIoControl failed to perform DOS IO control function. Error %d - %s\n", GetLastError(), Last_Error_Text());
//			Msg( __LINE__, __FILE__, "DeviceIoControl failed to perform DOS IO control function. Error %d - %s.", GetLastError(), Last_Error_Text() );
			Last_Error_Text( TEXT("DeviceIoControl failed to perform DOS IO control function."), GetLastError());
		}
		result = result && !(regs.reg_Flags & CARRY_FLAG);

	} while (result == false && device_cat != 0x08);

	return (result);
}

// WARNING: Must be called the same number of times as LockLogicalVolume() to completely unlock a
// volume.
// WARNING: Only the lock owner can unlock a volume.
bool WINAPI CDControlClass::Unlock_Logical_Volume (HANDLE vwin32, char drive)
{
	assert (WinVersion.Is_Win9x());
	bool           result;
	DIOC_REGISTERS regs = {0};
	char           device_cat;  // can be either 0x48 or 0x08
	unsigned long  cb;

	/*
	** Try first with device category 0x48 for FAT32 volumes. If it
	** doesn't work, try again with device category 0x08. If that
	** doesn't work, then the unlock failed.
	*/
	device_cat = 0;

	do {
		if (device_cat == 0) {
			device_cat = 0x48;
		}else{
			device_cat = 0x08;
		}

		/*
		** Set up the parameters for the call.
		*/
		regs.reg_EAX = 0x440D;
		regs.reg_EBX = drive;
		regs.reg_ECX = MAKEWORD(0x6A, device_cat);

		result = (DeviceIoControl (vwin32, VWIN32_DIOC_DOS_IOCTL, &regs, sizeof(regs), &regs, sizeof(regs), &cb, 0)) ? true : false;
		if (result == false) {
//			DebugString ("DeviceIoControl failed to perform DOS IO control function. Error %d - %s\n", GetLastError(), Last_Error_Text());
//			Msg( __LINE__, __FILE__, "DeviceIoControl failed to perform DOS IO control function. Error %d - %s.", GetLastError(), Last_Error_Text());
			Last_Error_Text( TEXT("DeviceIoControl failed to perform DOS IO control function."), GetLastError());
		}
		result = result && !(regs.reg_Flags & CARRY_FLAG);

	} while (result == false && device_cat != 0x08);

	return (result);
}

// Display error messages based on FormatMessage() and GetLastError().

void Last_Error_Text ( LPCTSTR szPrefix, HRESULT hr )
{
	LPVOID		szMessage;
	char		szDisplay[1000];

	if ( hr == S_OK ) {
		_stprintf( szDisplay, TEXT("%s"), szPrefix );
//		MessageBox( nullptr, szDisplay, TEXT("Msg"),0 );
		return;
	}

	if ( HRESULT_FACILITY( hr ) == FACILITY_WIN32 ) {
		hr = HRESULT_CODE( hr );
	}
	FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			nullptr,
			hr,
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			(LPTSTR)&szMessage,
			0,
			nullptr );

	_stprintf( szDisplay, TEXT( "%s: %s(%lx)" ), szPrefix, szMessage, hr );

	Msg( __LINE__, TEXT(__FILE__), TEXT("GetLastError: %s"), szDisplay );
//	MessageBox( nullptr, szDisplay, TEXT( "GetLastError" ), MB_OK );

	LocalFree( szMessage );

}




