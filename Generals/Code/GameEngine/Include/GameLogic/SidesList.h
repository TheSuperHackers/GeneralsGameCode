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


// SidesList.h
// Class to encapsulate Sides and Build Lists for maps.
// Author: John Ahlquist, November 2001

#pragma once

#include "Common/Dict.h"
#include "Common/Errors.h"
#include "Common/GameType.h"
#include "Common/Snapshot.h"
#include "Common/GameMemory.h"
#include "Common/STLTypedefs.h"

class DataChunkInput;
struct DataChunkInfo;
class DataChunkOutput;
class BuildListInfo;
class RenderObjClass;
class ScriptList;
class Shadow;

// ----------------------------------------------------------------------------------------------
/**
	This is a class that describes a Side, including build list.
	Note that a side corresponds to a Player in the game.  The lightweight
	Side is used to give the WB Editor somewhere to hang build lists.
*/
class SidesInfo
{
protected:
	BuildListInfo*	m_pBuildList;		///< linked list.
	Dict						m_dict;					///< general player dict.
	ScriptList			*m_scripts;			///< linked list.

public:
	SidesInfo(void);
	SidesInfo(const SidesInfo& thatref);
	~SidesInfo(void);
	void init(const Dict* d);
	void clear() { init(NULL); }
	Dict* getDict() { return &m_dict; }
	void addToBuildList(BuildListInfo *pBuildList, Int position);
	Int removeFromBuildList(BuildListInfo *pBuildList);
	void reorderInBuildList(BuildListInfo *pBuildList, Int newPosition);
	BuildListInfo* getBuildList(void) {return m_pBuildList;} ///< Gets the build list.
	void releaseBuildList(void)  {m_pBuildList=NULL;} ///< Used when the build list is passed to class Player.
	ScriptList *getScriptList(void) {return(m_scripts);};
	void setScriptList(ScriptList *pScriptList) {m_scripts = pScriptList;};

	// ug, I hate having to overload stuff, but this makes it a lot easier to make copies safely
	SidesInfo& operator=(const SidesInfo& that);
};

// ----------------------------------------------------------------------------------------------
class TeamsInfo
{
private:
	Dict m_dict;
public:
	Dict* getDict() { return &m_dict; }
	void init(const Dict* d) { m_dict.clear(); if (d) m_dict = *d; }
	void clear() { init(NULL); }
};


// ----------------------------------------------------------------------------------------------
// a wrapper class to make this a little cleaner.
class TeamsInfoRec
{
private:
	Int					m_numTeams;
	Int					m_numTeamsAllocated;
	TeamsInfo*	m_teams;

public:
	TeamsInfoRec();
	TeamsInfoRec(const TeamsInfoRec& thatref);
	~TeamsInfoRec();
	TeamsInfoRec& operator=(const TeamsInfoRec& thatref);
	void clear();
	TeamsInfo *findTeamInfo(AsciiString name, Int* index);
	void addTeam(const Dict* d);
	void removeTeam(Int i);
	Int getNumTeams() const { return m_numTeams; }
	TeamsInfo * getTeamInfo(Int team)
	{
		if (team>=0&&team<m_numTeams)
		{
			return(&m_teams[team]);
		}
		DEBUG_CRASH(("Out of range."));
		throw ERROR_BAD_ARG;
		return NULL;
	}
};

// ----------------------------------------------------------------------------------------------
/**
	This is a singleton class that maintains the list of Sides.
	In RTS it is associated with the Player singleton.
	Note that a side corresponds to a Player in the game.  The lightweight
	Side is used to give the WB Editor somewhere to hang build lists, as
	it doesn't instantiate the Player subsystem

	jba.
*/
class SidesList : public SubsystemInterface,
									public Snapshot
{

public:

	SidesList();
	~SidesList();

	void init() { }
	void update() { }
	void reset();

	/// Reads sides (including build list info && player dicts.)
	static Bool ParseSidesDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);
	/// Writes sides (including build list info.)
	static void WriteSidesDataChunk(DataChunkOutput &chunkWriter);

	Int getNumSides() { return m_numSides; }
	void emptySides();
	void addSide(const Dict* d);
	void removeSide(Int i);
	inline SidesInfo *getSideInfo(Int side);
	inline SidesInfo *getSkirmishSideInfo(Int side);
	Int getNumSkirmishSides() { return m_numSkirmishSides; }
	SidesInfo *findSideInfo(AsciiString name, Int* index = NULL);
	SidesInfo *findSkirmishSideInfo(AsciiString name, Int* index = NULL);

	void prepareForMP_or_Skirmish(void); // After a map is loaded, save & clear any players.

	Int getNumTeams() { return m_teamrec.getNumTeams(); }
	void emptyTeams();
	void addTeam(const Dict* d);
	void removeTeam(Int i);

	inline TeamsInfo *getTeamInfo(Int team);
	inline TeamsInfo *getSkirmishTeamInfo(Int team);
	Int getNumSkirmishTeams() { return m_skirmishTeamrec.getNumTeams(); }
 	void addSkirmishTeam(const Dict* d);

	TeamsInfo *findTeamInfo(AsciiString name, Int* index = NULL);

	Bool isPlayerDefaultTeam(TeamsInfo *t);

	void clear();
	Bool validateSides(void);

	void addPlayerByTemplate(AsciiString playerTemplateName);

	enum
	{
		MAX_TEAM_DEPTH = 3
	};

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	Int					m_numSides;
	SidesInfo		m_sides[MAX_PLAYER_COUNT];

	Int					m_numSkirmishSides;
	SidesInfo		m_skirmishSides[MAX_PLAYER_COUNT];

	TeamsInfoRec	m_teamrec;
	TeamsInfoRec	m_skirmishTeamrec;

	Bool validateAllyEnemyList(const AsciiString& tname, AsciiString& allies);
};


