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
 *                     $Archive:: /G/wwlib/BUFF.CPP                                           $* 
 *                                                                                             * 
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 4/15/99 10:14a                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   Buffer::Buffer -- Constructor for buffer object.                                          *
 *   Buffer::Buffer -- Copy constructor for buffer object.                                     *
 *   Buffer::Buffer -- Self-allocating constructor for buffer object.                          *
 *   Buffer::Reset -- Clears the buffer object to null state.                                  *
 *   Buffer::operator = -- Assignment operator for the buffer object.                          *
 *   Buffer::~Buffer -- Destructor for buffer object.                                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"BUFF.H"
//#include	<stddef.h>


/***********************************************************************************************
 * Buffer::Buffer -- Constructor for buffer object.                                            *
 *                                                                                             *
 *    This is the normal constructor for a buffer object. The buffer pointer and size are      *
 *    specified as parameters.                                                                 *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the buffer.                                                 *
 *                                                                                             *
 *          size     -- The size of the buffer.                                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   It is possible to construct a Buffer object that has a pointer but a size       *
 *             value of zero. The Buffer object can still be used for its pointer, but it      *
 *             any function that requires a size will fail.                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
Buffer::Buffer(void * buffer, long size) :
	BufferPtr(buffer),
	Size(size),
	IsAllocated(false)
{
	if (buffer == NULL && size > 0) {
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
	if (buffer == NULL && size > 0) {
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
	if (buffer == NULL && size > 0) {
		BufferPtr = W3DNEWARRAY char[size];
		IsAllocated = true;
	}
}


/***********************************************************************************************
 * Buffer::Buffer -- Self-allocating constructor for buffer object.                            *
 *                                                                                             *
 *    This construtor for a buffer object will automatically allocate the bytes necessary      *
 *    to fulfill the size requested. This object is also responsible for deleting the buffer   *
 *    it allocated.                                                                            *
 *                                                                                             *
 * INPUT:   size  -- The size of the buffer to allocated.                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   There is no way to tell if the allocation failed. To verify, call Get_Buffer    *
 *             and compare with NULL.                                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
Buffer::Buffer(long size) :
	BufferPtr(NULL),
	Size(size),
	IsAllocated(false)
{
	if (size > 0) {
		BufferPtr = W3DNEWARRAY char[size];
		IsAllocated = true;
	}
}


/***********************************************************************************************
 * Buffer::Buffer -- Copy constructor for buffer object.                                       *
 *                                                                                             *
 *    This will make a duplicate of the specified buffer object. The ownership of the pointer  *
 *    remains with the original object. This prevents multiple deletion of the same pointer.   *
 *                                                                                             *
 * INPUT:   buffer   -- Reference to the buffer object to be dupilcated.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
Buffer::Buffer(Buffer const & buffer) :
	IsAllocated(false)
{
	BufferPtr = buffer.BufferPtr;
	Size = buffer.Size;
}


/***********************************************************************************************
 * Buffer::operator = -- Assignment operator for the buffer object.                            *
 *                                                                                             *
 *    This will make a duplicate of the buffer object specified. Any buffer pointed to by the  *
 *    left hand buffer will be lost (possibley freed as a result).                             *
 *                                                                                             *
 * INPUT:   buffer   -- Reference to the right hand buffer object.                             *
 *                                                                                             *
 * OUTPUT:  Returns with a reference to the copied buffer object.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
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


/***********************************************************************************************
 * Buffer::~Buffer -- Destructor for buffer object.                                            *
 *                                                                                             *
 *    This destructor will free any buffer it is responsible for.                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
Buffer::~Buffer(void)
{
	Reset();
}


/***********************************************************************************************
 * Buffer::Reset -- Clears the buffer object to null state.                                    *
 *                                                                                             *
 *    This routine will bring the buffer object into a null (newly constructed) state. If      *
 *    there was any buffer allocated or referred to by this object, it will be freed or        *
 *    dereferenced as necessary.                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine will free the buffer if it is responsible for doing so when        *
 *             it is no longer referenced.                                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/07/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void Buffer::Reset(void)
{
	if (IsAllocated) {
		delete [] BufferPtr;
	}
	BufferPtr = NULL;
	Size = 0;
	IsAllocated = false;
}
