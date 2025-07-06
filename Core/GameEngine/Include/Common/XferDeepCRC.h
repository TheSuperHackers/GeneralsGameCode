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

// FILE: XferDeepCRC.h ////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Xfer hard disk read implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __XFERDEEPCRC_H_
#define __XFERDEEPCRC_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/Xfer.h"
#include "Common/XferCRC.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Snapshot;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class XferDeepCRC : public XferCRC
{

public:

	XferDeepCRC( void );
	virtual ~XferDeepCRC( void );

	// Xfer methods
	virtual void open( AsciiString identifier );		///< start a CRC session with this xfer instance
	virtual void close( void );											///< stop CRC session

	// xfer methods
	virtual void xferMarkerLabel( AsciiString asciiStringData );  ///< xfer ascii string (need our own)
	virtual void xferAsciiString( AsciiString *asciiStringData );  ///< xfer ascii string (need our own)
	virtual void xferUnicodeString( UnicodeString *unicodeStringData );	///< xfer unicode string (need our own);

protected:

	virtual void xferImplementation( void *data, Int dataSize );

	FILE * m_fileFP;																			///< pointer to file
};

#endif // __XFERDEEPCRC_H_

