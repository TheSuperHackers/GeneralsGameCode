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

// FILE: Radar.cpp ////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, January 2002
// Desc:   Radar logic implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameAudio.h"
#include "Common/GameState.h"
#include "Common/MiscAudio.h"
#include "Common/Radar.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingTemplate.h"
#include "Common/GlobalData.h"
#include "Common/Xfer.h"

#include "GameClient/Drawable.h"
#include "GameClient/Eva.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/InGameUI.h"
#include "GameClient/ControlBar.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/StealthUpdate.h"


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////////
Radar *TheRadar = NULL;  ///< the radar global singleton

// PRIVATE ////////////////////////////////////////////////////////////////////////////////////////
#define RADAR_QUEUE_TERRAIN_REFRESH_DELAY (LOGICFRAMES_PER_SECOND * 3.0f)

//-------------------------------------------------------------------------------------------------
/** Delete list resources used by the radar and return them to the memory pools */
//-------------------------------------------------------------------------------------------------
void Radar::deleteListResources( void )
{
	RadarObject *nextObject;

	// delete entries from the local object list
	while( m_localObjectList )
	{

		// get next object
		nextObject = m_localObjectList->friend_getNext();

		// remove radar data from object
		m_localObjectList->friend_getObject()->friend_setRadarData( NULL );

		// delete the head of the list
		deleteInstance(m_localObjectList);

		// set head of the list to the next object
		m_localObjectList = nextObject;

	}  // end while

	// delete entries from the regular object list
	while( m_objectList )
	{

		// get next object
		nextObject = m_objectList->friend_getNext();

		// remove radar data from object
		m_objectList->friend_getObject()->friend_setRadarData( NULL );

		// delete the head of the list
		deleteInstance(m_objectList);

		// set head of the list to the next object
		m_objectList = nextObject;

	}  // end while

	Object *obj;
	for( obj = TheGameLogic->getFirstObject(); obj; obj = obj->getNextObject() )
	{

		DEBUG_ASSERTCRASH( obj->friend_getRadarData() == NULL, ("oops") );

	}

}  // end deleteListResources

