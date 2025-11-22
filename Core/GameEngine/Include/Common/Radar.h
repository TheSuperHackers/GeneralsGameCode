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

// FILE: Radar.h //////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, January 2002
// Desc:   Logical radar implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "Common/SubsystemInterface.h"
#include "Common/GameMemory.h"
#include "GameClient/Display.h"	// for ShroudLevel
#include "GameClient/Color.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class GameWindow;
class Object;
class Player;
class TerrainLogic;

// GLOBAL /////////////////////////////////////////////////////////////////////////////////////////
//
// the following is used for the resolution of the radar "cells" ... this is how accurate
// the radar is and also reflects directly the size of the image we build ... which with
// WW3D must be a square power of two as well
//
enum
{
	RADAR_CELL_WIDTH  = 128,	// radar created at this horz resolution
	RADAR_CELL_HEIGHT = 128   // radar created at this vert resolution
};

//-------------------------------------------------------------------------------------------------
/** These event types determine the colors radar events happen in to make it easier for us
	* to play events with a consistent color scheme */
//-------------------------------------------------------------------------------------------------
enum RadarEventType CPP_11(: Int)
{
	RADAR_EVENT_INVALID = 0,
	RADAR_EVENT_CONSTRUCTION,
	RADAR_EVENT_UPGRADE,
	RADAR_EVENT_UNDER_ATTACK,
	RADAR_EVENT_INFORMATION,
	RADAR_EVENT_BEACON_PULSE,
	RADAR_EVENT_INFILTRATION, //for defection, hijacking, hacking, carbombing, and other sneaks
	RADAR_EVENT_BATTLE_PLAN,
	RADAR_EVENT_STEALTH_DISCOVERED,		// we discovered a stealth unit
	RADAR_EVENT_STEALTH_NEUTRALIZED,	// our stealth unit has been revealed
	RADAR_EVENT_FAKE,					//Internally creates a radar event, but doesn't notify the player (unit lost
														//for example, so we can use the spacebar to jump to the event).

	RADAR_EVENT_NUM_EVENTS

};

// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Radar objects are objects that are on the radar, go figure :) */
//-------------------------------------------------------------------------------------------------
class RadarObject : public MemoryPoolObject,
										public Snapshot
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( RadarObject, "RadarObject" )

public:

	RadarObject( void );
	// destructor prototype defined by memory pool glue

	// color management
	void setColor( Color c ) { m_color = c; }
	inline Color getColor( void ) const { return m_color; }

	inline void friend_setObject( Object *obj ) { m_object = obj; }
	inline Object *friend_getObject( void ) { return m_object; }
	inline const Object *friend_getObject( void ) const { return m_object; }

	inline void friend_setNext( RadarObject *next ) { m_next = next; }
	inline RadarObject *friend_getNext( void ) { return m_next; }
	inline const RadarObject *friend_getNext( void ) const { return m_next; }

	Bool isTemporarilyHidden() const;
	static Bool isTemporarilyHidden(const Object* obj);

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	Object *m_object;				///< the object
	RadarObject *m_next;		///< next radar object
	Color m_color;					///< color to draw for this object on the radar

};

//-------------------------------------------------------------------------------------------------
/** Radar priorities.  Keep this in sync with the priority names list below */
//-------------------------------------------------------------------------------------------------
enum RadarPriorityType CPP_11(: Int)
{
	RADAR_PRIORITY_INVALID,					// a priority that has not been set (in general it won't show up on the radar)
	RADAR_PRIORITY_NOT_ON_RADAR,		// object specifically forbidden from being on the radar
	RADAR_PRIORITY_STRUCTURE,				// structure level drawing priority
	RADAR_PRIORITY_UNIT,						// unit level drawing priority
	RADAR_PRIORITY_LOCAL_UNIT_ONLY,	// unit priority, but only on the radar if controlled by the local player

