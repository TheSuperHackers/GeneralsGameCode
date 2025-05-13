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

// FILE: Xfer.cpp /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   The Xfer system is capable of setting up operations to work with blocks of data
//				 from other subsystems.  It can work things such as file reading, file writing,
//				 CRC computations etc
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Upgrade.h"
#include "Common/GameState.h"
#include "Common/Xfer.h"
#include "Common/BitFlagsIO.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Xfer::Xfer( void )
{

	m_options = XO_NONE;
	m_xferMode = XFER_INVALID;

}  // end Xfer

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Xfer::~Xfer( void )
{

}  // end ~Xfer

// ------------------------------------------------------------------------------------------------
/** Open */
// ------------------------------------------------------------------------------------------------
void Xfer::open( AsciiString identifier )
{

	// save identifier
	m_identifier = identifier;

}  // end open

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferByte( Byte *byteData )
{

	xferImplementation( byteData, sizeof( Byte ) ); 

}  // end xferByte

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferVersion( XferVersion *versionData, XferVersion currentVersion )
{

	xferImplementation( versionData, sizeof( XferVersion ) );

	// sanity, after the xfer, version data is never allowed to be higher than the current version
	if( *versionData > currentVersion )
	{

		DEBUG_CRASH(( "XferVersion - Unknown version '%d' should be no higher than '%d'\n",
									*versionData, currentVersion ));
		throw XFER_INVALID_VERSION;

	}  // end if

}  // end xferVersion

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferUnsignedByte( UnsignedByte *unsignedByteData )
{

	xferImplementation( unsignedByteData, sizeof( UnsignedByte ) ); 

}  // end xferUnsignedByte

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferBool( Bool *boolData )
{

	xferImplementation( boolData, sizeof( Bool ) ); 

}  // end xferBool

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferInt( Int *intData ) 
{

	xferImplementation( intData, sizeof( Int ) );
	
}  // end xferInt

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferInt64( Int64 *int64Data )
{

	xferImplementation( int64Data, sizeof( Int64 ) );

}  // end xferInt64

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferUnsignedInt( UnsignedInt *unsignedIntData ) 
{

	xferImplementation( unsignedIntData, sizeof( UnsignedInt ) );
	
}  // end xferUnsignedInt

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferShort( Short *shortData )
{

	xferImplementation( shortData, sizeof( Short ) ); 

}  // end xferShort

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferUnsignedShort( UnsignedShort *unsignedShortData )
{

	xferImplementation( unsignedShortData, sizeof( UnsignedShort ) ); 

}  // end xferUnsignedShort

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferReal( Real *realData )
{

	xferImplementation( realData, sizeof( Real ) ); 
	
}  // end xferReal

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferMapName( AsciiString *mapNameData )
{
	if (getXferMode() == XFER_SAVE)
	{
		AsciiString tmp = TheGameState->realMapPathToPortableMapPath(*mapNameData);
		xferAsciiString(&tmp);
	}
	else if (getXferMode() == XFER_LOAD)
	{
		xferAsciiString(mapNameData);
		*mapNameData = TheGameState->portableMapPathToRealMapPath(*mapNameData);
	}
}  // end xferAsciiString

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferAsciiString( AsciiString *asciiStringData )
{

	xferImplementation( (void *)asciiStringData->str(), sizeof( Byte ) * asciiStringData->getLength() );

}  // end xferAsciiString

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferMarkerLabel( AsciiString asciiStringData )
{
}  // end xferMarkerLabel

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferUnicodeString( UnicodeString *unicodeStringData )
{

	xferImplementation( (void *)unicodeStringData->str(), sizeof( WideChar ) * unicodeStringData->getLength() );

}  // end xferUnicodeString

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferCoord3D( Coord3D *coord3D )
{

	xferReal( &coord3D->x );
	xferReal( &coord3D->y );
	xferReal( &coord3D->z );

}  // end xferCoord3D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferICoord3D( ICoord3D *iCoord3D )
{

	xferInt( &iCoord3D->x );
	xferInt( &iCoord3D->y );
	xferInt( &iCoord3D->z );

}  // end xferICoor3D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferRegion3D( Region3D *region3D )
{

	xferCoord3D( &region3D->lo );
	xferCoord3D( &region3D->hi );

}  // end xferRegion3D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferIRegion3D( IRegion3D *iRegion3D )
{

	xferICoord3D( &iRegion3D->lo );
	xferICoord3D( &iRegion3D->hi );

}  // end xferIRegion3D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferCoord2D( Coord2D *coord2D )
{

	xferReal( &coord2D->x );
	xferReal( &coord2D->y );

}  // end xferCoord2D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferICoord2D( ICoord2D *iCoord2D )
{

	xferInt( &iCoord2D->x );
	xferInt( &iCoord2D->y );

}  // end xferICoord2D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferRegion2D( Region2D *region2D )
{

	xferCoord2D( &region2D->lo );
	xferCoord2D( &region2D->hi );

}  // end xferRegion2D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferIRegion2D( IRegion2D *iRegion2D )
{

	xferICoord2D( &iRegion2D->lo );
	xferICoord2D( &iRegion2D->hi );

}  // end xferIRegion2D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferRealRange( RealRange *realRange )
{

	xferReal( &realRange->lo );
	xferReal( &realRange->hi );

}  // end xferRealRange

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferColor( Color *color )
{

	xferImplementation( color, sizeof( Color ) );

}  // end xferColor

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferRGBColor( RGBColor *rgbColor )
{

	xferReal( &rgbColor->red );
	xferReal( &rgbColor->green );
	xferReal( &rgbColor->blue );

}  // end xferRGBColor

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferRGBAColorReal( RGBAColorReal *rgbaColorReal )
{

	xferReal( &rgbaColorReal->red );
	xferReal( &rgbaColorReal->green );
	xferReal( &rgbaColorReal->blue );
	xferReal( &rgbaColorReal->alpha );

}  // end xferRGBAColorReal

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferRGBAColorInt( RGBAColorInt *rgbaColorInt )
{

	xferUnsignedInt( &rgbaColorInt->red );
	xferUnsignedInt( &rgbaColorInt->green );
	xferUnsignedInt( &rgbaColorInt->blue );
	xferUnsignedInt( &rgbaColorInt->alpha );

}  // end xferRGBAColorInt

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferObjectID( ObjectID *objectID )
{

	xferImplementation( objectID, sizeof( ObjectID ) );

}  // end xferObjeftID

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferDrawableID( DrawableID *drawableID )
{

	xferImplementation( drawableID, sizeof( DrawableID ) );

}  // end xferDrawableID