// PUBLIC METHODS /////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RadarObject::RadarObject( void )
{

	m_object = NULL;
	m_next = NULL;
	m_color = GameMakeColor( 255, 255, 255, 255 );

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RadarObject::~RadarObject( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool RadarObject::isTemporarilyHidden() const
{
	return isTemporarilyHidden(m_object);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool RadarObject::isTemporarilyHidden(const Object* obj)
{
	Drawable* draw = obj->getDrawable();
	if (draw == NULL || draw->getStealthLook() == STEALTHLOOK_INVISIBLE || draw->isDrawableEffectivelyHidden())
		return true;

	return false;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void RadarObject::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void RadarObject::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// object id
	ObjectID objectID = m_object ? m_object->getID() : INVALID_ID;
	xfer->xferObjectID( &objectID );
	if( xfer->getXferMode() == XFER_LOAD )
	{

		// find the object and save
		m_object = TheGameLogic->findObjectByID( objectID );
		if( m_object == NULL )
		{

			DEBUG_CRASH(( "RadarObject::xfer - Unable to find object for radar data" ));
			throw SC_INVALID_DATA;

		}  // end if

		// tell the object we now have some radar data
		m_object->friend_setRadarData( this );

	}  // end if

	// color
	xfer->xferColor( &m_color );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void RadarObject::loadPostProcess( void )
{

}  // end loadPostProcess

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Radar::Radar( void )
{

	m_radarWindow = NULL;
	m_objectList = NULL;
	m_localObjectList = NULL;
	m_radarHidden = false;
	m_radarForceOn = false;
	m_terrainAverageZ = 0.0f;
	m_waterAverageZ = 0.0f;
	m_xSample = 0.0f;
	m_ySample = 0.0f;
	m_mapExtent.lo.x = 0.0f;
	m_mapExtent.lo.y = 0.0f;
	m_mapExtent.lo.z = 0.0f;
	m_mapExtent.hi.x = 0.0f;
	m_mapExtent.hi.y = 0.0f;
	m_mapExtent.hi.z = 0.0f;
	m_queueTerrainRefreshFrame = 0;

	// clear the radar events
	clearAllEvents();

}  // end Radar

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Radar::~Radar( void )
{

	// delete list resources
	deleteListResources();

}  // end ~Radar

//-------------------------------------------------------------------------------------------------
/** Clear all radar events */
//-------------------------------------------------------------------------------------------------
void Radar::clearAllEvents( void )
{

	// set next free index to the first one
	m_nextFreeRadarEvent = 0;
	m_lastRadarEvent = -1;

	// zero out all data
	for( Int i = 0; i < MAX_RADAR_EVENTS; ++i )
	{

		m_event[ i ].type = RADAR_EVENT_INVALID;
		m_event[ i ].active = FALSE;
		m_event[ i ].createFrame = 0;
		m_event[ i ].dieFrame = 0;
		m_event[ i ].fadeFrame = 0;
		m_event[ i ].color1.red = 0;
		m_event[ i ].color1.green = 0;
		m_event[ i ].color1.blue = 0;
		m_event[ i ].color2.red = 0;
		m_event[ i ].color2.green = 0;
		m_event[ i ].color2.blue = 0;
		m_event[ i ].worldLoc.x = 0.0f;
		m_event[ i ].worldLoc.y = 0.0f;
		m_event[ i ].worldLoc.z = 0.0f;
		m_event[ i ].radarLoc.x = 0;
		m_event[ i ].radarLoc.y = 0;
		m_event[ i ].soundPlayed = FALSE;

	}  // end for i

}  // end clearAllEvents

//-------------------------------------------------------------------------------------------------
/** Reset radar data */
//-------------------------------------------------------------------------------------------------
void Radar::reset( void )
{

	// delete list resources
	deleteListResources();

	// clear all events
	clearAllEvents();

	// stop forcing the radar on
	m_radarForceOn = false;

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Radar per frame update */
//-------------------------------------------------------------------------------------------------
void Radar::update( void )
{
	Int i;
	UnsignedInt thisFrame = TheGameLogic->getFrame();

	//
	// traverse the radar event list, if an event has a creationFrame it means that it
	// exists ... check to see if it's time for that event to die and if so, just clear
	// out the active status
	//
	for( i = 0; i < MAX_RADAR_EVENTS; i++ )
	{

		if( m_event[ i ].active == TRUE && m_event[ i ].createFrame && 
				thisFrame > m_event[ i ].dieFrame )
			m_event[ i ].active = FALSE;
				
	}  // end for i

	// see if we should refresh the terrain
	if( m_queueTerrainRefreshFrame != 0 && 
			TheGameLogic->getFrame() - m_queueTerrainRefreshFrame > RADAR_QUEUE_TERRAIN_REFRESH_DELAY )
	{

		// refresh the terrain
		refreshTerrain( TheTerrainLogic );

	}  // end if

}  // end update

//-------------------------------------------------------------------------------------------------
/** Reset the radar for the new map data being given to it */
//-------------------------------------------------------------------------------------------------
void Radar::newMap( TerrainLogic *terrain )
{

	// keep a pointer for our radar window
	Int id = NAMEKEY( "ControlBar.wnd:LeftHUD" );
	m_radarWindow = TheWindowManager->winGetWindowFromId( NULL, id );
	DEBUG_ASSERTCRASH( m_radarWindow, ("Radar::newMap - Unable to find radar game window") );

	// reset all the data in the radar
	reset();

	// get the extents of the new map
	terrain->getExtent( &m_mapExtent );

	// we will sample at these intervals across the map
	m_xSample = m_mapExtent.width() / RADAR_CELL_WIDTH;
	m_ySample = m_mapExtent.height() / RADAR_CELL_HEIGHT;

	// find the "middle" height for the terrain (most used value) and water table
	Int x, y, z;
	Int terrainSamples = 0, waterSamples = 0;

	m_terrainAverageZ = 0.0f;
	m_waterAverageZ = 0.0f;
	ICoord2D radarPoint;
	Coord3D worldPoint;
	for( y = 0; y < RADAR_CELL_HEIGHT; y++ )
		for( x = 0; x < RADAR_CELL_WIDTH; x++ )
		{

			radarPoint.x = x;
			radarPoint.y = y;
			radarToWorld( &radarPoint, &worldPoint );
			z = terrain->getGroundHeight( worldPoint.x, worldPoint.y );
			Real waterZ;
			if( terrain->isUnderwater( worldPoint.x, worldPoint.y, &waterZ ) )
			{
				m_waterAverageZ += z;
				waterSamples++;
			}
			else
			{
				m_terrainAverageZ += z;
				terrainSamples++;
			}

		}  // end for x

	// avoid divide by zeros
	if( terrainSamples == 0 )
		terrainSamples = 1;
	if( waterSamples == 0 )
		waterSamples = 1;

	// compute averages
	m_terrainAverageZ = m_terrainAverageZ / INT_TO_REAL( terrainSamples );
	m_waterAverageZ = m_waterAverageZ / INT_TO_REAL( waterSamples );

}  // end newMap

//-------------------------------------------------------------------------------------------------
/** Add an object to the radar list.  The object will be sorted in the list to be grouped
	* using it's radar priority */
//-------------------------------------------------------------------------------------------------
RadarObjectType Radar::addObject( Object *obj )
{

	// get the radar priority for this object
	RadarPriorityType newPriority = obj->getRadarPriority();
	if( isPriorityVisible( newPriority ) == FALSE )
		return RadarObjectType_None;

	// if this object is on the radar, remove it in favor of the new add
	RadarObject **list;
	RadarObject *newObj;

	// sanity
	DEBUG_ASSERTCRASH( obj->friend_getRadarData() == NULL,
										 ("Radar: addObject - non NULL radar data for '%s'",
										 obj->getTemplate()->getName().str()) );

	// allocate a new object
	newObj = newInstance(RadarObject);

	// set the object data
	newObj->friend_setObject( obj );

	// set color for this object on the radar
	const Player *player = obj->getControllingPlayer();
	Bool useIndicatorColor = true;

	if( obj->isKindOf( KINDOF_DISGUISER ) )
	{
		//Because we have support for disguised units pretending to be units from another
		//team, we need to intercept it here and make sure it's rendered appropriately
		//based on which client is rendering it.
		StealthUpdate *update = obj->getStealth();
		if( update )
		{
			if( update->isDisguised() )
			{
				Player *clientPlayer = ThePlayerList->getLocalPlayer();
				Player *disguisedPlayer = ThePlayerList->getNthPlayer( update->getDisguisedPlayerIndex() );
				if( player->getRelationship( clientPlayer->getDefaultTeam() ) != ALLIES && clientPlayer->isPlayerActive() )
				{
					//Neutrals and enemies will see this disguised unit as the team it's disguised as.
					player = disguisedPlayer;
					if( player )
						useIndicatorColor = false;
				}
				//Otherwise, the color will show up as the team it really belongs to (already set above).
			}
		}
	}
	
	if( obj->getContain() )
	{
		// To handle Stealth garrison, ask containers what color they are drawing with to the local player.
		// Local is okay because radar display is not synced.
		player = obj->getContain()->getApparentControllingPlayer( ThePlayerList->getLocalPlayer() );
		if( player )
			useIndicatorColor = false;
	}

	if( useIndicatorColor || (player == NULL) )
	{
		newObj->setColor( obj->getIndicatorColor() );
	}
	else
	{	
		newObj->setColor( player->getPlayerColor() );
	}

	// set a chunk of radar data in the object
	obj->friend_setRadarData( newObj );

	RadarObjectType objectType;
	//
	// we will put this on either the local object list for objects that belong to the
	// local player, or on the regular object list for all other objects
	//
	if( obj->isLocallyControlled() )
	{
		list = &m_localObjectList;
		objectType = RadarObjectType_Local;
	}
	else
	{
		list = &m_objectList;
		objectType = RadarObjectType_Regular;
	}

	// link object to master list at the head of it's priority section
	if( *list == NULL )
		*list = newObj;  // trivial case, an empty list
	else
	{
		RadarPriorityType prevPriority, currPriority;
		RadarObject *currObject, *prevObject, *nextObject;

		prevObject = NULL;
		prevPriority = RADAR_PRIORITY_INVALID;
		for( currObject = *list; currObject; currObject = nextObject )
		{

			// get the next object
			nextObject = currObject->friend_getNext();

			// get the priority of this entry in the list (currPriority)
			currPriority = currObject->friend_getObject()->getRadarPriority();

			//
			// if there is no previous object, or the previous priority is less than the
			// our new priority, and the current object in the list has a priority 
			// higher than our equal to our own we need to be inserted here
			//
			if( (prevObject == NULL || prevPriority < newPriority ) &&
					(currPriority >= newPriority) )
			{

				// insert into the list just ahead of currObject
				if( prevObject )
				{

					// the new entry next points to what the previous one used to point to
					newObj->friend_setNext( prevObject->friend_getNext() );

					// the previous one next now points to the new entry
					prevObject->friend_setNext( newObj );

				}  // end if
				else
				{

					// the new object next points to the current object
					newObj->friend_setNext( currObject );

					// new list head is now newObj
					*list = newObj;

				}  // end else

				break;  // exit for, stop the insert

			}  // end if
			else if( nextObject == NULL )
			{

				// at the end of the list, put object here
				currObject->friend_setNext( newObj );

			}  // end else if

			// our current object is now the previous object
			prevObject = currObject;
			prevPriority = currPriority;

		}  // end if

	}  // end else

	return objectType;
}  // end addObject

//-------------------------------------------------------------------------------------------------
/** Try to delete an object from a specific list */
//-------------------------------------------------------------------------------------------------
Bool Radar::deleteFromList( Object *obj, RadarObject **list )
{
	RadarObject *radarObject, *prevObject = NULL;
					
	// find the object in list
	for( radarObject = *list; radarObject; radarObject = radarObject->friend_getNext() )
	{
		
		if( radarObject->friend_getObject() == obj )
		{

			// unlink the object from list
			if( prevObject == NULL )
				*list = radarObject->friend_getNext();  // removing head of list
			else
				prevObject->friend_setNext( radarObject->friend_getNext() );

			// set the object radar data to NULL
			obj->friend_setRadarData( NULL );

			// delete the object instance
			deleteInstance(radarObject);

			// all done, object found and deleted
			return TRUE;

		}  // end if

		// save this object as previous one encountered in the list
		prevObject = radarObject;

	}  // end for, radarObject

	// object was not found in this list
	return FALSE;

}  // end deleteFromList

//-------------------------------------------------------------------------------------------------
/** Remove an object from the radar, the object may reside in any list */
//-------------------------------------------------------------------------------------------------
RadarObjectType Radar::removeObject( Object *obj )
{

	// sanity
	if( obj->friend_getRadarData() == NULL )
		return RadarObjectType_None;

	if( deleteFromList( obj, &m_localObjectList ) == TRUE )
		return RadarObjectType_Local;
	else if( deleteFromList( obj, &m_objectList ) == TRUE )
		return RadarObjectType_Regular;
	else
	{

		// sanity
		DEBUG_ASSERTCRASH( 0, ("Radar: Tried to remove object '%s' which was not found",
											 obj->getTemplate()->getName().str()) );
		return RadarObjectType_None;
	}  // end else

}  // end removeObject

//-------------------------------------------------------------------------------------------------
/** Translate a 2D spot on the radar (from (0,0) to (RADAR_CELL_WIDTH,RADAR_CELL_HEIGHT)
	* to a 3D spot in the world. Does not determine Z value!
	* Return TRUE if the radar points translates to a valid world position
	* Return FALSE if the radar point is not a valid world position */
//-------------------------------------------------------------------------------------------------		
Bool Radar::radarToWorld2D( const ICoord2D *radar, Coord3D *world )
{
	Int x, y;

	// sanity
	if( radar == NULL || world == NULL )
		return FALSE;

	// get the coords
	x = radar->x;
	y = radar->y;

	// more sanity
	if( x < 0 )
		x = 0;
	if( x >= RADAR_CELL_WIDTH )
		x = RADAR_CELL_WIDTH - 1;
	if( y < 0 )
		y = 0;
	if( y >= RADAR_CELL_HEIGHT )
		y = RADAR_CELL_HEIGHT - 1;

	// translate to world
	world->x = x * m_xSample;
	world->y = y * m_ySample;
  return TRUE;
}

//-------------------------------------------------------------------------------------------------
/** Translate a 2D spot on the radar (from (0,0) to (RADAR_CELL_WIDTH,RADAR_CELL_HEIGHT)
	* to a 3D spot in the world on the terrain
	* Return TRUE if the radar points translates to a valid world position
	* Return FALSE if the radar point is not a valid world position */
//-------------------------------------------------------------------------------------------------		
Bool Radar::radarToWorld( const ICoord2D *radar, Coord3D *world )
{
  if (!radarToWorld2D(radar,world))
    return FALSE;

	// find the terrain height here
	world->z = TheTerrainLogic->getGroundHeight( world->x, world->y );
	
	return TRUE;  // valid translation

}  // end radarToWorld

//-------------------------------------------------------------------------------------------------
/** Translate a point in the world to the 2D radar (x,y)
	* Return TRUE if the world point successfully translates to a radar point
	* Return FALSE if world point is a bogus off the map position */
//-------------------------------------------------------------------------------------------------
Bool Radar::worldToRadar( const Coord3D *world, ICoord2D *radar )
{

	// sanity
	if( world == NULL || radar == NULL )
		return FALSE;

	// sanity check the world position
//	if( world->x < m_mapExtent.lo.x || world->x > m_mapExtent.hi.x ||
//			world->y < m_mapExtent.lo.y || world->y > m_mapExtent.hi.y )
//		return FALSE;
	// This is actually an insanity check.  Nobody uses the return value, so this just leaves garbage in the 
	// return pointer.  The reason the question gets asked is there are 60 partition cells to 128 radar cells
	// (for example), and the radar wants to draw a horizontal line.  This line ends up three pixels long
	// at the right side, so the radar gives up and doesn't draw the middle one.
	// We bind to on radar anyway, and we only ask if we are on radar, so don't intentionally add edge weirdness.

	// convert
	radar->x = world->x / m_xSample;
	radar->y = world->y / m_ySample;

	// keep it in bounds
	if( radar->x < 0 )
		radar->x = 0;
	if( radar->x >= RADAR_CELL_WIDTH )
		radar->x = RADAR_CELL_WIDTH - 1;
	if( radar->y < 0 )
		radar->y = 0;
	if( radar->y >= RADAR_CELL_HEIGHT )
		radar->y = RADAR_CELL_HEIGHT - 1;

	return TRUE;  // valid translation

}  // end worldToRadar

// ------------------------------------------------------------------------------------------------
/** Translate an actual pixel location (relative pixel with (0,0) being the top left of
	* the radar area) to the "logical" radar coords that would cover the entire area of display
	* on the screen.  This is needed because some maps are "long" or "tall" and need a translation
	* to any radar image that has been scaled to preserve the map aspect ratio */
// ------------------------------------------------------------------------------------------------
Bool Radar::localPixelToRadar( const ICoord2D *pixel, ICoord2D *radar )
{

	// sanity
	if( pixel == NULL || radar == NULL )
		return FALSE;

	// get window size of the radar
	ICoord2D size;
	m_radarWindow->winGetSize( &size.x, &size.y );

	//
	// act like we're going to draw and find the aspect ratio adjusted points of the 
	// terrain radar positions
	//
	ICoord2D start = { 0, 0 };
	ICoord2D ul, lr;
	findDrawPositions( start.x, start.y, size.x, size.y, &ul, &lr );

	// get the scaled width and height
	Int scaledWidth = lr.x - ul.x;
	Int scaledHeight = lr.y - ul.y;

	// if the pixel is outsize of the adjusted radar area there are no logical coords
	if( pixel->x < ul.x || pixel->x > lr.x ||
			pixel->y < ul.y || pixel->y > lr.y )
		return FALSE;

	if( scaledWidth >= scaledHeight )
	{

		// just normal conversion from full stretched to radar cells
		radar->x = (pixel->x - ul.x)* RADAR_CELL_WIDTH / scaledWidth;

		// conversion for scaled Y direction in map
		radar->y = REAL_TO_INT( ((pixel->y - ul.y) / INT_TO_REAL( scaledHeight )) * size.y );
		
		//
		// radar->y now refers to a point that was "as if" the map was square, translate to radar
		// note that y is inverted to have the radar align with the world (+x = right, -y = down)
		//
		radar->y = (size.y - radar->y) * RADAR_CELL_HEIGHT / size.y;


	}  // end if
	else
	{

		// conversion for scaled Y direction in map
		radar->x = REAL_TO_INT( ((pixel->x - ul.x) / INT_TO_REAL( scaledWidth )) * size.x );
		
		// radar->x now refers to a point that was "as if" the map was square, translate to radar
		radar->x = radar->x * RADAR_CELL_WIDTH / size.x;

		//
		// just normal conversion from full stretched to radar cells, note that y is inverted
		// to have the radar align with the world (+x = right, -y = down)
		//
		radar->y = (size.y - pixel->y) * RADAR_CELL_HEIGHT / size.y;

	}  // end else

	return TRUE;

}  // end localPixelToRadar

// ------------------------------------------------------------------------------------------------
/** Translate a screen mouse position to world coords if the screen position is within
	* the radar window and that spot in the radar corresponds to a point in the world */
// ------------------------------------------------------------------------------------------------
Bool Radar::screenPixelToWorld( const ICoord2D *pixel, Coord3D *world )
{

	// sanity
	if( pixel == NULL || world == NULL )
		return FALSE;

	// if we have no radar window can't do the conversion
	if( m_radarWindow == NULL )
		return FALSE;

	// translate pixel coords to local pixel coords relative to the radar window
	ICoord2D localPixel;
	ICoord2D screenPos;
	m_radarWindow->winGetScreenPosition( &screenPos.x, &screenPos.y );
	localPixel.x = pixel->x - screenPos.x;
	localPixel.y = pixel->y - screenPos.y;

	// translate local pixel to radar
	ICoord2D radar;
	if( localPixelToRadar( &localPixel, &radar ) == FALSE )
		return FALSE;

	// translate radar to world
	return radarToWorld( &radar, world );

}  // end screenPixelToWorld

// ------------------------------------------------------------------------------------------------
/** Given the pixel coordinates, see if there is an object that is exactly in this
	* spot represented on the radar */
// ------------------------------------------------------------------------------------------------
Object *Radar::objectUnderRadarPixel( const ICoord2D *pixel )
{

	// sanity
	if( pixel == NULL )
		return NULL;

	// convert pixel location to radar logical radar location
	ICoord2D radar;
	if( localPixelToRadar( pixel, &radar ) == FALSE )
		return NULL;

	// object we will return
	Object *obj = NULL;

	//
	// scan the objects on the radar list and return any object that maps its world location
	// to the radar location
	//
	
	// search the local object list
	obj = searchListForRadarLocationMatch( m_localObjectList, &radar );

	// search all other objects if not found
	if( obj == NULL )
		obj = searchListForRadarLocationMatch( m_objectList, &radar );

	// return the object found (if any)
	return obj;

}  // end objectUnderRadarPixel

// ------------------------------------------------------------------------------------------------
/** Search the object list for an object that maps to the given logical radar coords */
// ------------------------------------------------------------------------------------------------
Object *Radar::searchListForRadarLocationMatch( RadarObject *listHead, ICoord2D *radarMatch )
{

	// sanity
	if( listHead == NULL || radarMatch == NULL )
		return NULL;

	// scan the list
	RadarObject *radarObject;
	ICoord2D radar;
	for( radarObject = listHead; radarObject; radarObject = radarObject->friend_getNext() )
	{

		// get object
		Object *obj = radarObject->friend_getObject();
		
		// sanity
		if( obj == NULL )
		{

			DEBUG_CRASH(( "Radar::searchListForRadarLocationMatch - NULL object encountered in list" ));
			continue;

		}  // end if

		// convert object position to logical radar
		worldToRadar( obj->getPosition(), &radar );

		// see if this matches our match radar location
		if( radar.x >= radarMatch->x - 1 &&
				radar.x <= radarMatch->x + 1 && 
				radar.y >= radarMatch->y - 1 &&
				radar.y <= radarMatch->y + 1 )
			return obj;

	}  // end for, radarObject

	// no match found
	return NULL;

}  // end searchListForRadarLocationMatch

// ------------------------------------------------------------------------------------------------
/** Given the RELATIVE SCREEN start X and Y, the width and height of the area to draw the whole
	* radar in, compute what the upper left (ul) and lower right (lr) local coordinates are
	* that represent the actual terrain image part of the radar that will preserve the
	* aspect ratio of the map */
// ------------------------------------------------------------------------------------------------
void Radar::findDrawPositions( Int startX, Int startY, Int width, Int height, 
															 ICoord2D *ul, ICoord2D *lr )
{

	Real ratioWidth;
	Real ratioHeight;
	Coord2D radar;
	ratioWidth = m_mapExtent.width()/(width * 1.0f);
	ratioHeight = m_mapExtent.height()/(height* 1.0f);
	
	if( ratioWidth >= ratioHeight)
	{
		radar.x = m_mapExtent.width() / ratioWidth;
		radar.y = m_mapExtent.height()/ ratioWidth;
		ul->x = 0;
		ul->y = (height - radar.y) / 2.0f;
		lr->x = radar.x;
		lr->y = height - ul->y;
	}
	else
	{
		radar.x = m_mapExtent.width() / ratioHeight;
		radar.y = m_mapExtent.height()/ ratioHeight;
		ul->x = (width - radar.x ) / 2.0f;
		ul->y = 0;
		lr->x = width - ul->x;
		lr->y = radar.y;
	}
/*

	if( m_mapExtent.width() > m_mapExtent.height() )
	{

		//
		// +---------------+
		// |               |
		// |               |
		// +---------------+
		// |   map area    |
		// +---------------+
		// |               |
		// |               |
		// +---------------+
		//
		ul->x = 0;
		ul->y = (height - (m_mapExtent.height() / m_mapExtent.width() * height)) / 2.0f;
		lr->x = width;
		lr->y = height - ul->y;

	}  // end if
	else if(  m_mapExtent.height() > m_mapExtent.width() )
	{

		// +-----+---+-----+
		// |     | m |     |
		// |     | a |     |
		// |     | p |     |
		// |     |   |     |
		// |     | a |     |
		// |     | r |     |
		// |     | e |     |
		// |     | a |     |
		// +-----+---+-----+
		//

		ul->x = (width - (m_mapExtent.width() / m_mapExtent.height() * width)) / 2.0f;
		ul->y = 0;
		lr->x = width - ul->x;
		lr->y = height;

	}  // end else
	else
	{

		ul->x = 0;
		ul->y = 0;
		lr->x = width;
		lr->y = height;

	}  // end else
*/

	// make them pixel positions
	ul->x += startX;
	ul->y += startY;
	lr->x += startX;
	lr->y += startY;

}  // end findDrawPositions

//-------------------------------------------------------------------------------------------------
/** Radar color lookup table */
//-------------------------------------------------------------------------------------------------
struct RadarColorLookup
{
	RadarEventType event;
	RGBAColorInt color1;
	RGBAColorInt color2;
};
static RadarColorLookup radarColorLookupTable[] = 
{
	/*      Radar Event													Color 1									 Color 2       */
	{ RADAR_EVENT_CONSTRUCTION,					{ 128, 128, 255, 255 },  {  128, 255, 255, 255 } },
	{ RADAR_EVENT_UPGRADE,							{ 128,   0,  64, 255 },  {  255, 185, 220, 255 } },
	{ RADAR_EVENT_UNDER_ATTACK,					{ 255,   0,   0, 255 },  {  255, 128, 128, 255 } },
	{ RADAR_EVENT_INFORMATION,					{ 255, 255,   0, 255 },  {  255, 255, 128, 255 } },
	{ RADAR_EVENT_BEACON_PULSE,					{ 255, 255,   0, 255 },  {  255, 255, 128, 255 } },
	{ RADAR_EVENT_INFILTRATION,					{   0, 255, 255, 255 },  {  128, 255, 255, 255 } },
	{ RADAR_EVENT_BATTLE_PLAN,					{ 255, 255, 255, 255 },	 {  255, 255, 255, 255 } },
	{ RADAR_EVENT_STEALTH_DISCOVERED,		{   0, 255,   0, 255 },	 {    0, 128,   0, 255 } },
	{ RADAR_EVENT_STEALTH_NEUTRALIZED,	{   0, 255,   0, 255 },	 {    0, 128,   0, 255 } },
	{ RADAR_EVENT_FAKE,									{   0,	 0,   0,	 0 },	 {    0,	 0,   0,   0 } },
	{ RADAR_EVENT_INVALID,							{   0,   0,   0,   0 },  {    0,   0,   0,   0 } }
};

//-------------------------------------------------------------------------------------------------
/** Create a new radar event */
//-------------------------------------------------------------------------------------------------
void Radar::createEvent( const Coord3D *world, RadarEventType type, Real secondsToLive )
{
		
	// sanity
	if( world == NULL )
		return;

	// lookup the colors we are to used based on the event 
	RGBAColorInt color[ 2 ];
	Int i = 0;
	for( ; radarColorLookupTable[ i ].event != RADAR_EVENT_INVALID; ++i )
	{

		if( radarColorLookupTable[ i ].event == type )
		{

			color[ 0 ] = radarColorLookupTable[ i ].color1;
			color[ 1 ] = radarColorLookupTable[ i ].color2;
			break;

		}  // end if

	}  // end while

	// check for no match found in color table
	if( radarColorLookupTable[ i ].event == RADAR_EVENT_INVALID )
	{
		static RGBAColorInt color1 = { 255, 255, 255, 255 };
		static RGBAColorInt color2 = { 255, 255, 255, 255 };

		DEBUG_CRASH(( "Radar::createEvent - Event not found in color table, using default colors" ));
		color[ 0 ] = color1;
		color[ 1 ] = color2;

	}  // end if

	// call the internal method to create the event with these colors
	internalCreateEvent( world, type, secondsToLive, &color[ 0 ], &color[ 1 ] );

}  // end createEvent

// ------------------------------------------------------------------------------------------------
/** Create radar event using a specific colors from the player */
// ------------------------------------------------------------------------------------------------
void Radar::createPlayerEvent( Player *player, const Coord3D *world, 
															 RadarEventType type, Real secondsToLive )
{

	// sanity
	if( player == NULL || world == NULL )
		return;

	// figure out the two colors we should use
	Color c;
	UnsignedByte r, g, b, a;
	RGBAColorInt color[ 2 ];

	// color 1
	c = player->getPlayerColor();
	GameGetColorComponents( c, &r, &g, &b, &a );
	color[ 0 ].red = r;
	color[ 0 ].green = g;
	color[ 0 ].blue = b;
	color[ 0 ].alpha = a;

	// the second will be a darker color 1
	Real darkScale = 0.75f;
	color[ 1 ] = color[ 0 ];
	color[ 1 ].red -= REAL_TO_INT( color[ 0 ].red * darkScale );
	if( color[ 1 ].red < 0 )
		color[ 1 ].red = 0;
	color[ 1 ].green -= REAL_TO_INT( color[ 0 ].green * darkScale );
	if( color[ 1 ].green < 0 )
		color[ 1 ].green = 0;
	color[ 1 ].blue -= REAL_TO_INT( color[ 0 ].blue * darkScale );
	if( color[ 1 ].blue < 0 )
		color[ 1 ].blue = 0;

	// create the events using these colors
	internalCreateEvent( world, type, secondsToLive, &color[ 0 ], &color[ 1 ] );

}  // end createPlayerEvent

//-------------------------------------------------------------------------------------------------
/** Create a new radar event */
//-------------------------------------------------------------------------------------------------
void Radar::internalCreateEvent( const Coord3D *world, RadarEventType type, Real secondsToLive,
																 const RGBAColorInt *color1, const RGBAColorInt *color2 )
{
	static Real secondsBeforeDieToFade = 0.5f;  ///< this many seconds before we hit the die frame we start to fade away
		
	// sanity
	if( world == NULL || color1 == NULL || color2 == NULL )
		return;

	// translate the world coord to radar coords
	ICoord2D radar;
	worldToRadar( world, &radar );

	// add to the list of radar events
	m_event[ m_nextFreeRadarEvent ].type = type;
	m_event[ m_nextFreeRadarEvent ].active = TRUE;
	m_event[ m_nextFreeRadarEvent ].createFrame = TheGameLogic->getFrame();
	m_event[ m_nextFreeRadarEvent ].dieFrame = TheGameLogic->getFrame() + LOGICFRAMES_PER_SECOND * secondsToLive;
	m_event[ m_nextFreeRadarEvent ].fadeFrame = m_event[ m_nextFreeRadarEvent ].dieFrame - LOGICFRAMES_PER_SECOND * secondsBeforeDieToFade;
	m_event[ m_nextFreeRadarEvent ].color1 = *color1;
	m_event[ m_nextFreeRadarEvent ].color2 = *color2;
	m_event[ m_nextFreeRadarEvent ].worldLoc = *world;
	m_event[ m_nextFreeRadarEvent ].radarLoc = radar;
	m_event[ m_nextFreeRadarEvent ].soundPlayed = FALSE;

	// record the index of this, our "last" radar event.
	if ( type != RADAR_EVENT_BEACON_PULSE )
		m_lastRadarEvent = m_nextFreeRadarEvent;

	//
	// increment the next radar event index, wrapping to the beginning.  If we ever have so many
	// events that they fill up the buffer the oldest ones will just drop off, eh ... should be fine.
	//
	m_nextFreeRadarEvent++;
	if( m_nextFreeRadarEvent >= MAX_RADAR_EVENTS )
		m_nextFreeRadarEvent = 0;

}  // end createEvent

//-------------------------------------------------------------------------------------------------
/** Get the last event position, if any.
	* Returns TRUE if event was found
	* Returns FALSE if there is no "last event" */
//-------------------------------------------------------------------------------------------------
Bool Radar::getLastEventLoc( Coord3D *eventPos )
{

	// if we have an index for the last event, one was present
	if( m_lastRadarEvent != -1 )
	{

		if( eventPos )
			*eventPos = m_event[ m_lastRadarEvent ].worldLoc;
		return TRUE;

	}  // end if

	return FALSE;  // no last event

}  // end getLastEventLoc

// ------------------------------------------------------------------------------------------------
/** Try to create a radar event for "we're under attack".  This will be called every time
	* actual damage is dealt to an object that the player owns that shows up on the radar.
	* We don't want to create under attack events every time we are damaged and we also want
	* to limit them based on time and local area of other recent attack events */
// ------------------------------------------------------------------------------------------------
void Radar::tryUnderAttackEvent( const Object *obj )
{

	// sanity
	if( obj == NULL )
		return;

	// try to create the event
	Bool eventCreated = tryEvent( RADAR_EVENT_UNDER_ATTACK, obj->getPosition() );

	// if event created, do some more feedback
	if( eventCreated )
	{

		TheControlBar->triggerRadarAttackGlow();
		//
		///@todo Should make an INI data driven table for radar event strings, and audio events
		//
		// UI feedback for being under attack (note that we display these messages and audio
		// queues even if we don't have a radar)
		//
		Player *player = ThePlayerList->getLocalPlayer();

		// create a message for the attack event
		if( obj->isKindOf( KINDOF_INFANTRY ) || obj->isKindOf( KINDOF_VEHICLE ) )
		{
			AudioEventRTS unitAttackSound;
			if( obj->isKindOf(KINDOF_HARVESTER) )
			{
				// display special message
				TheInGameUI->message( "RADAR:HarvesterUnderAttack" );

				// play special audio event
				unitAttackSound = TheAudio->getMiscAudio()->m_radarHarvesterUnderAttackSound;
			}
			else
			{
				// display message
				TheInGameUI->message( "RADAR:UnitUnderAttack" );
				
				// play audio event
				unitAttackSound = TheAudio->getMiscAudio()->m_radarStructureUnderAttackSound;
			}
			unitAttackSound.setPlayerIndex(player->getPlayerIndex());
			TheAudio->addAudioEvent( &unitAttackSound );

		}  // end if
		else if( obj->isKindOf( KINDOF_STRUCTURE ) && obj->isKindOf( KINDOF_MP_COUNT_FOR_VICTORY ) )
		{
			// play EVA. If its our object, play Base under attack.
			if (obj->getControllingPlayer()->isLocalPlayer())
				TheEva->setShouldPlay(EVA_BaseUnderAttack);
			else if (ThePlayerList->getLocalPlayer()->getRelationship(obj->getTeam()) == ALLIES)
				TheEva->setShouldPlay(EVA_AllyUnderAttack);

			// display message
			TheInGameUI->message( "RADAR:StructureUnderAttack" );

			// play audio event
			static AudioEventRTS structureAttackSound = TheAudio->getMiscAudio()->m_radarStructureUnderAttackSound;
			structureAttackSound.setPlayerIndex(player->getPlayerIndex());
			TheAudio->addAudioEvent( &structureAttackSound );

		}  // end else if
		else
		{

			// display message
			TheInGameUI->message( "RADAR:UnderAttack" );

			// play audio event
			static AudioEventRTS underAttackSound = TheAudio->getMiscAudio()->m_radarStructureUnderAttackSound;
			underAttackSound.setPlayerIndex(player->getPlayerIndex());
			TheAudio->addAudioEvent( &underAttackSound );

		}  // end else

	}  // end if

}  // end tryUnderAttackEvent

// ------------------------------------------------------------------------------------------------
/** Try to create a radar event for "infiltration".
		This happens whenever a unit is hijacked, defected, converted to carbomb, hacked, or 
		otherwise snuck into */ 
// ------------------------------------------------------------------------------------------------
void Radar::tryInfiltrationEvent( const Object *obj )
{

	//Sanity!
	if( !obj )
	{
		return;
	}

	// We should only be warned against infiltrations that are taking place against us.
	if( obj->getControllingPlayer() != ThePlayerList->getLocalPlayer() )
		return;

	// create the radar event
	createEvent( obj->getPosition(), RADAR_EVENT_INFILTRATION );

	//
	///@todo Should make an INI data driven table for radar event strings, and audio events
	//
	// UI feedback for being under attack (note that we display these messages and audio
	// queues even if we don't have a radar)
	//
	Player *player = ThePlayerList->getLocalPlayer();

	// display message
	TheInGameUI->message( "RADAR:Infiltration" );

	// play audio event
	static AudioEventRTS infiltrationWarningSound = TheAudio->getMiscAudio()->m_radarInfiltrationSound;
	infiltrationWarningSound.setPlayerIndex(player->getPlayerIndex());
	TheAudio->addAudioEvent( &infiltrationWarningSound );

}  // end tryInfiltrationEvent

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool Radar::tryEvent( RadarEventType event, const Coord3D *pos )
{

	// sanity
	if( event <= RADAR_EVENT_INVALID || event >= RADAR_EVENT_NUM_EVENTS || pos == NULL )
		return FALSE;

	// see if there was an event of this type within the given range within the given time
	UnsignedInt currentFrame = TheGameLogic->getFrame();
	const Real closeEnoughDistanceSq = 250.0f * 250.0f;
	const UnsignedInt framesBetweenEvents = LOGICFRAMES_PER_SECOND * 10;
	//

	// see if there was any matching radar event within range of this one
	// that wasn't too long ago, if there was we won't create another and get outta here
	//
	for( Int i = 0; i < MAX_RADAR_EVENTS; ++i )
	{
	
		// only pay attention to under attack events
		if( m_event[ i ].type == event )
		{

			// get distance from our new event location to this event location in 2D
			Real distSquared = m_event[ i ].worldLoc.x - pos->x * m_event[ i ].worldLoc.x - pos->x +
												 m_event[ i ].worldLoc.y - pos->y * m_event[ i ].worldLoc.y - pos->y;

			if( distSquared <= closeEnoughDistanceSq )
			{

				// finally only reject making a new event of this existing one is "recent enough"
				if( currentFrame - m_event[ i ].createFrame < framesBetweenEvents )
					return FALSE;  // reject it

			}  // end if

		}  // end if

	}  // end for i

	// if we got here then we want to create a new event
	createEvent( pos, event );

	// return TRUE for successfully created event
	return TRUE;

}  // end tryEvent


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Radar::refreshTerrain( TerrainLogic *terrain )
{

	// no future queue is valid now
	m_queueTerrainRefreshFrame = 0;

}  // end refreshTerrain

// ------------------------------------------------------------------------------------------------
/** Queue a refresh of the radar terrain, we have this so that if there is code that
	* rapidly needs to refresh the radar, it should use this so we aren't continually
	* rebuilding the radar graphic because that process is slow.  If you need to update
	* the terrain on the radar immediately use refreshTerrain() */
// ------------------------------------------------------------------------------------------------
void Radar::queueTerrainRefresh( void )
{

	//
	// we just simply overwrite the frame we have recorded for a radar refresh.  If there was
	// already one there, it's simply just forgotten and whatever changes we wanted to see
	// with that refresh will have to wait until enough time has passed to show these
	// changes as well.  why you ask ... well, because if we're calling this in close enough
	// proximity for us to overwrite something, we're changing the terrain features 
	// quite often and can't afford the expense of rebuilding the radar visual
	//
	m_queueTerrainRefreshFrame = TheGameLogic->getFrame();

}  // end queueTerrainRefresh

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void Radar::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer a radar object list given the head pointer as a parameter
	* Version Info;
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
static void xferRadarObjectList( Xfer *xfer, RadarObject **head )
{
	RadarObject *radarObject;

	// sanity
	DEBUG_ASSERTCRASH( head != NULL, ("xferRadarObjectList - Invalid parameters" ));

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// write could of objects in list
	UnsignedShort count = 0;
	for( radarObject = *head; radarObject; radarObject = radarObject->friend_getNext() )
		count++;
	xfer->xferUnsignedShort( &count );

	// xfer the list data
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// save each object
		for( radarObject = *head; radarObject; radarObject = radarObject->friend_getNext() )
		{

			// save this object
			xfer->xferSnapshot( radarObject );

		}  // end for, radarObject

	}  // end if, save
	else
	{

		// the list should be empty at this point as we are loading it as a whole
		if( *head != NULL )
		{
#if 1
			// srj sez: yeah, it SHOULD be, but a few rogue things come into existence (via their ctor) preloaded
			// with stuff (eg, "CabooseFullOfTerrorists"). we immediately destroy 'em, but they don't go away just yet.
			// so just ignore 'em if possible.
			for( radarObject = *head; radarObject; radarObject = radarObject->friend_getNext() )
			{
				if (!radarObject->friend_getObject()->isDestroyed())
				{
					DEBUG_CRASH(( "xferRadarObjectList - List head should be NULL, or contain only destroyed objects" ));
					throw SC_INVALID_DATA;
				}
			}
#else
			DEBUG_CRASH(( "xferRadarObjectList - List head should be NULL, but isn't" ));
			throw SC_INVALID_DATA;
#endif
		}  // end if

		// read each element
		for( UnsignedShort i = 0; i < count; ++i )
		{

			// alloate a new radar object 
			radarObject = newInstance(RadarObject);

			// link to the end of the list
			if( *head == NULL )
				*head = radarObject;
			else
			{

				RadarObject *other;
				for( other = *head; other->friend_getNext() != NULL; other = other->friend_getNext() )
				{
				}  // end for, other

				// set the end of the list to point to the new object
				other->friend_setNext( radarObject );

			}  // end else

			// load the data
			xfer->xferSnapshot( radarObject );
						
		}  // end for i

	}  // end else, load

}  // end xferRadarObjectList

// ------------------------------------------------------------------------------------------------
/** Xfer Method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void Radar::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// radar hidden
	xfer->xferBool( &m_radarHidden );

	// radar force on
	xfer->xferBool( &m_radarForceOn );

	// save our local object list
	xferRadarObjectList( xfer, &m_localObjectList );

	// save the regular object list
	xferRadarObjectList( xfer, &m_objectList );

	// save the radar event count and data
	UnsignedShort eventCountVerify = MAX_RADAR_EVENTS;
	UnsignedShort eventCount = eventCountVerify;
	xfer->xferUnsignedShort( &eventCount );
	if( eventCount != eventCountVerify )
	{

		DEBUG_CRASH(( "Radar::xfer - size of MAX_RADAR_EVENTS has changed, you must version this xfer method to accomodate the new array size.  Was '%d' and is now '%d'",
									eventCount, eventCountVerify ));
		throw SC_INVALID_DATA;

	}  // end if
	for( UnsignedShort i = 0; i < eventCount; ++i )
	{

		// xfer event data
		xfer->xferUser( &m_event[ i ].type, sizeof( RadarEventType ) );
		xfer->xferBool( &m_event[ i ].active );
		xfer->xferUnsignedInt( &m_event[ i ].createFrame );
		xfer->xferUnsignedInt( &m_event[ i ].dieFrame );
		xfer->xferUnsignedInt( &m_event[ i ].fadeFrame );
		xfer->xferRGBAColorInt( &m_event[ i ].color1 );
		xfer->xferRGBAColorInt( &m_event[ i ].color2 );
		xfer->xferCoord3D( &m_event[ i ].worldLoc );
		xfer->xferICoord2D( &m_event[ i ].radarLoc );
		xfer->xferBool( &m_event[ i ].soundPlayed );
		
	}  // end for i

	// next event index
	xfer->xferInt( &m_nextFreeRadarEvent );

	// last event index
	xfer->xferInt( &m_lastRadarEvent );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void Radar::loadPostProcess( void )
{

	//
	// refresh the radar texture now that all the objects (specifically bridges) have
	// been loaded with their correct damage states from save game file
	//
	refreshTerrain( TheTerrainLogic );

}  // end loadPostProcess

// ------------------------------------------------------------------------------------------------
/** Is the priority type passed in a "visible" one that can show up on the radar */
// ------------------------------------------------------------------------------------------------
Bool Radar::isPriorityVisible( RadarPriorityType priority )
{

	switch( priority )
	{

		case RADAR_PRIORITY_INVALID:
		case RADAR_PRIORITY_NOT_ON_RADAR:
			return FALSE;

		default:
			return TRUE;

	}  // end switch

}  // end isPriorityVisible
