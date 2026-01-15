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

/******************************************************************************
*
* FILE
*     $Archive:  $
*
* DESCRIPTION
*     File functionality for WIN32
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author:  $
*
* VERSION INFO
*     $Modtime:  $
*     $Revision:  $
*
******************************************************************************/

#include "File.h"
#include <Debug/DebugPrint.h>
#include <assert.h>

// This should be whatever the file system uses for invalid files
const HANDLE File::INVALID_HANDLE = INVALID_HANDLE_VALUE;

/******************************************************************************
*
* NAME
*     File
*
* DESCRIPTION
*     Default constructor
*
* INPUTS
*     NONE
*
* RETURN
*     NONE
*
******************************************************************************/

File::File()
	: mRights(Rights_ReadOnly),
		mHandle(INVALID_HANDLE)
	{
	}


/******************************************************************************
*
* NAME
*     File
*
* DESCRIPTION
*     Create a file instance with the specified name and access rights.
*
* INPUTS
*     Name   - Name of file
*     Rights - Access rights
*
* RETURN
*     NONE
*
******************************************************************************/

File::File(const Char* name, ERights rights)
	: mRights(rights),
		mHandle(INVALID_HANDLE)
	{
	SetName(name);
	}


/******************************************************************************
*
* NAME
*     File
*
* DESCRIPTION
*     Create a file instance with the specified name and access rights.
*
* INPUTS
*     Name   - Name of file
*     Rights - Access rights
*
* RETURN
*     NONE
*
******************************************************************************/

File::File(const UString& name, ERights rights)
	: mName(name),
	  mRights(rights),
		mHandle(INVALID_HANDLE)
	{
	}


/******************************************************************************
*
* NAME
*     ~File
*
* DESCRIPTION
*     Destroy a file representation.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

File::~File()
	{
	// Close any open file.
	Close();
	}


/******************************************************************************
*
* NAME
*     GetName
*
* DESCRIPTION
*     Retrieve name of file
*
* INPUTS
*     NONE
*
* RETURN
*     Name - Name of file.
*
******************************************************************************/

const UString& File::GetName(void) const
	{
	return mName;
	}


/******************************************************************************
*
* NAME
*     SetName
*
* DESCRIPTION
*     Associate a filename to the file class. Any operations will be performed
*     on the associated file with the specified name.
*
* INPUTS
*     Filename - Name of file to associate.
*
* RESULT
*     NONE
*
******************************************************************************/

void File::SetName(const UString& name)
	{
	mName = name;
	}


/******************************************************************************
*
* NAME
*     GetRights
*
* DESCRIPTION
*     Retrieve file access rights
*
* INPUTS
*     NONE
*
* RETURN
*     Rights - Current access rights for file
*
******************************************************************************/

ERights File::GetRights(void) const
	{
	return mRights;
	}


/******************************************************************************
*
* NAME
*     SetRights
*
* DESCRIPTION
*     Set file access rights
*
* INPUTS
*     Rights - New rights
*
* RESULT
*     NONE
*
******************************************************************************/

void File::SetRights(ERights rights)
	{
	mRights = rights;
	}


/******************************************************************************
*
* NAME
*     IsAvailable
*
* DESCRIPTION
*     Checks if the associated file is available for access. If the force
*     flag is true then the file is check for availabilty until it is found
*     or the proceedure is aborted via OnError().
*
* INPUTS
*     Force - Force file availability flag.
*
* RESULT
*     Success - Success/Failure condition flag.
*
******************************************************************************/

