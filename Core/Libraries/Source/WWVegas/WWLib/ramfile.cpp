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
 *                     $Archive:: /Commando/Code/wwlib/ramfile.cpp                            $*
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/31/01 2:36p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include "RAMFILE.h"


// Access to the buffer will appear as if it was accessing a file. This is different from the
// caching ability of the buffered file class in that this file class has no real file counterpart.
// Typical use of this is for algorithms that were originally designed for file processing, but are
// now desired to work with a buffer. The buffer will already contain data if the file is opened for
// READ. It will be considered a scratch buffer if opened for WRITE. If the buffer pointer is
// nullptr but the length is not, then a buffer will be allocated of the specified length. This case
// is only useful for opening the file for WRITE.
RAMFileClass::RAMFileClass(void * buffer, int len) :
	Buffer((char *)buffer),
	MaxLength(len),
	Length(len),
	Offset(0),
	Access(READ),
	IsOpen(false),
	IsAllocated(false)
{
	if (buffer == nullptr && len > 0) {
		Buffer = W3DNEWARRAY char[len];
		IsAllocated = true;
	}
}


// Deallocates any buffer that it allocated. Otherwise it does nothing.
RAMFileClass::~RAMFileClass()
{
	Close();
	if (IsAllocated) {
		delete [] Buffer;
		Buffer = nullptr;
		IsAllocated = false;
	}
}


// Effectively clears the buffer of data. It only makes the buffer appear empty by resetting the
// internal length to zero.
// WARNING: If the file was open, then resetting by this routine is not allowed.
int RAMFileClass::Create()
{
	if (!Is_Open()) {
		Length = 0;
		return(true);
	}
	return(false);
}


// Effectively clears the buffer of data. It only makes the buffer appear empty by resetting the
// internal length to zero.
// WARNING: If the file was open, then resetting by this routine is not allowed.
int RAMFileClass::Delete()
{
	if (!Is_Open()) {
		Length = 0;
		return(true);
	}
	return(false);
}


// RAM files are always available.
bool RAMFileClass::Is_Available(int )
{
	return(true);
}


bool RAMFileClass::Is_Open() const
{
	return(IsOpen);
}


// The name is meaningless so that parameter is ignored. If the access mode is for write, then the
// pseudo-file can be written until the buffer is full. If the file is opened for read, then the
// buffer is presumed to be full of the data to be read.
int RAMFileClass::Open(char const *, int access)
{
	return(Open(access));
}


// If the file is opened for write, the 'file' can be written up to the limit of the buffer's size.
// If the file is opened for read, then the buffer is presumed to hold the data to be read.
int RAMFileClass::Open(int access)
{
	if (Buffer == nullptr || Is_Open()) {
		return(false);
	}

	Offset = 0;
	Access = access;
	IsOpen = true;

	switch (access) {
		default:
		case READ:
			break;

		case WRITE:
			Length = 0;
			break;

		case READ|WRITE:
			break;
	}

	return(Is_Open());
}


// When the ram buffer is exhausted, less bytes than requested will be read.
// WARNING: The read function only applies to ram 'files' opened for read access.
int RAMFileClass::Read(void * buffer, int size)
{
	if (Buffer == nullptr || buffer == nullptr || size == 0) {
		return(0);
	}

	bool hasopened = false;
	if (!Is_Open()) {
		Open(READ);
		hasopened = true;
	} else {
		if ((Access & READ) == 0) {
			return(0);
		}
	}

	int tocopy = (size < (Length-Offset)) ? size : (Length-Offset);
	memmove(buffer, &Buffer[Offset], tocopy);
	Offset += tocopy;

	if (hasopened) {
		Close();
	}

	return(tocopy);
}


int RAMFileClass::Seek(int pos, int dir)
{
	if (Buffer == nullptr || !Is_Open()) {
		return(Offset);
	}

	int maxoffset = Length;
	if ((Access & WRITE) != 0) {
		maxoffset = MaxLength;
	}

	switch (dir) {
		case SEEK_CUR:
			Offset += pos;
			break;

		case SEEK_SET:
			Offset = 0 + pos;
			break;

		case SEEK_END:
			Offset = maxoffset + pos;
			break;
	}

	if (Offset < 0) Offset = 0;
	if (Offset > maxoffset) Offset = maxoffset;

	if (Offset > Length) {
		Length = Offset;
	}

	return(Offset);
}


// The real data is either the entire buffer, if opened for READ, or just the written data if
// opened for WRITE.
int RAMFileClass::Size()
{
	return(Length);
}


// Returns less than requested if the buffer is exhausted of space prematurely.
int RAMFileClass::Write(void const * buffer, int size)
{
	if (Buffer == nullptr || buffer == nullptr || size == 0) {
		return(0);
	}

	bool hasopened = false;
	if (!Is_Open()) {
		Open(WRITE);
		hasopened = true;
	} else {
		if ((Access & WRITE) == 0) {
			return(0);
		}
	}

	int maxwrite = MaxLength - Offset;
	int towrite = (size < maxwrite) ? size : maxwrite;
	memmove(&Buffer[Offset], buffer, towrite);
	Offset += towrite;

	if (Offset > Length) {
		Length = Offset;
	}

	if (hasopened) {
		Close();
	}

	return(towrite);
}


// Closing a ram file actually does nothing but record that it is now closed.
void RAMFileClass::Close()
{
	IsOpen = false;
}


void RAMFileClass::Bias (int start, int length)
{
	Buffer	 = Buffer + start;
	Length	 = MIN (Length, start + length) - start;
	MaxLength =	MIN (MaxLength, start + length) - start;

	if (Is_Open()) {
		Seek (0, SEEK_SET);
	}
}
