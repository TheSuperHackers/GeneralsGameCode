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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Snapshot.h ///////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   The Snapshot object is the base class interface for data structures that will
//				 be considered during game saves, loads, and CRC checks.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/AsciiString.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Xfer;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class Snapshot
{

friend class GameState;
friend class XferLoad;
friend class XferSave;
friend class XferCRC;

public:

	Snapshot( void );
	~Snapshot( void );

protected:

	/// run the "light" crc check on this data structure
	virtual void crc( Xfer *xfer ) = 0;

	/** run save, load, or deep CRC check on this data structure, the type depends on the
	setup of the Xfer pointer */
	virtual void xfer( Xfer *xfer ) = 0;

	/** post process phase for loading save games.  All save systems have their xfer
	run using XferLoad mode, and then all systems each have their post process run */
	virtual void loadPostProcess( void ) = 0;

};