bool File::IsAvailable(bool force)
	{
	// If the filename is invalid then the file is not available.
	if (mName.Length() == 0)
		{
		return false;
		}

	// If the file is open then it must be available.
	if (IsOpen())
		{
		return true;
		}

	// If this is a forced open then go through the standard file open
	// procedure so that the abort/retry logic gets called.
	if (force == true)
		{
		Open(GetRights());
		Close();
		return true;
		}

	char name[MAX_PATH];
	mName.ConvertToANSI(name, sizeof(name));

	// Attempt to open the file
	mHandle = CreateFile(name, GENERIC_READ, FILE_SHARE_READ,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	// If the open failed then the file is not available.
	if (mHandle == INVALID_HANDLE)
		{
		return false;
		}

	// Close the file.
	CloseHandle(mHandle);
	mHandle = INVALID_HANDLE;

	return true;
	}


/******************************************************************************
*
* NAME
*     IsOpen
*
* DESCRIPTION
*     Check if the file is open.
*
* INPUTS
*     NONE
*
* RETURN
*     Opened - File is opened if true; otherwise false
*
******************************************************************************/

bool File::IsOpen(void) const
	{
	return ((mHandle != INVALID_HANDLE) ? true : false);
	}


/******************************************************************************
*
* NAME
*     Open
*
* DESCRIPTION
*     Opens the file associated with this file class.
*
* INPUTS
*     Rights - Access rights for the file to open.
*
* RESULT
*     Success - Success/Failure condition flag.
*
******************************************************************************/

File::EFileError File::Open(ERights rights)
	{
	// Close any currently opened file
	Close();

	// If the filename is invalid then there is no way to open the file.
	if (mName.Length() == 0)
		{
		OnFileError(FileError_FNF, false);
		return FileError_FNF;
		}

	mRights = rights;

	char name[MAX_PATH];
	mName.ConvertToANSI(name, sizeof(name));

	// Continue attempting open until successful or aborted.
	for (;;)
		{
		switch (rights)
			{
			// Read only access
			case Rights_ReadOnly:
				mHandle = CreateFile(name, GENERIC_READ, FILE_SHARE_READ,
						nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			break;

			// Write only access
			case Rights_WriteOnly:
				mHandle = CreateFile(name, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			break;

			// Read and Write access
			case Rights_ReadWrite:
				mHandle = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0,
						nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			break;

			// Unknown rights access violation
			default:
				OnFileError(FileError_Access, false);
				return FileError_Access;
			break;
			}

		// Check for open failure
		if (mHandle == INVALID_HANDLE)
			{
			PrintWin32Error("File::Open(%S)", mName.Get());

			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				{
				if (OnFileError(FileError_FNF, true) == false)
					{
					return FileError_FNF;
					}
				else
					{
					continue;
					}
				}
			else
				{
				OnFileError(FileError_Open, false);
				return FileError_Open;
				}
			}

		break;
		}

	return FileError_None;
	}


/******************************************************************************
*
* NAME
*     Open
*
* DESCRIPTION
*     Associate a file then open that file.
*
* INPUTS
*     Filename - Name of file to open.
*     Rights   - Access rights to file.
*
* RESULT
*     Success - Success/Failure condition flag.
*
******************************************************************************/

File::EFileError File::Open(const UString& name, ERights rights)
	{
	// Associate the file then attempt to open it.
	SetName(name);
	return Open(rights);
	}


/******************************************************************************
*
* NAME
*     Close
*
* DESCRIPTION
*     Close access to the file.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void File::Close(void)
	{
	if (IsOpen())
		{
		CloseHandle(mHandle);
		mHandle = INVALID_HANDLE;
		}
	}


/******************************************************************************
*
* NAME
*     Create
*
* DESCRIPTION
*     Creates a file on the local file system with a filename previously
*     associated with a call to SetName().
*
* INPUTS
*     NONE
*
* RESULT
*     Success - Success/Failure condition flag.
*
******************************************************************************/

File::EFileError File::Create(void)
	{
	// Close any previous handle
	Close();

	// Open the file for writing
	EFileError error = Open(Rights_WriteOnly);

	if (error == FileError_None)
		{
		Close();
		}

	return error;
	}


/******************************************************************************
*
* NAME
*     Delete
*
* DESCRIPTION
*     Deletes the file on the local file system with the name previously
*     associated by calling SetName().
*
* INPUTS
*     NONE
*
* RESULT
*     Success - Success/Failure condition flag.
*
******************************************************************************/

File::EFileError File::Delete(void)
	{
	// Make sure the file is closed.
	Close();

	// The file must be available before it can be deleted.
	if (IsAvailable() == false)
		{
		return FileError_FNF;
		}
	else
		{
		// Delete the file. Pass error conditions to OnError()
		char name[MAX_PATH];
		mName.ConvertToANSI(name, sizeof(name));

		if (!DeleteFile(name))
			{
			OnFileError(FileError_Fault, false);
			return FileError_Fault;
			}
		}

	return FileError_None;
	}


/******************************************************************************
*
* NAME
*     Load
*
* DESCRIPTION
*     The entire contents of the file is loaded into the specified buffer.
*
* INPUTS
*     outBuffer - Buffer with file contents.
*
* RESULT
*
******************************************************************************/

File::EFileError File::Load(void*& outBuffer, UInt32& outSize)
	{
	outBuffer = nullptr;
	outSize = 0;

	// Enforce access control
	if (mRights == Rights_WriteOnly)
		{
		OnFileError(FileError_Access, false);
		return FileError_Access;
		}

	EFileError result = FileError_None;

	// Open the file here if it isn't already.
	bool openedHere = false;

	if (IsOpen() == false)
		{
		result = Open(GetRights());

		if (result != FileError_None)
			{
			return result;
			}

		// The file was opened here.
		openedHere = true;
		}

	// Get the size of the file
	UInt32 size = GetFileSize(mHandle, nullptr);

	if (size == 0xFFFFFFFF)
		{
		PrintWin32Error("File::Load(%S) - GetFileSize", mName.Get());
		result = FileError_Fault;
		OnFileError(result, false);
		}

	if (result == FileError_None)
		{
		// Allocate buffer to hold file contents
		outBuffer = malloc(size);
		outSize = size;

		// If allocation succeded then load file data.
		if (outBuffer != nullptr)
			{
			// Fill the buffer with the file contents
			while (size > 0)
				{
				unsigned long bytesRead = 0;

				// Read in some bytes.
				if (ReadFile(mHandle, outBuffer, size, &bytesRead, nullptr) == 0)
					{
					result = FileError_Read;

					if (OnFileError(result, true) == false)
						{
						break;
						}

					result = FileError_None;
					}

				size -= bytesRead;

				if (bytesRead == 0)
					{
					break;
					}
				}
			}
		else
			{
			result = FileError_Nomem;
			OnFileError(result, false);
			}
		}

	// Close the file if we opened it here.
	if (openedHere == true)
		{
		Close();
		}

	return result;
	}


/******************************************************************************
*
* NAME
*     Save
*
* DESCRIPTION
*     Save data to the file.
*
* INPUTS
*     Buffer - Pointer to data to write.
*     Size   - Number of bytes to write.
*
* RESULT
*
******************************************************************************/

File::EFileError File::Save(const void* buffer, UInt32 size)
	{
	// Enforce access control
	if (mRights == Rights_ReadOnly)
		{
		OnFileError(FileError_Access, false);
		return FileError_Access;
		}

	EFileError result = FileError_None;

	// Open the file if it isn't already.
	bool openedHere = false;

	if (IsOpen() == false)
		{
		result = Open(GetRights());

		if (result == FileError_None)
			{
			openedHere = true;
			}
		}

	if (result == FileError_None)
		{
		SetMarker(0, EStreamFrom::FromStart);

		// Write the data to the file.
		unsigned long bytesWritten = 0;

		if (WriteFile(mHandle, buffer, size, &bytesWritten, nullptr) == 0)
			{
			result = FileError_Write;
			OnFileError(result, false);
			}
		else
			{
			SetEndOfFile(mHandle);
			}
		}

	// If we opened the file here then we must close it here.
	if (openedHere == true)
		{
		Close();
		}

	// Return the number of actual butes written.
	return result;
	}


/******************************************************************************
*
* NAME
*     OnFileError(Error, CanRetry)
*
* DESCRIPTION
*
* INPUTS
*     Error    - Error condition code.
*     CanRetry - Retry allowed flag. If this is false then retrys are not
*                permitted for this failure condition.
*
* RESULT
*     Retry - Retry flag.
*
******************************************************************************/
#pragma warning(disable:4100)

bool File::OnFileError(EFileError error, bool)
	{
	#ifdef RTS_DEBUG
	const char* _errorNames[] =
		{
		"FileError_None",
		"FileError_FNF",
		"FileError_Access",
		"FileError_Open",
		"FileError_Read",
		"FileError_Write",
		"FileError_Seek",
		"FileError_Nomem",
		"FileError_Fault",
		};

	DebugPrint("File::OnFileError(%S) - %s\n", mName.Get(), _errorNames[error]);
	#endif

	return false;
	}


/******************************************************************************
*
* NAME
*     GetLength
*
* DESCRIPTION
*     Get the size of the file and return it to the caller.
*
* INPUTS
*     NONE
*
* RESULT
*     Length of the file in bytes.
*
******************************************************************************/

UInt32 File::GetLength(void)
	{
	// If the file is not open then it must be opened first.
	bool openedHere = false;

	if (IsOpen() == false)
		{
		if (Open(GetRights()) != FileError_None)
			{
			return 0xFFFFFFFF;
			}

		openedHere = true;
		}

	UInt32 length = GetFileSize(mHandle, nullptr);

	if (length == 0xFFFFFFFF)
		{
		OnFileError(FileError_Fault, false);
		}

	// If the file was opened here then we need to close it.
	if (openedHere == true)
		{
		Close();
		}

	return length;
	}


/******************************************************************************
*
* NAME
*     SetLength
*
* DESCRIPTION
*     Set the length of the streamable file.
*
* INPUTS
*     Length - Length of stream in bytes
*
* RESULT
*     NONE
*
******************************************************************************/

void File::SetLength(UInt32 length)
	{
	// Get the current file position
	UInt32 position = SetFilePointer(mHandle, 0, nullptr, FILE_CURRENT);

	// Extend the file size by positioning the Win32 file pointer to the
	// specified location then setting the end of file.
	SetFilePointer(mHandle, length, nullptr, FILE_BEGIN);
	SetEndOfFile(mHandle);

	// Restore file position
	SetFilePointer(mHandle, position, nullptr, FILE_BEGIN);
	}


/******************************************************************************
*
* NAME
*     GetMarker
*
* DESCRIPTION
*     Returns the current position in the file stream.
*
* INPUTS
*     NONE
*
* RESULT
*     Current position in file
*
******************************************************************************/

UInt32 File::GetMarker(void)
	{
	return SetFilePointer(mHandle, 0, nullptr, FILE_CURRENT);
	}


/******************************************************************************
*
* NAME
*     SetMarker
*
* DESCRIPTION
*     Position the marker within the file stream.
*
* INPUTS
*     Offset - Offset to adjust marker by
*     From   -
*
* RESULT
*     NONE
*
******************************************************************************/

void File::SetMarker(Int32 offset, EStreamFrom from)
	{
	// The file must be open before we can seek into it.
	if (IsOpen() == false)
		{
		OnFileError(FileError_Seek, false);
		}
	else
		{
		unsigned long dir;

		switch (from)
			{
			default:
			case Stream::FromStart:
				dir = FILE_BEGIN;
			break;

			case Stream::FromMarker:
				dir = FILE_CURRENT;
			break;

			case Stream::FromEnd:
				dir = FILE_END;
			break;
			}

		offset = SetFilePointer(mHandle, offset, nullptr, dir);

		if (offset == 0xFFFFFFFF)
			{
			OnFileError(FileError_Seek, false);
			}
		}
	}


/******************************************************************************
*
* NAME
*     AtEnd
*
* DESCRIPTION
*     Test if at the end of stream.
*
* INPUTS
*     NONE
*
* RESULT
*     End - True if at end; otherwise False.
*
******************************************************************************/

bool File::AtEnd(void)
	{
	return (GetMarker() >= GetLength());
	}


/******************************************************************************
*
* NAME
*     GetBytes
*
* DESCRIPTION
*     Retrieve an arbitrary number of bytes from the file stream.
*
* INPUTS
*     Buffer - Pointer to buffer to receive data
*     Bytes  - Number of bytes to read
*
* RESULT
*     Read - Number of bytes actually read.
*
******************************************************************************/

UInt32 File::GetBytes(void* ptr, UInt32 bytes)
	{
	// Parameter check; Null pointers are bad!
	assert(ptr != nullptr);

	// Enforce rights control
	if (GetRights() == Rights_WriteOnly)
		{
		OnFileError(FileError_Access, false);
		return 0;
		}

	// Open the file if it isn't already
	if (IsOpen() == false)
		{
		if (Open(GetRights()) != FileError_None)
			{
			return 0;
			}
		}

	// Zero total bytes read count
	UInt32 totalRead = 0;
	UInt32 bytesToRead = bytes;

	while (bytesToRead > 0)
		{
		unsigned long read;

		if (ReadFile(mHandle, ptr, bytesToRead, &read, nullptr) == 0)
			{
			if (OnFileError(FileError_Read, true) == false)
				{
				return 0;
				}
			}

		// End of file condition?
		if (read == 0)
			{
			break;
			}

		// Adjust counts
		bytesToRead -= read;
		totalRead += read;
		}

	return totalRead;
	}


/******************************************************************************
*
* NAME
*     PutBytes(Buffer, Bytes)
*
* DESCRIPTION
*     Write an arbitrary number of bytes to the stream
*
* INPUTS
*     Buffer - Pointer to buffer containing data to write
*     Bytes  - Number of bytes to transfer
*
* RESULT
*     Actual number of bytes written
*
******************************************************************************/

UInt32 File::PutBytes(const void* ptr, UInt32 bytes)
	{
	// Parameter check; Null pointers are bad!
	assert(ptr != nullptr);

	// Enforce access control
	if (GetRights() == Rights_ReadOnly)
		{
		OnFileError(FileError_Access, false);
		return 0;
		}

	// The file must already be open.
	if (IsOpen() == false)
		{
		if (Open(GetRights()) != FileError_None)
			{
			return 0;
			}
		}

	// Zero total bytes written count
	UInt32 totalWritten = 0;
	UInt32 bytesToWrite = bytes;

	while (bytesToWrite > 0)
		{
		unsigned long written;

		if (WriteFile(mHandle, ptr, bytes, &written, nullptr) == 0)
			{
			if (OnFileError(FileError_Write, true) == false)
				{
				return 0;
				}
			}

		bytesToWrite -= written;
		totalWritten += written;
		}

	return totalWritten;
	}


/******************************************************************************
*
* NAME
*     PeekBytes(Buffer, Bytes)
*
* DESCRIPTION
*     Retrieve bytes from the stream without moving the position marker.
*
* INPUTS
*     Buffer - Pointer to buffer to receive data
*     Bytes  - Number of bytes to retrieve
*
* RESULT
*     Actual number of bytes transfered
*
******************************************************************************/

UInt32 File::PeekBytes(void* ptr, UInt32 bytes)
	{
	// Parameter check; nullptr pointers are bad!
	assert(ptr != nullptr);

	// Get current position
	UInt32 pos = GetMarker();

	// Get bytes
	UInt32 bytesPeeked = GetBytes(ptr, bytes);

	// Restore previous position
	SetMarker(pos, Stream::FromStart);

	return bytesPeeked;
	}


/******************************************************************************
*
* NAME
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void File::Flush(void)
	{
	}