	RADAR_PRIORITY_NUM_PRIORITIES
};
#ifdef DEFINE_RADAR_PRIORITY_NAMES
static const char *const RadarPriorityNames[] =
{
	"INVALID",											// a priority that has not been set (in general it won't show up on the radar)
	"NOT_ON_RADAR",									// object specifically forbidden from being on the radar
	"STRUCTURE",										// structure level drawing priority
	"UNIT",													// unit level drawing priority
	"LOCAL_UNIT_ONLY",							// unit priority, but only on the radar if controlled by the local player

	NULL
};
static_assert(ARRAY_SIZE(RadarPriorityNames) == RADAR_PRIORITY_NUM_PRIORITIES + 1, "Incorrect array size");
#endif  // DEFINE_RADAR_PRIOTITY_NAMES

//-------------------------------------------------------------------------------------------------
/** Interface for the radar */
//-------------------------------------------------------------------------------------------------
class Radar : public Snapshot,
							public SubsystemInterface
{

public:

	Radar( void );
	virtual ~Radar( void );

	virtual void init( void ) { }														///< subsystem initialization
	virtual void reset( void );															///< subsystem reset
	virtual void update( void );														///< subsystem per frame update

	// is the game window parameter the radar window
	Bool isRadarWindow( GameWindow *window ) { return (m_radarWindow == window) && (m_radarWindow != NULL); }

	Bool radarToWorld( const ICoord2D *radar, Coord3D *world );		///< radar point to world point on terrain
	Bool radarToWorld2D( const ICoord2D *radar, Coord3D *world );		///< radar point to world point (x,y only!)
	Bool worldToRadar( const Coord3D *world, ICoord2D *radar );		///< translate world point to radar (x,y)
	Bool localPixelToRadar( const ICoord2D *pixel, ICoord2D *radar );	///< translate pixel (with UL of radar being (0,0)) to logical radar coordinates
	Bool screenPixelToWorld( const ICoord2D *pixel, Coord3D *world ); ///< translate pixel (with UL of the screen being (0,0)) to world position in the world
	Object *objectUnderRadarPixel( const ICoord2D *pixel );				///< return the object (if any) represented by the pixel coordinates passed in
	void findDrawPositions( Int startX, Int startY, Int width, Int height,
													ICoord2D *ul, ICoord2D *lr );					///< make translation for screen area of radar square to scaled aspect ratio preserving points inside the radar area

	// priority inquiry
	static Bool isPriorityVisible( RadarPriorityType priority );		///< is the priority passed in a "visible" one on the radar

	// radar events
	void createEvent( const Coord3D *world, RadarEventType type, Real secondsToLive = 4.0f );	///< create radar event at location in world
	void createPlayerEvent( Player *player, const Coord3D *world, RadarEventType type, Real secondsToLive = 4.0f );  ///< create radar event using player colors

	Bool getLastEventLoc( Coord3D *eventPos );							///< get last event loc (if any)
	void tryUnderAttackEvent( const Object *obj );					///< try to make an "under attack" event if it's the proper time
	void tryInfiltrationEvent( const Object *obj );					///< try to make an "infiltration" event if it's the proper time
 	Bool tryEvent( RadarEventType event, const Coord3D *pos );	///< try to make a "stealth" event

	// adding and removing objects from the radar
	virtual Bool addObject( Object *obj ); ///< add object to radar
	virtual Bool removeObject( Object *obj ); ///< remove object from radar

	// radar options
	void hide( Int playerIndex, Bool hide ) { m_radarHidden[playerIndex] = hide; } ///< hide/show the radar
	Bool isRadarHidden( Int playerIndex ) { return m_radarHidden[playerIndex]; } ///< is radar hidden
	// other radar option methods here like the ability to show a certain
	// team, show buildings, show units at all, etc

	// forcing the radar on/off regardless of player situation
	void forceOn( Int playerIndex, Bool force ) { m_radarForceOn[playerIndex] = force; } ///< force the radar to be on
	Bool isRadarForced( Int playerIndex ) { return m_radarForceOn[playerIndex]; } ///< is radar forced on?

	/// refresh the water values for the radar
	virtual void refreshTerrain( TerrainLogic *terrain );

	/// refresh the radar when the state of world objects changes drastically
	virtual void refreshObjects() {};

	/// queue a refresh of the terrain at the next available time
	virtual void queueTerrainRefresh( void );

	virtual void newMap( TerrainLogic *terrain );	///< reset radar for new map

	virtual void draw( Int pixelX, Int pixelY, Int width, Int height ) = 0;	///< draw the radar

	/// empty the entire shroud
	virtual void clearShroud() = 0;

	/// set the shroud level at shroud cell x,y
	virtual void setShroudLevel( Int x, Int y, CellShroudStatus setting ) = 0;

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	/// internal method for creating a radar event with specific colors
	void internalCreateEvent( const Coord3D *world, RadarEventType type, Real secondsToLive,
														const RGBAColorInt *color1, const RGBAColorInt *color2 );

	void deleteList( RadarObject **list );
	void deleteListResources( void );			///< delete list radar resources used
	Bool deleteFromList( Object *obj, RadarObject **list );	///< try to remove object from specific list

	inline Real getTerrainAverageZ() const { return m_terrainAverageZ; }
	inline Real getWaterAverageZ() const { return m_waterAverageZ; }

	void clearAllEvents( void );					///< remove all radar events in progress

	// search the object list for an object that maps to the given logical radar coordinates
	Object *searchListForRadarLocationMatch( RadarObject *listHead, ICoord2D *radarMatch );

	void linkRadarObject( RadarObject *newObj, RadarObject **list );
	void assignObjectColorToRadarObject( RadarObject *radarObj, Object *obj );

	Bool m_radarHidden[MAX_PLAYER_COUNT]; ///< true when radar is not visible
	Bool m_radarForceOn[MAX_PLAYER_COUNT]; ///< true when radar is forced to be on

	RadarObject *m_objectList;						///< list of objects in the radar
	RadarObject *m_localObjectList;				/** list of objects for the local player, sorted
																					* in exactly the same priority as the regular
																					* object list for all other objects */

	// TheSuperHackers @bugfix xezon 22/11/2025 Now stores local heroes in a separate list,
	// because they are treated with special icons but should otherwise work like all other
	// radar objects. In retail version, the cached hero object data was able to dangle
	// for a few frames and cause undefined behavior.
	RadarObject *m_localHeroObjectList; ///< list of hero objects for the local player

	Real m_terrainAverageZ;								///< average Z for terrain samples
	Real m_waterAverageZ;									///< average Z for water samples

	//
	// when dealing with world sampling we will sample at these intervals so that
	// the whole map can be accounted for within our RADAR_CELL_WIDTH and
	// RADAR_CELL_HEIGHT resolutions
	//
	Real m_xSample;
	Real m_ySample;

	enum { MAX_RADAR_EVENTS = 64 };
	struct RadarEvent
	{
		RadarEventType type;								///< type of this radar event
		Bool active;												///< TRUE when event is "active", otherwise it's just historical information in the event array to look through
		UnsignedInt createFrame;						///< frame event was created on
		UnsignedInt dieFrame;								///< frame the event will go away on
		UnsignedInt fadeFrame;							///< start fading out on this frame
		RGBAColorInt color1;								///< color 1 for drawing
		RGBAColorInt color2;								///< color 2 for drawing
		Coord3D worldLoc;										///< location of event in the world
		ICoord2D radarLoc;									///< 2D radar location of the event
		Bool soundPlayed;										///< TRUE when we have played the radar sound for this
	};
	RadarEvent m_event[ MAX_RADAR_EVENTS ];///< our radar events
	Int m_nextFreeRadarEvent;							///< index into m_event for where to store the next event
	Int m_lastRadarEvent;									///< index of the most recent radar event

	GameWindow *m_radarWindow;						///< window we display the radar in

	Region3D m_mapExtent;									///< extents of the current map

	UnsignedInt m_queueTerrainRefreshFrame;  ///< frame we requested the last terrain refresh on

};

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern Radar *TheRadar;  ///< the radar singleton extern

// TheSuperHackers @feature helmutbuhler 10/04/2025
// Radar that does nothing. Used for Headless Mode.
class RadarDummy : public Radar
{
public:
	virtual void draw(Int pixelX, Int pixelY, Int width, Int height) { }
	virtual void clearShroud() { }
	virtual void setShroudLevel(Int x, Int y, CellShroudStatus setting) { }
};
