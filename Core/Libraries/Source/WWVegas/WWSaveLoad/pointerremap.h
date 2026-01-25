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
 *                 Project Name : WWSaveLoad                                                   *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwsaveload/pointerremap.h                    $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 4/30/01 1:54p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifdef _UNIX
#include "osdep/osdep.h"
#endif

#include "always.h"
#include "Vector.h"

class RefCountClass;


class PointerRemapClass
{
	public:

		PointerRemapClass(void);
		~PointerRemapClass(void);

		void		Reset(void);
		void		Process(void);

		void		Register_Pointer (void *old_pointer, void *new_pointer);

#ifdef WWDEBUG
		void		Request_Pointer_Remap (void **pointer_to_convert,const char * file,int line);
		void		Request_Ref_Counted_Pointer_Remap (RefCountClass **pointer_to_convert,const char * file, int line);
#else
		void		Request_Pointer_Remap (void **pointer_to_convert);
		void		Request_Ref_Counted_Pointer_Remap (RefCountClass **pointer_to_convert);
#endif

	private:

		struct PtrPairStruct
		{
			PtrPairStruct(void) {}
			PtrPairStruct(void * oldptr,void * newptr) : OldPointer(oldptr),NewPointer(newptr) {}
			bool operator == (const PtrPairStruct & that) { return ((OldPointer == that.OldPointer) && (NewPointer == that.NewPointer)); }
			bool operator != (const PtrPairStruct & that) { return !(*this == that); }

			void *		OldPointer;
			void *		NewPointer;
		};

		struct PtrRemapStruct
		{
			PtrRemapStruct(void) {}
			bool operator == (const PtrRemapStruct & that) { return (PointerToRemap == that.PointerToRemap); }
			bool operator != (const PtrRemapStruct & that) { return !(*this == that); }

			void **			PointerToRemap;
#ifdef WWDEBUG
			const char *	File;
			int				Line;
#endif
		};

		void		Process_Request_Table(DynamicVectorClass<PtrRemapStruct> & request_table,bool refcount);
		static int __cdecl ptr_pair_compare_function(void const * ptr1, void const * ptr2);
		static int __cdecl ptr_request_compare_function(void const * ptr1, void const * ptr2);

		/*
		**	Array of pointers associated with ID values to assist in swizzling.
		*/
		DynamicVectorClass<PtrPairStruct>	PointerPairTable;
		DynamicVectorClass<PtrRemapStruct>	PointerRequestTable;
		DynamicVectorClass<PtrRemapStruct>	RefCountRequestTable;
};
