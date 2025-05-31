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

// FILE: XferLoad.h ///////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Xfer hard disk read implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __XFER_LOAD_H_
#define __XFER_LOAD_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "Common/Xfer.h"

// FOWARD REFERNCES ///////////////////////////////////////////////////////////////////////////////
class Snapshot;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class XferLoad : public Xfer
{

public:

	XferLoad( void );
	virtual ~XferLoad( void );

	virtual void open( AsciiString identifier );				///< open file for writing
	virtual void close( void );													///< close file
	virtual Int beginBlock( void );														///< read placeholder block size
	virtual void endBlock( void );											///< reading an end block is a no-op
	virtual void skip( Int dataSize );									///< skip forward dataSize bytes in file

	virtual void xferSnapshot( Snapshot *snapshot );		///< entry point for xfering a snapshot

	// xfer methods
	virtual void xferAsciiString( AsciiString *asciiStringData );  ///< xfer ascii string (need our own)
	virtual void xferUnicodeString( UnicodeString *unicodeStringData );	///< xfer unicode string (need our own);

protected:

	virtual void xferImplementation( void *data, Int dataSize );		///< the xfer implementation

	FILE * m_fileFP;																					///< pointer to file

};

#endif // __XFER_LOAD_H_

