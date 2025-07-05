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

// FILE: ObjectTypes.cpp //////////////////////////////////////////////////////////////////////////
// Author: John McDonald, Jr.
// September 2002
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "GameLogic/ObjectTypes.h"

#include "Common/GameState.h"
#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

//-------------------------------------------------------------------------------------------------
ObjectTypes::ObjectTypes()
{
	// Nada
}

//-------------------------------------------------------------------------------------------------
ObjectTypes::ObjectTypes(const AsciiString& listName) : m_listName(listName)
{
	// Nada
}

//-------------------------------------------------------------------------------------------------
void ObjectTypes::addObjectType(const AsciiString &objectType)
{
	if (isInSet(objectType)) {
		return;
	}

	m_objectTypes.push_back(objectType);
}

//-------------------------------------------------------------------------------------------------
void ObjectTypes::removeObjectType(const AsciiString &objectType)
{
	if (!isInSet(objectType)) {
		DEBUG_CRASH(("Attempted to remove '%s' from '%s', but it wasn't there.", objectType.str(), m_listName.str()));
		return;
	}

	AsciiStringVec::iterator it = std::find(m_objectTypes.begin(), m_objectTypes.end(), objectType);

	m_objectTypes.erase(it);
}

//-------------------------------------------------------------------------------------------------
const AsciiString& ObjectTypes::getListName() const
{
	return m_listName;
}

//-------------------------------------------------------------------------------------------------
void ObjectTypes::setListName(const AsciiString& listName)
{
	m_listName = listName;
}

//-------------------------------------------------------------------------------------------------
Bool ObjectTypes::isInSet(const AsciiString& objectType) const
{
	return (std::find(m_objectTypes.begin(), m_objectTypes.end(), objectType) != m_objectTypes.end());

}

//-------------------------------------------------------------------------------------------------
Bool ObjectTypes::isInSet(const ThingTemplate* objectType) const
{
	if (!objectType) {
		return FALSE;
	}

	return isInSet(objectType->getName());
}

//-------------------------------------------------------------------------------------------------
Int ObjectTypes::prepForPlayerCounting( std::vector<const ThingTemplate *>& templates, std::vector<Int>& counts)
{
	AsciiStringVecIt it;
	for (it = m_objectTypes.begin(); it != m_objectTypes.end(); ++it) {
		const ThingTemplate *templ = TheThingFactory->findTemplate(*it);
		if (templ) {
			templates.push_back(templ);
		}
	}

	Int retVal = templates.size();
	counts.resize(retVal);

	return retVal;
}

//-------------------------------------------------------------------------------------------------
Bool ObjectTypes::canBuildAny(Player *player)
{
	AsciiStringVecIt it;
	for (it = m_objectTypes.begin(); it != m_objectTypes.end(); ++it) {
		const ThingTemplate *templ = TheThingFactory->findTemplate(*it);
		if (templ && player->canBuild(templ)) {
			return TRUE;
		}
	}

	return FALSE;
}

// ------------------------------------------------------------------------------------------------
void ObjectTypes::crc(Xfer *xfer)
{

}

// ------------------------------------------------------------------------------------------------
/** Version Info:
	* 1: Initial version
	*/
// ------------------------------------------------------------------------------------------------
void ObjectTypes::xfer(Xfer *xfer)
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// list name
	xfer->xferAsciiString( &m_listName );

	// size of object types vector
	UnsignedShort objectTypesCount = m_objectTypes.size();
	xfer->xferUnsignedShort( &objectTypesCount );

	// object types data
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// iterate vector
		AsciiStringVecIt it;
		for( it = m_objectTypes.begin(); it != m_objectTypes.end(); ++it )
		{

			// write type name
			xfer->xferAsciiString( &(*it) );

		}  // end for, it

	}  // end if, save
	else
	{

		// sanity, the vector should be empty when loading
		if( m_objectTypes.empty() == FALSE )
		{

			DEBUG_CRASH(( "ObjectTypes::xfer - m_objectTypes vector should be emtpy but is not!" ));
			throw SC_INVALID_DATA;

		}  // end if

		// read all data
		AsciiString typeName;
		for( UnsignedShort i = 0; i < objectTypesCount; ++i )
		{

			// read name
			xfer->xferAsciiString( &typeName );

			// put on vector
			m_objectTypes.push_back( typeName );

		}  // end for, i

	}  // end else, load

}

// ------------------------------------------------------------------------------------------------
void ObjectTypes::loadPostProcess()
{

}
