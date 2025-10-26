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


// MapObject.h
// Class to encapsulate height map.
// Author: John Ahlquist, April 2001

#pragma once

#include "Common/Dict.h"
#include "Common/GameMemory.h"
#include "GameClient/TerrainRoads.h"

/** MapObject class
Not ref counted.  Do not store pointers to this class.  */
class WorldHeightMap;
class RenderObjClass;
class ThingTemplate;
class Shadow;
enum WaypointID CPP_11(: Int);

#define MAP_XY_FACTOR			(10.0f)	 //How wide and tall each height map square is in world space.
#define MAP_HEIGHT_SCALE	(MAP_XY_FACTOR/16.0f)		//divide all map heights by 8.

// m_flags bit values.
enum {
	FLAG_DRAWS_IN_MIRROR		= 0x00000001,			 ///< If set, draws in water mirror.
	FLAG_ROAD_POINT1				= 0x00000002,			 ///< If set, is the first point in a road segment.
	FLAG_ROAD_POINT2				= 0x00000004,			 ///< If set, is the second point in a road segment.
	FLAG_ROAD_FLAGS					= (FLAG_ROAD_POINT1|FLAG_ROAD_POINT2),	 ///< If nonzero, object is a road piece.
	FLAG_ROAD_CORNER_ANGLED	= 0x00000008,			 ///< If set, the road corner is angled rather than curved.
	FLAG_BRIDGE_POINT1			= 0x00000010,			 ///< If set, is the first point in a bridge.
	FLAG_BRIDGE_POINT2			= 0x00000020,			 ///< If set, is the second point in a bridge.
	FLAG_BRIDGE_FLAGS				= (FLAG_BRIDGE_POINT1|FLAG_BRIDGE_POINT2),	 ///< If nonzero, object is a bridge piece.
	FLAG_ROAD_CORNER_TIGHT	= 0x00000040,
	FLAG_ROAD_JOIN					= 0x00000080,			 ///< If set, this road end does a generic alpha join.
	FLAG_DONT_RENDER				= 0x00000100			 ///< If set, do not render this object. Only WB pays attention to this. (Right now, anyways)
};

class MapObject : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(MapObject, "MapObject")

// friend doesn't play well with MPO -- srj
//	friend class WorldHeightMap;
//	friend class WorldHeightMapEdit;
//	friend class AddObjectUndoable;
//	friend class DeleteInfo;

	enum
	{
		MO_SELECTED = 0x01,
		MO_LIGHT = 0x02,
		MO_WAYPOINT = 0x04,
		MO_SCORCH = 0x08
	};

	// This data is currently written out into the map data file.
	Coord3D								m_location;					///< Location of the center of the object.
	AsciiString						m_objectName;				///< The object name.
	const ThingTemplate*	m_thingTemplate; ///< thing template for map object
	Real									m_angle;						///< positive x is 0 degrees, angle is counterclockwise in degrees.
	MapObject*						m_nextMapObject;		///< linked list.
	Int										m_flags;						///< Bit flags.
	Dict									m_properties;				///< general property sheet.
	// This data is runtime data that is used by the worldbuider editor, but
	// not saved in the map file.
	Int										m_color;		 ///< Display color.
	RenderObjClass*				m_renderObj; ///< object that renders in the 3d scene.
	Shadow*								m_shadowObj; ///< object that renders shadow in the 3d scene.
	RenderObjClass*				m_bridgeTowers[ BRIDGE_MAX_TOWERS ];		///< for bridge towers
	Int										m_runtimeFlags;

public:
	static MapObject *TheMapObjectListPtr;
	static Dict TheWorldDict;

public:
	MapObject(Coord3D loc, AsciiString name, Real angle, Int flags, const Dict* props,
						const ThingTemplate *thingTemplate );
	//~MapObject(void);		///< Note that deleting the head of a list deletes all linked objects in the list.

public:

	Dict *getProperties() { return &m_properties; }	///< return the object's property sheet.

	void setNextMap(MapObject *nextMap) {m_nextMapObject = nextMap;} ///< Link the next map object.
	const Coord3D *getLocation(void) const {return &m_location;} ///< Get the center point.
	Real getAngle(void) const {return m_angle;} ///< Get the angle.
	Int getColor(void) const {return m_color;} ///< Gets whatever ui color we set.
	void setColor(Int color) {m_color=color;} ///< Sets the ui color.
	AsciiString getName(void) const {return m_objectName;} ///< Gets the object name
	void setName(AsciiString name); ///< Sets the object name
	void setThingTemplate( const ThingTemplate* thing ); ///< set template
	const ThingTemplate *getThingTemplate( void ) const;
	MapObject *getNext(void) const {return m_nextMapObject;}		 ///< Next map object in the list.  Not a copy, don't delete it.
	MapObject *duplicate(void);		 ///< Allocates a copy.  Caller is responsible for delete-ing this when done with it.

	void setAngle(Real angle) {m_angle = normalizeAngle(angle);}
	void setLocation(Coord3D *pLoc) {m_location = *pLoc;}
	void setFlag(Int flag) {m_flags |= flag;}
	void clearFlag(Int flag) {m_flags &= (~flag);}
	Bool getFlag(Int flag) const {return (m_flags&flag)?true:false;}
	Int getFlags(void) const {return (m_flags);}

	Bool isSelected(void) const {return (m_runtimeFlags & MO_SELECTED) != 0;}
	void setSelected(Bool sel) { if (sel) m_runtimeFlags |= MO_SELECTED; else m_runtimeFlags &= ~MO_SELECTED; }

	Bool isLight(void) const {return (m_runtimeFlags & MO_LIGHT) != 0;}
	Bool isWaypoint(void) const {return (m_runtimeFlags & MO_WAYPOINT) != 0;}
	Bool isScorch(void) const {return (m_runtimeFlags & MO_SCORCH) != 0;}

	void setIsLight() {m_runtimeFlags |= MO_LIGHT;}
	void setIsWaypoint() { m_runtimeFlags |= MO_WAYPOINT; }
	void setIsScorch() { m_runtimeFlags |= MO_SCORCH; }

	void setRenderObj(RenderObjClass *pObj);
	RenderObjClass *getRenderObj(void) const {return m_renderObj;}
	void setShadowObj(Shadow *pObj)	{m_shadowObj=pObj;}
	Shadow *getShadowObj(void) const {return m_shadowObj;}

	RenderObjClass* getBridgeRenderObject( BridgeTowerType type );
	void setBridgeRenderObject( BridgeTowerType type, RenderObjClass* renderObj );

	WaypointID getWaypointID();
	AsciiString getWaypointName();
	void setWaypointID(Int i);
	void setWaypointName(AsciiString n);

	// calling validate will call verifyValidTeam and verifyValidUniqueID.
	void validate(void);

	// verifyValidTeam will either place the map object on an approrpriate team, or leave the
	// current team (if it is valid)
	void verifyValidTeam(void);

	// verifyValidUniqueID will ensure that this unit isn't sharing a number with another unit.
	void verifyValidUniqueID(void);

	// The fast version doesn't attempt to verify uniqueness. It goes
	static void fastAssignAllUniqueIDs(void);


	static MapObject *getFirstMapObject(void) { return TheMapObjectListPtr; }
	static Dict* getWorldDict() { return &TheWorldDict; }
	static Int countMapObjectsWithOwner(const AsciiString& n);
};
