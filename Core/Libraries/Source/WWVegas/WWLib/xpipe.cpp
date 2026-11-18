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
 *                     $Archive:: /Commando/Code/Library/XPIPE.cpp                            $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/28/98 12:06p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include	"always.h"
#include "XPIPE.h"
#include	<stddef.h>


//---------------------------------------------------------------------------------------------------------
// BufferPipe
//---------------------------------------------------------------------------------------------------------


// The buffered pipe is a pipe terminator. That is, the data never flows onto subsequent pipe
// chains. The data is stored into the buffer previously submitted to the pipe. If the buffer is
// full, no more data is output to the buffer.
int BufferPipe::Put(void const * source, int slen)
{
	int total = 0;

	if (Is_Valid() && source != nullptr && slen > 0) {
		int len = slen;
		if (BufferPtr.Get_Size() != 0) {
			int theoretical_max = BufferPtr.Get_Size() - Index;
			len = (slen < theoretical_max) ? slen : theoretical_max;
		}

		if (len > 0) {
			memmove(((char *)BufferPtr.Get_Buffer()) + Index, source, len);
		}

		Index += len;
//		Length -= len;
//		Buffer = ((char *)Buffer) + len;
		total += len;
	}
	return(total);
}


//---------------------------------------------------------------------------------------------------------
// FilePipe
//---------------------------------------------------------------------------------------------------------

FilePipe::~FilePipe()
{
	if (Valid_File() && HasOpened) {
		HasOpened = false;
		File->Close();
		File = nullptr;
	}
}


// This routine is called when there will be no more data sent through the pipe. It is responsible
// for cleaning up anything it needs to. This is not handled by the destructor, although it serves a
// similar purpose, because pipes are linked together and the destructor order is not easily
// controlled. If the destructors for a pipe chain were called out of order, the result might be
// less than pleasant.
// WARNING: Don't send any more data through the pipe after this routine is called.
int FilePipe::End()
{
	int total = Pipe::End();
	if (Valid_File() && HasOpened) {
		HasOpened = false;
		File->Close();
	}
	return(total);
}


// If the file was not already open, this routine will open it for write.
int FilePipe::Put(void const * source, int slen)
{
	if (Valid_File() && source != nullptr && slen > 0) {
		if (!File->Is_Open()) {
			HasOpened = true;
			File->Open(FileClass::WRITE);
		}

		return(File->Write(source, slen));
	}
	return(0);
}
