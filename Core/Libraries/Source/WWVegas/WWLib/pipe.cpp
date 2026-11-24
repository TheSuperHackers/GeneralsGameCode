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
 *                     $Archive:: /G/wwlib/PIPE.cpp                                           $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/15/99 10:15a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include	"always.h"
#include "PIPE.h"
#include	<stddef.h>


// Unlinks itself from any other pipes that it may be chained to. In the process, it will flush any
// output it may have pending.
Pipe::~Pipe()
{
	if (ChainTo != nullptr) {
		ChainTo->ChainFrom = ChainFrom;
	}
	if (ChainFrom != nullptr) {
		ChainFrom->Put_To(ChainTo);
	}

	ChainFrom = nullptr;
	ChainTo = nullptr;
}


// The specified pipe will be fed data from this pipe.
void Pipe::Put_To(Pipe * pipe)
{
	if (ChainTo != pipe) {
		if (pipe != nullptr && pipe->ChainFrom != nullptr) {
			pipe->ChainFrom->Put_To(nullptr);
			pipe->ChainFrom = nullptr;
		}

		if (ChainTo != nullptr) {
			ChainTo->ChainFrom = nullptr;
			ChainTo->Flush();
		}

		ChainTo = pipe;
		if (ChainTo != nullptr) {
			ChainTo->ChainFrom = this;
		}
	}
}


// It is guaranteed to accept data as fast as you can supply it.
int Pipe::Put(void const * source, int length)
{
	if (ChainTo != nullptr) {
		return(ChainTo->Put(source, length));
	}
	return(length);
}


// Since pipe segments might have internal staging buffer for the data, this routine is necessary to
// force all staging buffers to be clear. This routine is called when the pipe is being destroyed.
int Pipe::Flush()
{
	if (ChainTo != nullptr) {
		return(ChainTo->Flush());
	}
	return(0);
}