inline TeamsInfo * SidesList::getTeamInfo(Int team)
{
	return m_teamrec.getTeamInfo(team);
}

inline TeamsInfo * SidesList::getSkirmishTeamInfo(Int team)
{
	return m_skirmishTeamrec.getTeamInfo(team);
}

inline SidesInfo * SidesList::getSideInfo(Int side)
{
	if (side>=0&&side<m_numSides)
	{
		return(&m_sides[side]);
	}
	DEBUG_CRASH(("Out of range."));
	throw ERROR_BAD_ARG;
	return NULL;
}

inline SidesInfo * SidesList::getSkirmishSideInfo(Int side)
{
	if (side>=0&&side<m_numSkirmishSides)
	{
		return(&m_skirmishSides[side]);
	}
	DEBUG_CRASH(("Out of range."));
	throw ERROR_BAD_ARG;
	return NULL;
}


// ----------------------------------------------------------------------------------------------
extern SidesList *TheSidesList;	 ///< singleton instance of SidesList


// ----------------------------------------------------------------------------------------------
/**
	This is a class that describes an entry in the build list.
*/
class BuildListInfo : public MemoryPoolObject, public Snapshot
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(BuildListInfo, "BuildListInfo")

// friend doesn't play well with MPO... it reveals delete, which shouldn't be visible, even to friends (srj)
//friend class SidesInfo;  // This is essentially a component of the SidesInfo data structure.
//friend class SidesList;  // This is essentially a component of the SidesInfo data structure.
//friend class BuildList;  // This is the class in WB that edits build lists.
//friend class BuildListTool; // Other class in wb that edits build lists.
//friend class WBDocUndoable; // Other class in wb that edits build lists.
//friend class Player;
//friend class AISideBuildList;
//friend class AISkirmishPlayer;

public:
	enum {UNLIMITED_REBUILDS=0xFFFFFFFF};
	enum {MAX_RESOURCE_GATHERERS = 10};
	BuildListInfo(void);
	//~BuildListInfo(void);								///< Note that deleting the head of a list deletes all linked objects in the list.

	static void parseStructure(INI *ini, void *instance, void* /*store*/, const void* /*userData*/);

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	AsciiString			m_buildingName;			///< The name of this building.
	AsciiString			m_templateName;			///< The thing template name for this model's info.
	Coord3D					m_location;					///< The location of the object.
	Coord2D					m_rallyPointOffset; ///< Offset to the natural rally point.
	Real						m_angle;						///< Initial orientation of the building.
	Bool						m_isInitiallyBuilt; ///< Whether the building is built at the start of the game.
	UnsignedInt			m_numRebuilds;			///< Number of rebuilds allowed.
	BuildListInfo*	m_nextBuildList;		///< linked list.
	AsciiString		m_script;
	Int				m_health;
	Bool			m_whiner;
	Bool			m_unsellable;
	Bool			m_repairable;
	Bool			m_automaticallyBuild;			///< If true, the ai will build.  If false, script has to enable this.


	// For WorldBuilder use only:
	RenderObjClass	*m_renderObj;				///< object that renders in the 3d scene.
	Shadow			*m_shadowObj;				///< object that renders shadows in the 3d scane.
	Bool				m_selected;					///< True if the obj is selected in the editor.

	// For solo AI use only:
	ObjectID m_objectID;								///< the object on the map instantiated by this info
	UnsignedInt m_objectTimestamp;			///< frame when object was built
	Bool		 m_underConstruction;				///< True if being constructed by dozer.
	ObjectID m_resourceGatherers[MAX_RESOURCE_GATHERERS];	///< gatherers for this supply center type building.
	Bool		 m_isSupplyBuilding;				///< Uses gatherers to get supplies.
	Int			 m_desiredGatherers;				///< Number of gatherers desired.
	Int			 m_currentGatherers;				///< Number of gatherers available.
	Bool		 m_priorityBuild;						///< Is priority build.

