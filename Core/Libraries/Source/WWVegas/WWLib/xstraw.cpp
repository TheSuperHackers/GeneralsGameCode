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
 *                     $Archive:: /Commando/Code/Library/XSTRAW.cpp                           $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/28/98 12:06p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include "XSTRAW.h"
#include	<stddef.h>

//---------------------------------------------------------------------------------------------------------
// BufferStraw
//---------------------------------------------------------------------------------------------------------


// The holding tank is set up by the straw's constructor. Returns fewer bytes than requested when
// the holding tank is exhausted.
int BufferStraw::Get(void * source, int slen)
{
	int total = 0;

	if (Is_Valid() && source != nullptr && slen > 0) {
		int len = slen;
		if (BufferPtr.Get_Size() != 0) {
			int theoretical_max = BufferPtr.Get_Size() - Index;
			len = (slen < theoretical_max) ? slen : theoretical_max;
		}

		if (len > 0) {
			memmove(source, ((char*)BufferPtr.Get_Buffer()) + Index, len);
		}

		Index += len;
//		Length -= len;
//		BufferPtr = ((char *)BufferPtr) + len;
		total += len;
	}
	return(total);
}


//---------------------------------------------------------------------------------------------------------
// FileStraw
//---------------------------------------------------------------------------------------------------------


// The file is specified in the straw's constructor. Returns fewer bytes than requested when the
// file is exhausted.
int FileStraw::Get(void * source, int slen)
{
	if (Valid_File() && source != nullptr && slen > 0) {
		if (!File->Is_Open()) {
			HasOpened = true;
			if (!File->Is_Available()) return(0);
			if (!File->Open(FileClass::READ)) return(0);
		}

		return(File->Read(source, slen));
	}
	return(0);
}


// This destructor only needs to close the file if it was the one to open it.
FileStraw::~FileStraw()
{
	if (Valid_File() && HasOpened) {
		File->Close();
		HasOpened = false;
		File = nullptr;
	}
}
