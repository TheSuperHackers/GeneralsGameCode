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
 *                     $Archive:: /G/wwlib/BUFF.H                                             $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/02/99 11:59a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef CCBUFF_H
#define CCBUFF_H


/*
**	The "bool" integral type was defined by the C++ committee in
**	November of '94. Until the compiler supports this, use the following
**	definition.
*/
//#include	"bool.h"


/*
**	A general purpose buffer pointer handler object. It holds not only the pointer to the
**	buffer, but its size as well. By using this class instead of separate pointer and size
**	values, function interfaces and algorithms become simpler to manage and understand.
*/
class Buffer {
	public:
		Buffer(char * ptr, long size=0);
		Buffer(void * ptr=0, long size=0);
		Buffer(void const * ptr, long size=0);
		Buffer(long size);
		Buffer(Buffer const & buffer);
		~Buffer(void);

		Buffer & operator = (Buffer const & buffer);
		operator void * (void) const {return(BufferPtr);}
		operator char * (void) const {return((char *)BufferPtr);}

		void Reset(void);
		void * Get_Buffer(void) const {return(BufferPtr);}
		long Get_Size(void) const {return(Size);}
		bool Is_Valid(void) const {return(BufferPtr != 0);}

	protected:

		/*
		**	Pointer to the buffer memory.
		*/
		void * BufferPtr;

		/*
		**	The size of the buffer memory.
		*/
		long Size;

		/*
		**	Was the buffer allocated by this class? If so, then this class
		**	will be responsible for freeing the buffer.
		*/
		bool IsAllocated;
};


#endif