public:
	// srj sez: naughty public access to avoid 'friend' -- should be friend for JUST THESE, but hey
	void setNumRebuilds(UnsignedInt numRebuilds) {m_numRebuilds = numRebuilds;}
	void setNextBuildList(BuildListInfo *pNext) {m_nextBuildList = pNext;}
	void setLocation(Coord3D loc) {m_location = loc;}
	void setAngle(Real angle) {m_angle = angle;}
	void setInitiallyBuilt(Bool built) {m_isInitiallyBuilt = built;}
	void setBuildingName(AsciiString name) {m_buildingName = name;}
	void setScript(AsciiString script) {m_script = script;}
	void setHealth(Int health) {m_health = health;}
	void setWhiner(Bool whiner) {m_whiner = whiner;}
	void setUnsellable(Bool unsellable) {m_unsellable = unsellable;}
	void setRepairable(Bool repairable) {m_repairable = repairable;}

public:
	BuildListInfo *getNext(void) const {return m_nextBuildList;}
	AsciiString getBuildingName(void) const {return m_buildingName;} ///< Gets the name.
	AsciiString getTemplateName(void) const {return m_templateName;} ///< Gets the name.
	void setTemplateName(AsciiString name) {m_templateName = name;}
	Int getNumRebuilds(void) {return m_numRebuilds;}
	void decrementNumRebuilds(void);
	void incrementNumRebuilds(void);
	const Coord3D *getLocation(void) const {return &m_location;}
	const Coord2D *getRallyOffset(void) const {return &m_rallyPointOffset;}
	Real getAngle(void) const {return m_angle;}
	Bool isInitiallyBuilt(void) {return m_isInitiallyBuilt;}
	AsciiString getScript(void) {return m_script;}
	Int getHealth(void) {return m_health;}
	Bool getWhiner(void) {return m_whiner;}
	Bool getUnsellable(void) {return m_unsellable;}
	Bool getRepairable(void) {return m_repairable;}

	void setRenderObj(RenderObjClass *pObj) {m_renderObj = pObj;}
	RenderObjClass *getRenderObj(void) {return m_renderObj;}
	void setShadowObj(Shadow *pObj) {m_shadowObj = pObj;}
	Shadow *getShadowObj(void) {return m_shadowObj;}

	void setSelected(Bool sel) {m_selected = sel;}
	Bool isSelected(void) {return m_selected;}

	// used by the solo AI
	void setObjectID( ObjectID objID ) { m_objectID = objID; }
	ObjectID getObjectID( void ) const { return m_objectID; }
	void setObjectTimestamp( UnsignedInt frame ) { m_objectTimestamp = frame; }
	UnsignedInt getObjectTimestamp( void ) const { return m_objectTimestamp; }
	Bool isBuildable( void );															///< returns true if has enough rebuilds left to build again
	Bool isUnderConstruction(void) {return m_underConstruction;}
	void setUnderConstruction(Bool construction) { m_underConstruction=construction;}
	void markPriorityBuild(void) {m_priorityBuild = true; }
	Bool isPriorityBuild(void) {return m_priorityBuild;}
	Bool isAutomaticBuild(void) {return m_automaticallyBuild;}

	Bool isSupplyBuilding(void) {return m_isSupplyBuilding;}
	void setSupplyBuilding(Bool isSupply) {m_isSupplyBuilding = isSupply;}
	ObjectID getGathererID(Int ndx) {if (ndx>=0 && ndx < MAX_RESOURCE_GATHERERS) return m_resourceGatherers[ndx]; return INVALID_ID;}
	void setGathererID(Int ndx, ObjectID id)  {if (ndx>=0 && ndx < MAX_RESOURCE_GATHERERS) m_resourceGatherers[ndx] = id;}
	Int getDesiredGatherers(void) {return m_desiredGatherers;};
	void setDesiredGatherers(Int desired) {m_desiredGatherers = desired;}
	Int getCurrentGatherers(void) {return m_currentGatherers;};
	void setCurrentGatherers(Int cur) {m_currentGatherers = cur;}

	BuildListInfo *duplicate(void);
};

inline void BuildListInfo::decrementNumRebuilds(void)
{
	if (m_numRebuilds > 0 && m_numRebuilds != UNLIMITED_REBUILDS)
		m_numRebuilds--;
}

inline void BuildListInfo::incrementNumRebuilds(void)
{
	if (m_numRebuilds != UNLIMITED_REBUILDS)
		m_numRebuilds++;
}

inline Bool BuildListInfo::isBuildable( void )
{
	if (getNumRebuilds() > 0 || getNumRebuilds() == BuildListInfo::UNLIMITED_REBUILDS)
		return true;

	return false;
}