// ------------------------------------------------------------------------------------------------
void Xfer::xferSTLObjectIDVector( std::vector<ObjectID> *objectIDVectorData )
{
	//
	// the fact that this is a list and a little higher level than a simple data type
	// is reason enough to have every one of these versioned
	//
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xferVersion( &version, currentVersion );

	// xfer the count of the vector
	UnsignedShort listCount = objectIDVectorData->size();
	xferUnsignedShort( &listCount );
	
	// xfer vector data
	ObjectID objectID;
	if( getXferMode() == XFER_SAVE || getXferMode() == XFER_CRC )
	{

		// save all ids
		std::vector< ObjectID >::const_iterator it;
		for( it = objectIDVectorData->begin(); it != objectIDVectorData->end(); ++it )
		{

			objectID = *it;
			xferObjectID( &objectID );

		}  // end for

	}  // end if, save
	else if( getXferMode() == XFER_LOAD )
	{

		// sanity, the list should be empty before we transfer more data into it
		if( objectIDVectorData->size() != 0 )
		{

			DEBUG_CRASH(( "Xfer::xferSTLObjectIDList - object vector should be empty before loading\n" ));
			throw XFER_LIST_NOT_EMPTY;

		}  // end if

		// read all ids
		for( UnsignedShort i = 0; i < listCount; ++i )
		{

			xferObjectID( &objectID );
			objectIDVectorData->push_back( objectID );

		}  // end for, i

	}  // end else if
	else
	{

		DEBUG_CRASH(( "xferSTLObjectIDList - Unknown xfer mode '%d'\n", getXferMode() ));
		throw XFER_MODE_UNKNOWN;

	}  // end else
}

