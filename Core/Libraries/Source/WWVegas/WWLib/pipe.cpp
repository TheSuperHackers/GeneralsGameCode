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
 *                     $Archive:: /G/wwlib/PIPE.CPP                                           $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/15/99 10:15a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Pipe::Put_To -- Connect a pipe to flow data into from this pipe.                          *
 *   Pipe::Flush -- Flush all pending data out the pipe.                                       *
 *   Pipe::Put -- Feed some data through the pipe.                                             *
 *   Pipe::~Pipe -- Destructor for pipe class object.                                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include	"always.h"
#include	"PIPE.H"
#include	<stddef.h>


/***********************************************************************************************
 * Pipe::~Pipe -- Destructor for pipe class object.                                            *
 *                                                                                             *
 *    This destructor will unlink itself from any other pipes that it may be chained to. In    *
 *    the process, it will flush any output it may have pending.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
Pipe::~Pipe(void)
{
	if (ChainTo != NULL) {
		ChainTo->ChainFrom = ChainFrom;
	}
	if (ChainFrom != NULL) {
		ChainFrom->Put_To(ChainTo);
	}

	ChainFrom = NULL;
	ChainTo = NULL;
}


/***********************************************************************************************
 * Pipe::Put_To -- Connect a pipe to flow data into from this pipe.                            *
 *                                                                                             *
 *    This routine will link two pipes together. The specified pipe will be fed data from      *
 *    this pipe.                                                                               *
 *                                                                                             *
 * INPUT:   pipe  -- Pointer to the pipe that data will flow to.                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void Pipe::Put_To(Pipe * pipe)
{
	if (ChainTo != pipe) {
		if (pipe != NULL && pipe->ChainFrom != NULL) {
			pipe->ChainFrom->Put_To(NULL);
			pipe->ChainFrom = NULL;
		}

		if (ChainTo != NULL) {
			ChainTo->ChainFrom = NULL;
			ChainTo->Flush();
		}

		ChainTo = pipe;
		if (ChainTo != NULL) {
			ChainTo->ChainFrom = this;
		}
	}
}


/***********************************************************************************************
 * Pipe::Put -- Feed some data through the pipe.                                               *
 *                                                                                             *
 *    Use this to force feed data through the pipe. It is guaranteed to accept data as fast    *
 *    as you can supply it.                                                                    *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the data to feed to this routine.                           *
 *                                                                                             *
 *          length   -- The number of bytes of data to submit.                                 *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes actually output at the other far distant final    *
 *          end of the pipe.                                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int Pipe::Put(void const * source, int length)
{
	if (ChainTo != NULL) {
		return(ChainTo->Put(source, length));
	}
	return(length);
}


/***********************************************************************************************
 * Pipe::Flush -- Flush all pending data out the pipe.                                         *
 *                                                                                             *
 *    Then the pipe needs to be flushed, this routine will be called. Since pipe segments      *
 *    might have internal staging buffer for the data, this routine is necessary to force      *
 *    all staging buffers to be clear. This routine is called when the pipe is being           *
 *    destroyed.                                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes output at the far distant final end of the pipe   *
 *          chain.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int Pipe::Flush(void)
{
	if (ChainTo != NULL) {
		return(ChainTo->Flush());
	}
	return(0);
}


