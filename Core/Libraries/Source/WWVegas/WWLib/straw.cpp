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
 *                     $Archive:: /G/wwlib/STRAW.cpp                                          $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/15/99 10:16a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include "STRAW.h"
#include	<stddef.h>


// Removes this segment from the straw chain. If there were any connected segments to either side,
// they will be joined so that the straw chain will still remain linked.
Straw::~Straw()
{
	if (ChainTo != nullptr) {
		ChainTo->ChainFrom = ChainFrom;
	}
	if (ChainFrom != nullptr) {
		ChainFrom->Get_From(ChainTo);
	}

	ChainFrom = nullptr;
	ChainTo = nullptr;
}


// The straw segment specified as the parameter will be linked to the chain such that when data is
// requested, it will be fetched from the specified link before being processed by this link.
void Straw::Get_From(Straw * straw)
{
	if (ChainTo != straw) {
		if (straw != nullptr && straw->ChainFrom != nullptr) {
			straw->ChainFrom->Get_From(nullptr);
			straw->ChainFrom = nullptr;
		}

		if (ChainTo != nullptr) {
			ChainTo->ChainFrom = nullptr;
		}

		ChainTo = straw;
		if (ChainTo != nullptr) {
			ChainTo->ChainFrom = this;
		}
	}
}


// It is presumed that this routine will be overridden to provide some useful functionality. At this
// level, the straw chain merely passes the request on to the next straw in the chain. Returns fewer
// bytes than requested when the straw data has been exhausted.
int Straw::Get(void * source, int slen)
{
	if (ChainTo != nullptr) {
		return(ChainTo->Get(source, slen));
	}
	return(0);
}


