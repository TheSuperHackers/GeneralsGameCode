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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/verchk.cpp                             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/15/01 11:29a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "verchk.h"
#include <windows.h>
#include <winnt.h>
#include "RAWFILE.h"
#include "ffactory.h"


/******************************************************************************
*
* NAME
*     GetVersionInfo
*
* DESCRIPTION
*     Retrieve version information from files version resource.
*
* INPUTS
*     Filename - Name of file to retrieve version information for.
*     FileInfo - Pointer to VS_FIXEDFILEINFO structure to be filled in.
*
* RESULT
*     Success - True if successful in obtaining version information.
*
******************************************************************************/

bool GetVersionInfo(char* filename, VS_FIXEDFILEINFO* fileInfo)
	{
	if (filename == nullptr || fileInfo == nullptr)
		{
		return false;
		}

	// Get version information from the application
	DWORD verHandle;
	DWORD verInfoSize = GetFileVersionInfoSize(filename, &verHandle);

	if (verInfoSize)
		{
		// If we were able to get the information, process it:
		HANDLE memHandle = GlobalAlloc(GMEM_MOVEABLE, verInfoSize);

		if (memHandle)
			{
			LPVOID buffer = GlobalLock(memHandle);

			if (buffer)
				{
				BOOL success = GetFileVersionInfo(filename, verHandle, verInfoSize, buffer);

				if (success)
					{
					VS_FIXEDFILEINFO* data;
					UINT dataSize = 0;
					success = VerQueryValue(buffer, "\\", (LPVOID*)&data, &dataSize);

					if (success && (dataSize == sizeof(VS_FIXEDFILEINFO)))
						{
						memcpy(fileInfo, data, sizeof(VS_FIXEDFILEINFO));
						return true;
						}
					}

				GlobalUnlock(memHandle);
				}

			GlobalFree(memHandle);
			}
		}

	return false;
	}


bool GetFileCreationTime(char* filename, FILETIME* createTime)
	{
	if (filename && createTime)
		{
		createTime->dwLowDateTime = 0;
		createTime->dwHighDateTime = 0;
		FileClass* file = _TheFileFactory->Get_File(filename);

		if (file && file->Open())
			{
			HANDLE handle = file->Get_File_Handle();

			if (handle != INVALID_HANDLE_VALUE)
				{
				if (GetFileTime(handle, nullptr, nullptr, createTime))
					{
					return true;
					}
				}
			}
		}

	return false;
	}


////////////////////////////////////////////////////////////////////////
//
//	Get_Image_File_Header
//
////////////////////////////////////////////////////////////////////////
bool
Get_Image_File_Header (const char *filename, IMAGE_FILE_HEADER *file_header)
{
	bool retval = false;

	//
	//	Attempt to open the file
	//
	FileClass *file=_TheFileFactory->Get_File(filename);

	if (file && file->Open ()) {

		//
		//	Read the dos header (all PE exectuable files begin with this)
		//
		IMAGE_DOS_HEADER dos_header;
		if (file->Read (&dos_header, sizeof (dos_header)) == sizeof (dos_header)) {

			//
			//	Determine the index where the image header resides
			//
			int file_header_offset = dos_header.e_lfanew + sizeof (DWORD);
			file->Seek (file_header_offset, SEEK_SET);

			//
			//	Read the image header from the file
			//
			int size = sizeof (IMAGE_FILE_HEADER);
			if (file->Read (file_header, size) == size) {
				retval = true;
			}
		}
	}

	_TheFileFactory->Return_File(file);
	file=nullptr;

	return retval;
}


////////////////////////////////////////////////////////////////////////
//
//	Get_Image_File_Header
//
////////////////////////////////////////////////////////////////////////
bool
Get_Image_File_Header (HINSTANCE app_instance, IMAGE_FILE_HEADER *file_header)
{
	bool retval = false;

	//
	//	Read the dos header (all PE exectuable files begin with this)
	//
	IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)app_instance;
	if (dos_header != nullptr) {

		//
		//	Determine the offset where the image header resides
		//
		int image_header_offset = dos_header->e_lfanew + sizeof (DWORD);

		//
		//	Copy the file header into the provided structure
		//
		::memcpy (	file_header,
						(((char *)dos_header) + image_header_offset),
						sizeof (IMAGE_FILE_HEADER));
		retval = true;
	}


	return retval;
}


////////////////////////////////////////////////////////////////////////
//
//	Compare_EXE_Version
//
//	Used to compare 2 versions of an executable, the currently executing
// exe and a version saved to disk.  These exe's do not need to have
// a version resource.
//
//	The return value is the same as strcmp, < 0 if the current process is
// older, 0 if they are the same, and > 0 if the current process is newer.
//
////////////////////////////////////////////////////////////////////////
int
Compare_EXE_Version (int app_instance, const char *filename)
{
	int retval = 0;

	//
	//	Get the image header for both executables
	//
	IMAGE_FILE_HEADER header1 = { 0 };
	IMAGE_FILE_HEADER header2 = { 0 };
	if	(	::Get_Image_File_Header ((HINSTANCE)app_instance, &header1) &&
			::Get_Image_File_Header (filename, &header2))
	{
		retval = int(header1.TimeDateStamp - header2.TimeDateStamp);
	}

	return retval;
}
