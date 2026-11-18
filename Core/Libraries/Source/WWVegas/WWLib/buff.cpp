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
 *                     $Archive:: /G/wwlib/BUFF.cpp                                           $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/15/99 10:14a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include "BUFF.h"
//#include	<stddef.h>


// WARNING: It is possible to construct a Buffer object that has a pointer but a size value of zero.
// The Buffer object can still be used for its pointer, but any function that requires a size will
// fail.
Buffer::Buffer(void * buffer, long size) :
	BufferPtr(buffer),
	Size(size),
	IsAllocated(false)
{
	if (buffer == nullptr && size > 0) {
		BufferPtr = W3DNEWARRAY char[size];
		IsAllocated = true;
	}
}


// Alternate constructor for char * pointer.
Buffer::Buffer(char * buffer, long size) :
	BufferPtr(buffer),
	Size(size),
	IsAllocated(false)
{
	if (buffer == nullptr && size > 0) {
		BufferPtr = W3DNEWARRAY char[size];
		IsAllocated = true;
	}
}


// Alternate constructor for void const * pointer.
Buffer::Buffer(void const * buffer, long size) :
	BufferPtr((void*)buffer),
	Size(size),
	IsAllocated(false)
{
	if (buffer == nullptr && size > 0) {
		BufferPtr = W3DNEWARRAY char[size];
		IsAllocated = true;
	}
}


// Self-allocating constructor for buffer object. This object is responsible for deleting the buffer
// it allocated.
// WARNING: There is no way to tell if the allocation failed. To verify, call Get_Buffer and compare
// with nullptr.
Buffer::Buffer(long size) :
	BufferPtr(nullptr),
	Size(size),
	IsAllocated(false)
{
	if (size > 0) {
		BufferPtr = W3DNEWARRAY char[size];
		IsAllocated = true;
	}
}


// The ownership of the pointer remains with the original object. This prevents multiple deletion of
// the same pointer.
Buffer::Buffer(Buffer const & buffer) :
	IsAllocated(false)
{
	BufferPtr = buffer.BufferPtr;
	Size = buffer.Size;
}


// Any buffer pointed to by the left hand buffer will be lost (possibly freed as a result).
Buffer & Buffer::operator = (Buffer const & buffer)
{
	if (&buffer != this) {
		if (IsAllocated) {
			delete [] BufferPtr;
		}
		IsAllocated = false;
		BufferPtr = buffer.BufferPtr;
		Size = buffer.Size;
	}
	return(*this);
}


// Frees any buffer it is responsible for.
Buffer::~Buffer()
{
	Reset();
}


// If there was any buffer allocated or referred to by this object, it will be freed or dereferenced
// as necessary.
void Buffer::Reset()
{
	if (IsAllocated) {
		delete [] BufferPtr;
	}
	BufferPtr = nullptr;
	Size = 0;
	IsAllocated = false;
}
