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
 *                     $Archive:: /Commando/Library/CRCPIPE.CPP                               $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   CRCPipe::Result -- Fetches the current CRC of the data.                                   *
 *   CRCPipe::Put -- Retrieves the data bytes specified and calculates CRC on it.              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"crcpipe.h"


/***********************************************************************************************
 * CRCPipe::Put -- Retrieves the data bytes specified and calculates CRC on it.                *
 *                                                                                             *
 *    This routine will fetch the number of bytes requested from the straw. The data is        *
 *    not modified by this straw segment, but it is examined by the CRC engine in order to     *
 *    keep an accurate CRC of the data that passes through this routine.                       *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the buffer that will hold the data requested.               *
 *                                                                                             *
 *          length   -- The number of bytes requested.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the actual number of bytes stored into the buffer. If this number is  *
 *          less than the number requested, then this indicates that the data stream has been  *
 *          exhausted.                                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int CRCPipe::Put(void const * source, int slen)
{
	CRC(source, slen);
	return(Pipe::Put(source, slen));
}


/***********************************************************************************************
 * CRCPipe::Result -- Fetches the current CRC of the data.                                     *
 *                                                                                             *
 *    This routine will return the CRC of the data that has passed through the pipe up to      *
 *    this time.                                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the CRC value.                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
long CRCPipe::Result(void) const
{
	return(CRC());
}