// ------------------------------------------------------------------------------------------------
/** STL Object ID list (cause it's a common data structure we use a lot)
	* Version Info;
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void Xfer::xferSTLObjectIDList( std::list< ObjectID > *objectIDListData )
{

	//
	// the fact that this is a list and a little higher level than a simple data type
	// is reason enough to have every one of these versioned
	//
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xferVersion( &version, currentVersion );

	// xfer the count of the list
	UnsignedShort listCount = objectIDListData->size();
	xferUnsignedShort( &listCount );
	
	// xfer list data
	ObjectID objectID;
	if( getXferMode() == XFER_SAVE || getXferMode() == XFER_CRC )
	{

		// save all ids
		std::list< ObjectID >::const_iterator it;
		for( it = objectIDListData->begin(); it != objectIDListData->end(); ++it )
		{

			objectID = *it;
			xferObjectID( &objectID );

		}  // end for

	}  // end if, save
	else if( getXferMode() == XFER_LOAD )
	{

		// sanity, the list should be empty before we transfer more data into it
		if( objectIDListData->size() != 0 )
		{

			DEBUG_CRASH(( "Xfer::xferSTLObjectIDList - object list should be empty before loading\n" ));
			throw XFER_LIST_NOT_EMPTY;

		}  // end if

		// read all ids
		for( UnsignedShort i = 0; i < listCount; ++i )
		{

			xferObjectID( &objectID );
			objectIDListData->push_back( objectID );

		}  // end for, i

	}  // end else if
	else
	{

		DEBUG_CRASH(( "xferSTLObjectIDList - Unknown xfer mode '%d'\n", getXferMode() ));
		throw XFER_MODE_UNKNOWN;

	}  // end else

}  // end xferSTLObjectIDList

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferSTLIntList( std::list< Int > *intListData )
{

	// sanity
	if( intListData == NULL )
		return;

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xferVersion( &version, currentVersion );

	// xfer the count of the list
	UnsignedShort listCount = intListData->size();
	xferUnsignedShort( &listCount );
	
	// xfer list data
	Int intData;
	if( getXferMode() == XFER_SAVE || getXferMode() == XFER_CRC )
	{

		// save all ids
		std::list< Int >::const_iterator it;
		for( it = intListData->begin(); it != intListData->end(); ++it )
		{

			intData = *it;
			xferInt( &intData );

		}  // end for

	}  // end if, save
	else if( getXferMode() == XFER_LOAD )
	{

		// sanity, the list should be empty before we transfer more data into it
		if( intListData->size() != 0 )
		{

			DEBUG_CRASH(( "Xfer::xferSTLIntList - int list should be empty before loading\n" ));
			throw XFER_LIST_NOT_EMPTY;

		}  // end if

		// read all ids
		for( UnsignedShort i = 0; i < listCount; ++i )
		{

			xferInt( &intData );
			intListData->push_back( intData );

		}  // end for, i

	}  // end else if
	else
	{

		DEBUG_CRASH(( "xferSTLIntList - Unknown xfer mode '%d'\n", getXferMode() ));
		throw XFER_MODE_UNKNOWN;

	}  // end else

}  // end xferSTLIntList

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferScienceType( ScienceType *science )
{

	// sanity
	DEBUG_ASSERTCRASH( science != NULL, ("xferScienceType - Invalid parameters\n") );

	AsciiString scienceName;

	if( getXferMode() == XFER_SAVE )
	{
		// translate to string
		scienceName = TheScienceStore->getInternalNameForScience( *science );

		// write the string
		xferAsciiString( &scienceName );

	}  // end if, save
	else if( getXferMode() == XFER_LOAD )
	{
		xferAsciiString( &scienceName );

		// translate to science
		*science = TheScienceStore->getScienceFromInternalName( scienceName );
		if( *science == SCIENCE_INVALID )
		{

			DEBUG_CRASH(( "xferScienceType - Unknown science '%s'\n", scienceName.str() ));
			throw XFER_UNKNOWN_STRING;

		}  // end if
			
	}  // end else if, load
	else if( getXferMode() == XFER_CRC )
	{
			xferImplementation( science, sizeof( *science ) );

	}  // end else if, crc
	else
	{

		DEBUG_CRASH(( "xferScienceType - Unknown xfer mode '%d'\n", getXferMode() ));
		throw XFER_MODE_UNKNOWN;

	}  // end else

}  // end xferScienceType

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferScienceVec( ScienceVec *scienceVec )
{

	// sanity
	DEBUG_ASSERTCRASH( scienceVec != NULL, ("xferScienceVec - Invalid parameters\n") );

	// this deserves a version number
	const XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xferVersion( &version, currentVersion );

	// count of vector
	UnsignedShort count = scienceVec->size();
	xferUnsignedShort( &count );

	if( getXferMode() == XFER_SAVE )
	{
		for( ScienceVec::const_iterator it = scienceVec->begin(); it != scienceVec->end(); ++it )
		{
			ScienceType science = *it;
			xferScienceType(&science);
		}
	}
	else if( getXferMode() == XFER_LOAD )
	{
		// vector should be empty at this point
		if( scienceVec->empty() == FALSE )
		{
			// Not worth an assert, since things can give you Sciences on creation.  Just handle it and load.
			scienceVec->clear();

			// Homework for today.  Write 2000 words reconciling "Your code must never crash" with "Intentionally putting crashes in the code".  Fucktard.
//			DEBUG_CRASH(( "xferScienceVec - vector is not empty, but should be\n" ));
//			throw XFER_LIST_NOT_EMPTY;
		}

		for( UnsignedShort i = 0; i < count; ++i )
		{
			ScienceType science;
			xferScienceType(&science);
			scienceVec->push_back( science );				
		}
			
	}
	else if( getXferMode() == XFER_CRC )
	{
		for( ScienceVec::const_iterator it = scienceVec->begin(); it != scienceVec->end(); ++it )
		{
			ScienceType science = *it;
			xferImplementation( &science, sizeof( ScienceType ) );
		}  // end for, it
	}  // end else if, crc
	else
	{

		DEBUG_CRASH(( "xferScienceVec - Unknown xfer mode '%d'\n", getXferMode() ));
		throw XFER_MODE_UNKNOWN;

	}  // end else

}  // end xferScienceVec

// ------------------------------------------------------------------------------------------------
/** kind of type, for load/save it is xfered as a string so we can reorder the 
	* kindofs if we like
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void Xfer::xferKindOf( KindOfType *kindOfData )
{

	// this deserves a version number
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xferVersion( &version, currentVersion );

	// check which type of xfer we're doing
	if( getXferMode() == XFER_SAVE )
	{

		// save as an ascii string
		AsciiString kindOfName = KindOfMaskType::getNameFromSingleBit(*kindOfData);
		xferAsciiString( &kindOfName );

	}  // end if, save
	else if( getXferMode() == XFER_LOAD )
	{
		
		// read ascii string from file
		AsciiString kindOfName;
		xferAsciiString( &kindOfName );

		// turn kind of name into an enum value
		Int bit = KindOfMaskType::getSingleBitFromName(kindOfName.str());
		if (bit != -1)
			*kindOfData = (KindOfType)bit;
				
	}  // end else if, load
	else if( getXferMode() == XFER_CRC )
	{

		// just call the xfer implementation on the data values
		xferImplementation( kindOfData, sizeof( KindOfType ) );

	}  // end else if, crc
	else
	{

		DEBUG_CRASH(( "xferKindOf - Unknown xfer mode '%d'\n", getXferMode() ));
		throw XFER_MODE_UNKNOWN;

	}  // end else

}  // end xferKindOf

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferUpgradeMask( UpgradeMaskType *upgradeMaskData )
{

	// this deserves a version number
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xferVersion( &version, currentVersion );

	//Kris: The Upgrade system has been converted from Int64 to BitFlags. However because the 
	//names of upgrades are saved to preserve order reassignments (inserting a new upgrade in
	//the INI file will skew the bit values), we must continue saving the names of the upgrades
	//in order to recalculate the actual bit value of said upgrade.
	//---------------------------------------------------------------------------------------------
	//NOTE: The xfer code didn't have to change with the bitset upgrades, because either way, we're 
	//converting data <-> Ascii, so the minor syntax works with the before and after code!

	// check which type of xfer we're doing
	if( getXferMode() == XFER_SAVE )
	{
		AsciiString upgradeName;

		// count how many bits are set in the mask
		UnsignedShort count = 0;
		UpgradeTemplate *upgradeTemplate;
		for( upgradeTemplate = TheUpgradeCenter->firstUpgradeTemplate(); upgradeTemplate; upgradeTemplate = upgradeTemplate->friend_getNext() )
		{
			// if the mask of this upgrade is set, it counts
			if( upgradeMaskData->testForAll( upgradeTemplate->getUpgradeMask() ) )
			{
				count++;
			}
		}  // end for, upgradeTemplate

		// write the count
		xferUnsignedShort( &count );

		// write out the upgrades as strings
		for( upgradeTemplate = TheUpgradeCenter->firstUpgradeTemplate(); upgradeTemplate; upgradeTemplate = upgradeTemplate->friend_getNext() )
		{
			// if the mask of this upgrade is set, it counts
			if( upgradeMaskData->testForAll( upgradeTemplate->getUpgradeMask() ) )
			{
				upgradeName = upgradeTemplate->getUpgradeName();
				xferAsciiString( &upgradeName );
			}  // end if
		}  // end for, upgradeTemplate
	}  // end if, save
	else if( getXferMode() == XFER_LOAD )
	{
		AsciiString upgradeName;
		const UpgradeTemplate *upgradeTemplate;

		// how many strings are we going to read from the file
		UnsignedShort count;
		xferUnsignedShort( &count );

		// zero the mask data
		upgradeMaskData->clear();

		// read all the strings and set the mask vaules
		for( UnsignedShort i = 0; i < count; ++i )
		{

			// read the string
			xferAsciiString( &upgradeName );

			// find this upgrade template
			upgradeTemplate = TheUpgradeCenter->findUpgrade( upgradeName );
			if( upgradeTemplate == NULL )
			{

				DEBUG_CRASH(( "Xfer::xferUpgradeMask - Unknown upgrade '%s'\n", upgradeName.str() ));
				throw XFER_UNKNOWN_STRING;

			}  // end if

			// set the mask data
			upgradeMaskData->set( upgradeTemplate->getUpgradeMask() );

		}  // end for i

	}  // end else if, load
	else if( getXferMode() == XFER_CRC )
	{

		// just xfer implementation the data itself
		xferImplementation( upgradeMaskData, sizeof( UpgradeMaskType ) );

	}  // end else if, crc
	else
	{

		DEBUG_CRASH(( "xferUpgradeMask - Unknown xfer mode '%d'\n", getXferMode() ));
		throw XFER_MODE_UNKNOWN;

	}  // end else
	
}  // end xferUpgradeMask

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferUser( void *data, Int dataSize )
{

	xferImplementation( data, dataSize );

}  // end xferUser

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Xfer::xferMatrix3D( Matrix3D* mtx )
{
	// this deserves a version number
	const XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xferVersion( &version, currentVersion );

 	Vector4& tmp0 = (*mtx)[0];
 	Vector4& tmp1 = (*mtx)[1];
 	Vector4& tmp2 = (*mtx)[2];

	xferReal(&tmp0.X);
	xferReal(&tmp0.Y);
	xferReal(&tmp0.Z);
	xferReal(&tmp0.W);

	xferReal(&tmp1.X);
	xferReal(&tmp1.Y);
	xferReal(&tmp1.Z);
	xferReal(&tmp1.W);

	xferReal(&tmp2.X);
	xferReal(&tmp2.Y);
	xferReal(&tmp2.Z);
	xferReal(&tmp2.W);
}


