/*
**	Command & Conquer Generals(tm)
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

// FILE: Xfer.h ///////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   The Xfer system is capable of setting up operations to work with blocks of data
//				 from other subsystems.  It can work things such as file reading, file writing,
//				 CRC computations etc
//
// TheSuperHackers @info helmutbuhler 04/09/2025
//         The baseclass Xfer has 3 implementations:
//          - XferLoad: Load gamestate
//          - XferSave: Save gamestate
//          - XferCRC: Calculate gamestate CRC
//            - XferDeepCRC: This derives from XferCRC and also writes the gamestate data relevant
//              to crc calculation to a file (only used in developer builds)
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __XFER_H_
#define __XFER_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/ModelState.h"
#include "Common/Science.h"
#include "Common/Upgrade.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Snapshot;
typedef Int Color;
enum ObjectID CPP_11(: Int);
enum DrawableID CPP_11(: Int);
enum KindOfType CPP_11(: Int);
enum ScienceType CPP_11(: Int);
class Matrix3D;

// ------------------------------------------------------------------------------------------------
typedef UnsignedByte XferVersion;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
enum XferMode CPP_11(: Int)
{
	XFER_INVALID = 0,

	XFER_SAVE,
	XFER_LOAD,
	XFER_CRC,

	NUM_XFER_TYPES  // please keep this last
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
enum XferStatus CPP_11(: Int)
{
	XFER_STATUS_INVALID = 0,
	
	XFER_OK,														///< all is green and good
	XFER_EOF,														///< end of file encountered
	XFER_FILE_NOT_FOUND,								///< requested file does not exist
	XFER_FILE_NOT_OPEN,									///< file was not open
	XFER_FILE_ALREADY_OPEN,							///< this xfer is already open
	XFER_READ_ERROR,										///< error reading from file
	XFER_WRITE_ERROR,										///< error writing to file
	XFER_MODE_UNKNOWN,									///< unknown xfer mode
	XFER_SKIP_ERROR,										///< error skipping file
	XFER_BEGIN_END_MISMATCH,						///< mismatched pair calls of begin/end block
	XFER_OUT_OF_MEMORY,									///< out of memory
	XFER_STRING_ERROR,									///< error with strings
	XFER_INVALID_VERSION,								///< invalid version encountered
	XFER_INVALID_PARAMETERS,						///< invalid parameters
	XFER_LIST_NOT_EMPTY,								///< trying to xfer into a list that should be empty, but isn't
	XFER_UNKNOWN_STRING,								///< unrecognized string value
	
	XFER_ERROR_UNKNOWN,									///< unknown error (isn't that useful!)

	NUM_XFER_STATUS  // please keep this last
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
enum XferOptions CPP_11(: UnsignedInt)
{
	XO_NONE										= 0x00000000,
	XO_NO_POST_PROCESSING			= 0x00000001,

	XO_ALL										= 0xFFFFFFFF  // keep this last please
};

///////////////////////////////////////////////////////////////////////////////////////////////////
typedef Int XferBlockSize;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class Xfer
{

public:

	Xfer( void );
	virtual ~Xfer( void );

	virtual XferMode getXferMode( void ) { return m_xferMode; }
	AsciiString getIdentifier( void ) { return m_identifier; }

	// xfer management
	virtual void setOptions( UnsignedInt options ) { BitSet( m_options, options ); }
	virtual void clearOptions( UnsignedInt options ) { BitClear( m_options, options ); }
	virtual UnsignedInt getOptions( void ) { return m_options; }
	virtual void open( AsciiString identifier ) = 0;		///< xfer open event
	virtual void close( void ) = 0;											///< xfer close event
	virtual Int beginBlock( void ) = 0;									///< xfer begin block event
	virtual void endBlock( void ) = 0;									///< xfer end block event
	virtual void skip( Int dataSize ) = 0;							///< xfer skip data

	virtual void xferSnapshot( Snapshot *snapshot ) = 0;		///< entry point for xfering a snapshot

	//
	// default transfer methods, these call the implementation method with the data
	// parameters.  You may use the default, or derive and create new ways to xfer each
	// of these types of data
	//
	virtual void xferVersion( XferVersion *versionData, XferVersion currentVersion );
	virtual void xferByte( Byte *byteData );
	virtual void xferUnsignedByte( UnsignedByte *unsignedByteData );
	virtual void xferBool( Bool *boolData );
	virtual void xferInt( Int *intData );
	virtual void xferInt64( Int64 *int64Data );
	virtual void xferUnsignedInt( UnsignedInt *unsignedIntData );
	virtual void xferShort( Short *shortData );
	virtual void xferUnsignedShort( UnsignedShort *unsignedShortData );
	virtual void xferReal( Real *realData );
	virtual void xferMarkerLabel( AsciiString asciiStringData ); // This is purely for readability purposes - it is explicitly discarded on load.
	virtual void xferAsciiString( AsciiString *asciiStringData );
	virtual void xferUnicodeString( UnicodeString *unicodeStringData );
	virtual void xferCoord3D( Coord3D *coord3D );
	virtual void xferICoord3D( ICoord3D *iCoord3D );
	virtual void xferRegion3D( Region3D *region3D );
	virtual void xferIRegion3D( IRegion3D *iRegion3D );
	virtual void xferCoord2D( Coord2D *coord2D );
	virtual void xferICoord2D( ICoord2D *iCoord2D );
	virtual void xferRegion2D( Region2D *region2D );
	virtual void xferIRegion2D( IRegion2D *iRegion2D );
	virtual void xferRealRange( RealRange *realRange );
	virtual void xferColor( Color *color );
	virtual void xferRGBColor( RGBColor *rgbColor );
	virtual void xferRGBAColorReal( RGBAColorReal *rgbaColorReal );
	virtual void xferRGBAColorInt( RGBAColorInt *rgbaColorInt );
	virtual void xferObjectID( ObjectID *objectID );
	virtual void xferDrawableID( DrawableID *drawableID );
	virtual void xferSTLObjectIDList( std::list< ObjectID > *objectIDListData );
	virtual void xferSTLIntList( std::list< Int > *intListData );
	virtual void xferScienceType( ScienceType *science );
	virtual void xferScienceVec( ScienceVec *scienceVec );
	virtual void xferKindOf( KindOfType *kindOfData );
	virtual void xferUpgradeMask( UpgradeMaskType *upgradeMaskData );
	virtual void xferUser( void *data, Int dataSize );
	virtual void xferMatrix3D( Matrix3D* mtx );
	virtual void xferMapName( AsciiString *mapNameData );

protected:

	// this is the actual xfer impelmentation that each derived class should implement
	virtual void xferImplementation( void *data, Int dataSize ) = 0;

	UnsignedInt m_options;					///< xfer options
	XferMode m_xferMode;						///< the current xfer mode
	AsciiString m_identifier;				///< the string identifier
			
};

#endif // __XFER_H_

