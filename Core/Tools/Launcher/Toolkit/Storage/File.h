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
*
* FILE
*     $Archive:  $
*
* DESCRIPTION
*     File definitions (Windows)
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author:  $
*
* VERSION INFO
*     $Modtime:  $
*     $Revision:  $
*
****************************************************************************/

#ifndef FILE_H
#define FILE_H

#include <Support/UTypes.h>
#include "Stream.h"
#include "Rights.h"
#include <Support/UString.h>
#include <windows.h>

class File : public Stream
	{
	public:
		// File Error conditions
		typedef enum
			{
			FileError_None = 0,
			FileError_FNF,
			FileError_Access,
			FileError_Open,
			FileError_Read,
			FileError_Write,
			FileError_Seek,
			FileError_Nomem,
			FileError_Fault,
			} EFileError;

		//! Default constructor - Create an unassociated File
		File();

		//! Name constructor - Create a File with an associated name
		File(const Char* name, ERights rights = Rights_ReadOnly);
		File(const UString& name, ERights rights = Rights_ReadOnly);

		//! Destructor
		virtual ~File();

		//! Retrieve name of file
		const UString& GetName(void) const;

		//! Associate a name to the file
		virtual void SetName(const UString& name);

		//! Retrieve file access rights
		virtual ERights GetRights(void) const;

		//! Set file access rights
		virtual void SetRights(ERights rights);

		//! Check if the file is available
		virtual bool IsAvailable(bool force = false);

		//! Check if te file is open
		virtual bool IsOpen(void) const;

		//! Open the file for access.
		virtual EFileError Open(ERights rights);

		//! Open the file with the associated name for access
		virtual EFileError Open(const UString& name, ERights rights);

		//! Close the file
		virtual void Close(void);

		//! Create a new file
		virtual EFileError Create(void);

		//! Delete an existing file
		virtual EFileError Delete(void);

		//! Load the file into memory
		virtual EFileError Load(void*& outBuffer, UInt32& outSize);

		//! Write file data
		virtual EFileError Save(const void* buffer, UInt32 size);

		//! Error handling hook
		virtual bool OnFileError(EFileError error, bool canRetry);

		//-----------------------------------------------------------------------
		// STREAM INTERFACE
		//-----------------------------------------------------------------------

		//! Get the length of the file
		virtual UInt32 GetLength(void);

		//! Set the length of the file
		virtual void SetLength(UInt32 length);

		//! Get file position marker
		virtual UInt32 GetMarker(void);

		//! Set file position marker
		virtual void SetMarker(Int32 offset, EStreamFrom from);

		//! End of file test
		virtual bool AtEnd(void);

		//! Read bytes from the file
		virtual UInt32 GetBytes(void* ptr, UInt32 bytes);

		//! Write bytes to the file
		virtual UInt32 PutBytes(const void* ptr, UInt32 bytes);

		//! Read bytes from the file without marker adjustment
		virtual UInt32 PeekBytes(void* ptr, UInt32 bytes);

		//! Flush the stream
		virtual void Flush(void);

	private:
		UString mName;
		ERights mRights;
		HANDLE mHandle;
		static const HANDLE INVALID_HANDLE;
	};

#endif // FILE_H
