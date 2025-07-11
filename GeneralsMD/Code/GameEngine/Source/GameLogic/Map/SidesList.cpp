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

// FILE: SidesList.cpp /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: SidesList.cpp
//
// Created:   John Ahlquist, Nov 2001
//
// Desc:      Contains the information describing Sides (player, ai, neutral etc.)
//						in a scenario, including build lists for non-player sides.
//
//-----------------------------------------------------------------------------

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/DataChunk.h"
#include "Common/GameState.h"
#include "Common/PlayerTemplate.h"
#include "Common/WellKnownKeys.h"
#include "Common/Xfer.h"
#include "GameLogic/AI.h"
#include "GameLogic/Scripts.h"
#include "GameLogic/SidesList.h"

static const Int K_SIDES_DATA_VERSION_1 = 1;
static const Int K_SIDES_DATA_VERSION_2 = 2;	// includes Team list.
static const Int K_SIDES_DATA_VERSION_3 = 3;	// includes Team list.

/* ********* SidesInfo class ****************************/
/**
 SidesInfo - Constructor.
*/
SidesInfo::SidesInfo(void) :
	m_pBuildList(NULL),
	m_scripts(NULL)
{
}

SidesInfo::SidesInfo(const SidesInfo& thatref) :
	m_pBuildList(NULL),
	m_scripts(NULL)
{
	*this = thatref;
}

/**
 SidesInfo - Destructor - 
*/
SidesInfo::~SidesInfo(void)
{
	clear();
}

void SidesInfo::init(const Dict* d)
{
	deleteInstance(m_pBuildList);
	m_pBuildList = NULL;
	m_dict.clear();
	if (m_scripts) 
		deleteInstance(m_scripts);
	m_scripts = NULL;
	if (d)
		m_dict = *d;
}

// ug, I hate having to overload stuff, but this makes it a lot easier to make copies safely
SidesInfo& SidesInfo::operator=(const SidesInfo& that)
{
	if (this != &that)
	{
		this->clear();
		this->m_dict = that.m_dict;

		BuildListInfo* thisBLTail = NULL;
		for (BuildListInfo* thatBL = that.m_pBuildList; thatBL; thatBL = thatBL->getNext())
		{
			BuildListInfo* thisBL = newInstance( BuildListInfo );	
			*thisBL = *thatBL;
			thisBL->setNextBuildList(NULL);

			if (thisBLTail)
				thisBLTail->setNextBuildList(thisBL);
			else
				this->m_pBuildList = thisBL;
				
			thisBLTail = thisBL;
		}

		if (that.m_scripts)
			this->m_scripts = that.m_scripts->duplicate();
		else
			this->m_scripts = NULL;
	}
	return *this;
}

/**
* SidesInfo::addToBuildList - Adds a build list entry as the nth entry.
*		
*/
void SidesInfo::addToBuildList(BuildListInfo *pBuildList, Int position)
{
	DEBUG_ASSERTLOG(pBuildList->getNext()==NULL, ("WARNING***Adding already linked element."));
	BuildListInfo *pCur = NULL;
	while (position) {
		position--;
		if (pCur==NULL) {
			pCur = m_pBuildList;
		} else {
			if (pCur->getNext()) {
				pCur = pCur->getNext();
			}	else {
				break; // at end of list.
			}
		}
	}
	if (pCur==NULL) {
		// add to front of list.
		pBuildList->setNextBuildList(m_pBuildList);
		m_pBuildList = pBuildList;
	}	else {
		pBuildList->setNextBuildList(pCur->getNext());
		pCur->setNextBuildList(pBuildList);
	}
}

/**
* SidesInfo::reorderInBuildList - Reorders a build list entry as the nth entry.
*		
*/
void SidesInfo::reorderInBuildList(BuildListInfo *pBuildList, Int newPosition)
{
	/*Int oldPos =*/ removeFromBuildList(pBuildList);
	addToBuildList(pBuildList, newPosition);
}

/**
* SidesInfo::removeFromBuildList - Removes a build list entry.
* Returns the position in the list that the item occupied.
*		
*/
Int SidesInfo::removeFromBuildList(BuildListInfo *pBuildList)
{
	DEBUG_ASSERTCRASH(pBuildList, ("Removing NULL list."));
	if (pBuildList==NULL) return 0;

	Int position = 0;

	if (pBuildList == m_pBuildList) {
		// First item in list, so update head.
		m_pBuildList = pBuildList->getNext();
	} else {
		position = 1;
		// Not the first item, so find the preceeding list element.
		BuildListInfo *pPrev = m_pBuildList;
		while (pPrev && (pPrev->getNext()!=pBuildList) ) {
			pPrev = pPrev->getNext();
			position++;
		}
		DEBUG_ASSERTCRASH(pPrev, ("Removing item not in list."));
		if (pPrev) {
			pPrev->setNextBuildList(pBuildList->getNext());
		}
	}
	pBuildList->setNextBuildList(NULL);
	return position;
}

/* ********* SidesList class ****************************/
/*extern*/ SidesList *TheSidesList = NULL;	 ///< singleton instance of SidesList
/**
 SidesList - Constructor.
*/
SidesList::SidesList(void) : m_numSides(0), m_numSkirmishSides(0)
{
}

/**
 SidesList - Destructor - 
*/
SidesList::~SidesList(void)
{
}

/**
 SidesList - reset - 
*/
void SidesList::reset(void)
{
	clear();
}

/**
 SidesList - clear - 
*/
void SidesList::clear(void)
{
	emptySides();
	emptyTeams();
}



/**
* SidesList::ParseSidesDataChunk - read a Sides chunk.
* Format is the newer CHUNKY format.
*	See SidesList::WriteSidesDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Bool SidesList::ParseSidesDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	DEBUG_ASSERTCRASH(TheSidesList, ("TheSidesList is null"));

	if (TheSidesList==NULL) 
		return false;

	TheSidesList->clear();
	Int count = file.readInt();
	Int i, j;
	TheSidesList->emptySides();
	for (i=0; i<count; i++) 
	{
		if (i >= MAX_PLAYER_COUNT) break;
		Dict d =  file.readDict();
		TheSidesList->addSide(&d);
		BuildListInfo* pBuildList;
		Int count = file.readInt();
		for (j=0; j<count; j++) 
		{
			pBuildList = newInstance( BuildListInfo );
			pBuildList->setBuildingName(file.readAsciiString());
			pBuildList->setTemplateName(file.readAsciiString());
			Coord3D loc;
			loc.x = file.readReal();
			loc.y = file.readReal();
			loc.z = file.readReal();
			loc.z = 0; // force to ground level
			pBuildList->setLocation(loc);
			pBuildList->setAngle(file.readReal());
			pBuildList->setInitiallyBuilt(file.readByte());
			pBuildList->setNumRebuilds(file.readInt());
			if (info->version >= K_SIDES_DATA_VERSION_3)
			{
				pBuildList->setScript(file.readAsciiString());
				pBuildList->setHealth(file.readInt());
				pBuildList->setWhiner(file.readByte());
				pBuildList->setUnsellable(file.readByte());
				pBuildList->setRepairable(file.readByte());
			}
			TheSidesList->getSideInfo(i)->addToBuildList(pBuildList, j);
		}
	}	
	if (info->version >= K_SIDES_DATA_VERSION_2)
	{
		count = file.readInt();
		TheSidesList->emptyTeams();
		for (i=0; i<count; i++) 
		{
			Dict d =  file.readDict();
			TheSidesList->addTeam(&d);
		}
	}
	
	file.registerParser( AsciiString("PlayerScriptsList"), info->label, ScriptList::ParseScriptsDataChunk );
	if (!file.parse(NULL)) {
		throw(ERROR_CORRUPT_FILE_FORMAT);
	}
	ScriptList *scripts[MAX_PLAYER_COUNT];
	count = ScriptList::getReadScripts(scripts);
	for (i=0; i<count; i++) {
		if (i<TheSidesList->getNumSides()) {
			ScriptList *pSL = TheSidesList->getSideInfo(i)->getScriptList();
			deleteInstance(pSL);
			TheSidesList->getSideInfo(i)->setScriptList(scripts[i]);
			scripts[i] = NULL;
		} else {
			// Read in more players worth than we have.
			deleteInstance(scripts[i]);
			scripts[i] = NULL;
		}
	}
	TheSidesList->validateSides();

	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Incorrect data file length."));
	return true;
}


/**
* SidesList::WriteSidesDataChunk - Writes a Sides chunk.
* Format is the newer CHUNKY format.
*	See SidesList::ParseSidesDataChunk for the reader.
*	Input: DataChunkInput 
*		
*/
void SidesList::WriteSidesDataChunk(DataChunkOutput &chunkWriter)
{
	DEBUG_ASSERTCRASH(TheSidesList, ("TheSidesList is null"));
	if (TheSidesList==NULL) 
		return;
	/**********HEIGHT MAP DATA ***********************/
	chunkWriter.openDataChunk("SidesList", K_SIDES_DATA_VERSION_3);	
	
		chunkWriter.writeInt(TheSidesList->getNumSides());
		Int i;
		for (i=0; i<TheSidesList->getNumSides(); i++) {
			chunkWriter.writeDict(*TheSidesList->getSideInfo(i)->getDict());	
			BuildListInfo* pBuildList = TheSidesList->getSideInfo(i)->getBuildList();
			Int count = 0;
			while (pBuildList) {
				count++;
				pBuildList = pBuildList->getNext();
			}
			chunkWriter.writeInt(count);
			pBuildList = TheSidesList->getSideInfo(i)->getBuildList();
			while (pBuildList) {
				chunkWriter.writeAsciiString(pBuildList->getBuildingName());
				chunkWriter.writeAsciiString(pBuildList->getTemplateName());
				chunkWriter.writeReal(pBuildList->getLocation()->x);
				chunkWriter.writeReal(pBuildList->getLocation()->y);
				chunkWriter.writeReal(pBuildList->getLocation()->z);
				chunkWriter.writeReal(pBuildList->getAngle());
				chunkWriter.writeByte(pBuildList->isInitiallyBuilt());
				chunkWriter.writeInt(pBuildList->getNumRebuilds());
				// BEGIN stuff new to K_SIDES_DATA_VERSION_3
				chunkWriter.writeAsciiString(pBuildList->getScript());
				chunkWriter.writeInt(pBuildList->getHealth());
				chunkWriter.writeByte(pBuildList->getWhiner());
				chunkWriter.writeByte(pBuildList->getUnsellable());
				chunkWriter.writeByte(pBuildList->getRepairable());
				// END stuff new to K_SIDES_DATA_VERSION_3

				pBuildList = pBuildList->getNext();
			}
		}	
		
		// BEGIN stuff new to K_SIDES_DATA_VERSION_2
		chunkWriter.writeInt(TheSidesList->getNumTeams());
		for (i=0; i<TheSidesList->getNumTeams(); i++) {
			chunkWriter.writeDict(*TheSidesList->getTeamInfo(i)->getDict());	
		}
		// END stuff new to K_SIDES_DATA_VERSION_2

		ScriptList *scripts[MAX_PLAYER_COUNT];
		for (i=0; i<TheSidesList->getNumSides(); i++) {
			scripts[i] = TheSidesList->getSideInfo(i)->getScriptList();
		}
		ScriptList::WriteScriptsDataChunk(chunkWriter, scripts, TheSidesList->getNumSides());
	chunkWriter.closeDataChunk();

	Bool modified = TheSidesList->validateSides();
	DEBUG_ASSERTLOG(!modified, ("*** had to clean up sideslist on read"));
	modified = false;	// silence compiler warnings in release build
	
}

TeamsInfo *SidesList::findTeamInfo(AsciiString name, Int* index /*= NULL*/)
{
	return m_teamrec.findTeamInfo(name, index);
}

SidesInfo *SidesList::findSideInfo(AsciiString name, Int* index /*= NULL*/)
{
	for (int i = 0; i < m_numSides; i++) 
	{
		if (m_sides[i].getDict()->getAsciiString(TheKey_playerName) == name)
		{
			if (index)
				*index = i;
			return &m_sides[i];
		}
	}
	return NULL;
}

SidesInfo *SidesList::findSkirmishSideInfo(AsciiString name, Int* index /*= NULL*/)
{
	for (int i = 0; i < m_numSkirmishSides; i++) 
	{
		if (m_skirmishSides[i].getDict()->getAsciiString(TheKey_playerName) == name)
		{
			if (index)
				*index = i;
			return &m_skirmishSides[i];
		}
	}
	return NULL;
}

static AsciiString static_readPlayerNames[MAX_PLAYER_COUNT];

/**
* ParsePlayersDataChunk - read players names data chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
#define K_PLAYERS_NAMES_FOR_SCRIPTS_VERSION_1 1
#define K_PLAYERS_NAMES_FOR_SCRIPTS_VERSION_2 2

static Bool ParsePlayersDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	Int readDicts = 0;
	if (info->version >= K_PLAYERS_NAMES_FOR_SCRIPTS_VERSION_2) {
		readDicts = file.readInt();
	}
	Int numNames = file.readInt();
	Int i;
	for (i=0; i<numNames; i++) {
		if (i>=MAX_PLAYER_COUNT) break;
		static_readPlayerNames[i] = file.readAsciiString();
		if (readDicts) {
			Dict sideDict = file.readDict();
		}
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));
	return true;
}

/**
* ParseTeamsDataChunk - read teams data chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
static Bool ParseTeamsDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	SidesList *sides = (SidesList *)userData;
	while (!file.atEndOfChunk()) {
		Dict teamDict = file.readDict();
		AsciiString teamName = teamDict.getAsciiString(TheKey_teamName);
		AsciiString player = teamDict.getAsciiString(TheKey_teamOwner);
		if (sides->findSkirmishSideInfo(player)) {
			// player exists, so just add it.
			sides->addSkirmishTeam(&teamDict);
			//DEBUG_LOG(("Adding team %s\n", teamName.str()));
		} else {
			//DEBUG_LOG(("Couldn't add team %s, no player %s\n", teamName.str(), player.str()));
		}
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));
	return true;
}

void SidesList::prepareForMP_or_Skirmish(void)
{
	m_skirmishTeamrec.clear();
	Int i;
	for (i = 0; i < getNumTeams(); i++)
	{
		Dict *tdict = getTeamInfo(i)->getDict();
		m_skirmishTeamrec.addTeam(tdict);
	}
	m_teamrec.clear();

	for (i = 0; i < MAX_PLAYER_COUNT; i++) {
		m_skirmishSides[i].clear();
	}
	m_numSkirmishSides = 0;

	for (i = 0; i < m_numSides; i++) 
	{
		m_skirmishSides[m_numSkirmishSides] = m_sides[i];
		m_numSkirmishSides++;
		if (m_sides[i].getDict()->getAsciiString(TheKey_playerFaction) == "FactionCivilian") {
			// Don't remove FactionCivilian.
			continue;
		}
		if (m_numSides == 1) break;	// can't remove the last side.
		removeSide(i);
		i--;
	}
	Bool gotScripts = false;
	for (i=0; i<m_numSkirmishSides; i++) {
		if (m_skirmishSides[i].getDict()->getAsciiString(TheKey_playerFaction) == "FactionCivilian") {
			// Don't consider FactionCivilian.
			continue;
		}
		if (m_skirmishSides[i].getScriptList()==NULL) continue;
		if (m_skirmishSides[i].getScriptList()->getScript() != NULL || 
			m_skirmishSides[i].getScriptList()->getScriptGroup()!=NULL) {
			gotScripts = true;
		}
	}
	if (!gotScripts) {
		AsciiString path = "data\\Scripts\\SkirmishScripts.scb";
		DEBUG_LOG(("Skirmish map using standard scripts\n"));
		m_skirmishTeamrec.clear();
		CachedFileInputStream theInputStream;
		if (theInputStream.open(path)) { 
				ChunkInputStream *pStrm = &theInputStream;
				DataChunkInput file( pStrm );
				file.registerParser( AsciiString("PlayerScriptsList"), AsciiString::TheEmptyString, ScriptList::ParseScriptsDataChunk );
				file.registerParser( AsciiString("ScriptsPlayers"), AsciiString::TheEmptyString, ParsePlayersDataChunk );
				file.registerParser( AsciiString("ScriptTeams"), AsciiString::TheEmptyString, ParseTeamsDataChunk );
				if (!file.parse(this)) {
					DEBUG_LOG(("ERROR - Unable to read in skirmish scripts.\n"));
					return;
				}
				ScriptList *scripts[MAX_PLAYER_COUNT];
				Int count = ScriptList::getReadScripts(scripts);
				Int i;
				for (i=0; i<count; i++) {
					Int curSide = -1;
					Int j;
					for (j=0; j<m_numSkirmishSides; j++) {
 						AsciiString name = getSkirmishSideInfo(j)->getDict()->getAsciiString(TheKey_playerName);
						if (name == static_readPlayerNames[i]) {
							curSide = j;
							break;
						}
					}
					if (curSide == -1) continue;

					ScriptList *pSL = getSkirmishSideInfo(curSide)->getScriptList();
					getSkirmishSideInfo(curSide)->setScriptList(scripts[i]);
					scripts[i] = NULL;
					if (pSL) 
						deleteInstance(pSL);
					scripts[i] = NULL;
				}
				for (i=0; i<MAX_PLAYER_COUNT; i++) {
					static_readPlayerNames[i].clear();
				}
		}


	}
}


Bool SidesList::isPlayerDefaultTeam(TeamsInfo *t)
{
	// if our name is "teamfoo" and there is a player named "foo", we are a player-default team.
	AsciiString tname = t->getDict()->getAsciiString(TheKey_teamName);
	if (tname.startsWith("team"))
	{
		const char* rest = tname.str() + 4;
		for (int j = 0; j < m_numSides; j++)
		{
			AsciiString pname = m_sides[j].getDict()->getAsciiString(TheKey_playerName);
			if (strcmp(pname.str(), rest) == 0)
			{
				return true;
			}
		}
	}
	return false;
}

void SidesList::emptySides() 
{ 
	Int i;

	m_numSides = 0; 
	m_numSkirmishSides = 0; 
	for (i = 0; i < MAX_PLAYER_COUNT; i++) {
		m_sides[i].clear(); 
		m_skirmishSides[i].clear();
	}
}

void SidesList::emptyTeams() 
{ 
	m_teamrec.clear();
	m_skirmishTeamrec.clear();
}

void SidesList::addSide(const Dict* d)
{
	DEBUG_ASSERTCRASH(m_numSides < MAX_PLAYER_COUNT, ("too many players"));
	if (m_numSides < MAX_PLAYER_COUNT)
		m_sides[m_numSides++].init(d);
}

void SidesList::addTeam(const Dict* d)
{
	m_teamrec.addTeam(d);
}

void SidesList::addSkirmishTeam(const Dict* d)
{
	m_skirmishTeamrec.addTeam(d);
}

void SidesList::removeSide(Int i)
{
	if (i < 0 || i >= m_numSides || m_numSides <= 1)
		return;

	for ( ; i < m_numSides-1; i++)
		m_sides[i] = m_sides[i+1];

	for ( ; i < MAX_PLAYER_COUNT; i++)
		m_sides[i].clear();

	--m_numSides;
}

void SidesList::removeTeam(Int i)
{
	m_teamrec.removeTeam(i);
}

Bool SidesList::validateAllyEnemyList(const AsciiString& tname, AsciiString& allies)
{
		// owners/allies/enemies must be players.

	Bool modified = false;

	AsciiString str, newstr, token;
	
	str = allies;
	newstr.clear();
	while (str.nextToken(&token))
	{
		if (token == tname)
		{
			modified = true;
			continue;	// no allies/enemies with self
		}

		SidesInfo *si = findSideInfo(token);
		if (!si)
		{
			modified = true;
			continue;	// player not found.
		}

		if (!newstr.isEmpty())
			newstr.concat(" ");
		newstr.concat(token);
	}

	allies = newstr;
	return modified;
}

void SidesList::addPlayerByTemplate(AsciiString playerTemplateName)
{
	AsciiString playerName;
	UnicodeString playerDisplayName;
	Bool isHuman = false;

	if (playerTemplateName.isEmpty())
	{
		playerName.set("");	// magic code for "neutral"
		playerDisplayName = L"Neutral";
		isHuman = false;
	}
	else
	{
		playerName.set("Plyr");
		if (playerTemplateName.startsWith("Faction"))
		{
			playerName.concat(playerTemplateName.str() + 7);
		}
		else
		{
			playerName.concat(playerTemplateName);
		}
		playerDisplayName.translate(playerName);
		isHuman = true;
		// special-case "civilian"...
		if (playerName == "PlyrCivilian")
			isHuman = false;
	}

	Dict d;

	d.clear();
	d.setAsciiString(TheKey_playerName, playerName);	
	d.setBool(TheKey_playerIsHuman, isHuman);
	d.setUnicodeString(TheKey_playerDisplayName, playerDisplayName);
	d.setAsciiString(TheKey_playerFaction, playerTemplateName);
	d.setAsciiString(TheKey_playerAllies, AsciiString::TheEmptyString);
	d.setAsciiString(TheKey_playerEnemies, AsciiString::TheEmptyString);

	addSide(&d);

	AsciiString playerTeamName;
	playerTeamName.set("team");
	playerTeamName.concat(playerName);

	d.clear();
	d.setAsciiString(TheKey_teamName, playerTeamName);
	d.setAsciiString(TheKey_teamOwner, playerName);
	d.setBool(TheKey_teamIsSingleton, true);
	addTeam(&d);
}

Bool SidesList::validateSides()
{
	Bool modified = false;

	// ensure we have at least one player, and at least one neutral player.
	Int i;
	Int neutral = -1;
	Int num = getNumSides();
	for (i = 0; i < num; i++)
	{
		if (getSideInfo(i)->getDict()->getAsciiString(TheKey_playerName).isEmpty())
		{
			neutral = i;
			break;
		}
	}
	if (neutral == -1)
	{
		addPlayerByTemplate(AsciiString::TheEmptyString);
		modified = true;
	}

	// now ensure that every player has a proper "default team"
	for (i = 0; i < getNumSides(); i++) 
	{
		Dict *pdict = getSideInfo(i)->getDict();
		AsciiString pname = pdict->getAsciiString(TheKey_playerName);
		AsciiString tname("team");
		tname.concat(pname);
		TeamsInfo *ti = findTeamInfo(tname);
		if (ti)
		{
			// make sure the team owner points back to the player.
			if (ti->getDict()->getAsciiString(TheKey_teamOwner) != pname)
			{
				DEBUG_CRASH(("hmm, team owner mismatch (%s) (%s), this should not normally be possible",ti->getDict()->getAsciiString(TheKey_teamOwner).str(), pname.str()));
				ti->getDict()->setAsciiString(TheKey_teamOwner, pname);
				modified = true;
			}
			// default teams are always singletons.
			if (!ti->getDict()->getBool(TheKey_teamIsSingleton))
			{
				DEBUG_CRASH(("hmm, this should not normally be possible"));
				ti->getDict()->setBool(TheKey_teamIsSingleton, true);
				modified = true;
			}
		}
		else
		{
			DEBUG_LOG(("*** default team for player %s missing (should not be possible), adding it...\n",tname.str()));
			Dict d;
			d.setAsciiString(TheKey_teamName, tname);
			d.setAsciiString(TheKey_teamOwner, pname);
			d.setBool(TheKey_teamIsSingleton, true);
			addTeam(&d);
			modified = true;
		}

		AsciiString allies = pdict->getAsciiString(TheKey_playerAllies);
		AsciiString enemies = pdict->getAsciiString(TheKey_playerEnemies);

		// ensure all teams have valid allies & enemies.
		// (note that owners can be teams or players, but allies/enemies can only be teams.)
		if (validateAllyEnemyList(pname, allies))
		{
			DEBUG_LOG(("bad allies...\n"));
			pdict->setAsciiString(TheKey_playerAllies, allies);
			modified = true;
		}

		if (validateAllyEnemyList(pname, enemies))
		{
			DEBUG_LOG(("bad enemies...\n"));
			pdict->setAsciiString(TheKey_playerEnemies, enemies);
			modified = true;
		}
	}

	// ensure there's no overlap between team names and player names.
	// (if there is, the player wins and the team is whacked.)
validate_team_names:
	for (i = 0; i < getNumTeams(); i++)
	{
		Dict *tdict = getTeamInfo(i)->getDict();
		AsciiString tname = tdict->getAsciiString(TheKey_teamName);
		if (findSideInfo(tname))
		{
			DEBUG_CRASH(("name %s is duplicate between player and team, removing...\n",tname.str()));
			removeTeam(i);
			modified = true;
			goto validate_team_names;
		}
	}

	for (i = 0; i < getNumTeams(); i++)
	{
		Dict *tdict = getTeamInfo(i)->getDict();
		AsciiString tname = tdict->getAsciiString(TheKey_teamName);
		AsciiString towner = tdict->getAsciiString(TheKey_teamOwner);
		SidesInfo* si = findSideInfo(towner);
		if (si == NULL || towner == tname)
		{
			DEBUG_LOG(("bad owner %s; reparenting to neutral...\n",towner.str()));
			tdict->setAsciiString(TheKey_teamOwner, AsciiString::TheEmptyString);
			modified = true;
		}
//		if (tdict->getType(NAMEKEY(AsciiString("teamAllies"))) != Dict::DICT_NONE)
//			tdict->remove(NAMEKEY(AsciiString("teamAllies")));
//		if (tdict->getType(NAMEKEY(AsciiString("teamEnemies"))) != Dict::DICT_NONE)
//			tdict->remove(NAMEKEY(AsciiString("teamEnemies")));

	}

	return modified;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SidesList::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method 
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SidesList::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// xfer num sides
	Int sideCount = getNumSides();
	xfer->xferInt( &sideCount );
	if( sideCount != getNumSides() )
	{

		DEBUG_CRASH(( "SidesList::xfer - The sides list size has changed, this was not supposed to happen, you must version this method and figure out how to translate between old and new versions now\n" ));
		throw SC_INVALID_DATA;

	}  // end if

	// side data
	ScriptList *scriptList;
	Bool scriptListPresent;
	for( Int i = 0; i < sideCount; ++i )
	{

		// xfer script list data that can change
		scriptList = getSideInfo( i )->getScriptList();
		scriptListPresent = scriptList ? TRUE : FALSE;
		xfer->xferBool( &scriptListPresent );
		if( (scriptList == NULL && scriptListPresent == TRUE) ||
				(scriptList != NULL && scriptListPresent == FALSE) )
		{

			DEBUG_CRASH(( "SidesList::xfer - script list missing/present mismatch\n" ));
			throw SC_INVALID_DATA;

		}  // end if
		if( scriptListPresent )
			xfer->xferSnapshot( scriptList );

	}  // end for i

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SidesList::loadPostProcess( void )
{

}  // end loadPostProcess

/* ********* BuildListInfo class ****************************/
/**
 BuildListInfo - Constructor.
*/
BuildListInfo::BuildListInfo(void) :
m_nextBuildList(NULL),
m_renderObj(NULL),
m_shadowObj(NULL),
m_isInitiallyBuilt(false),
m_numRebuilds(0),
m_angle(0),
m_script(AsciiString::TheEmptyString),
m_health(100),
m_whiner(true),
m_unsellable(false),
m_repairable(true),
m_objectID(INVALID_ID),
m_objectTimestamp(0),
m_underConstruction(false),
m_isSupplyBuilding(false),
m_desiredGatherers(0),
m_currentGatherers(0),
m_automaticallyBuild(true),
m_priorityBuild(false),
m_buildingName(AsciiString::TheEmptyString)
{
	// Added by Sadullah Nader
	// these initialized values are necessary!!!
	m_location.zero();
	m_rallyPointOffset.x = 0.0f;
	m_rallyPointOffset.y = 0.0f;
	m_selected = FALSE;

	Int i;
	for (i=0; i<MAX_RESOURCE_GATHERERS; i++) 
	{
		m_resourceGatherers[i] = INVALID_ID;
	}
}


/**
 BuildListInfo - Destructor - note - if linked, deletes linked items.
*/
BuildListInfo::~BuildListInfo(void)
{
	if (m_nextBuildList) {
		BuildListInfo *cur = m_nextBuildList;
		BuildListInfo *next;
		while (cur) {
			next = cur->getNext();
			cur->setNextBuildList(NULL); // prevents recursion. 
			deleteInstance(cur);
			cur = next; 
		}
	}
}

void BuildListInfo::parseStructure(INI *ini, void *instance, void* /*store*/, const void* /*userData*/)
{
	const char* c = ini->getNextToken();
	AsciiString tTemplateName(c);

	static const FieldParse myFieldParse[] = 
		{
			{ "Name",				INI::parseAsciiString,		NULL, offsetof( BuildListInfo, m_buildingName	 ) },
			{ "Location",		INI::parseCoord2D,				NULL, offsetof( BuildListInfo, m_location ) },
      { "Rebuilds",		INI::parseInt,						NULL, offsetof( BuildListInfo, m_numRebuilds ) },
      { "Angle",			INI::parseAngleReal,			NULL, offsetof( BuildListInfo, m_angle ) },
      { "InitiallyBuilt",			INI::parseBool,		NULL, offsetof( BuildListInfo, m_isInitiallyBuilt ) },
      { "RallyPointOffset",			INI::parseCoord2D,		NULL, offsetof( BuildListInfo, m_rallyPointOffset ) },
      { "AutomaticallyBuild",			INI::parseBool,	NULL, offsetof( BuildListInfo, m_automaticallyBuild ) },
			{ NULL,							NULL,											NULL, 0 }  // keep this last
		};

	BuildListInfo *buildInfo = newInstance( BuildListInfo );
	buildInfo->setTemplateName(tTemplateName);
	ini->initFromINI(buildInfo, myFieldParse);
	((AISideBuildList*)instance)->addInfo(buildInfo);
}


/**
 BuildListInfo - Duplicate - note - if linked, duplicates linked items.
*/
BuildListInfo *BuildListInfo::duplicate(void)
{
	BuildListInfo *first = newInstance( BuildListInfo );
	*first = *this;
	first->m_nextBuildList = NULL;
	BuildListInfo *next = this->m_nextBuildList;
	BuildListInfo *cur = first;
	while (next) {
		BuildListInfo *link = newInstance( BuildListInfo );
		*link = *next;
		link->m_nextBuildList = NULL;
		cur->m_nextBuildList = link;
		cur = link;
		next = next->m_nextBuildList;
	}
	return first;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BuildListInfo::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void BuildListInfo::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	xfer->xferAsciiString( &m_buildingName );
	xfer->xferAsciiString( &m_templateName );
	xfer->xferCoord3D( &m_location );
	xfer->xferCoord2D( &m_rallyPointOffset );
	xfer->xferReal( &m_angle );
	xfer->xferBool( &m_isInitiallyBuilt );
	xfer->xferUnsignedInt( &m_numRebuilds );
	xfer->xferAsciiString( &m_script );
	xfer->xferInt( &m_health );
	xfer->xferBool( &m_whiner );
	xfer->xferBool( &m_unsellable );
	xfer->xferBool( &m_repairable );
	xfer->xferBool( &m_automaticallyBuild );
	// m_renderObj we don't need to xfer this, its for the editor only
	// m_shadowObj we don't need to xfer this, its for the editor only
	// m_selected we don't need to xfer this, its for the editor only
	xfer->xferObjectID( &m_objectID );
	xfer->xferUnsignedInt( &m_objectTimestamp );
	xfer->xferBool( &m_underConstruction );
	xfer->xferUser( m_resourceGatherers, sizeof( ObjectID ) * MAX_RESOURCE_GATHERERS );
	xfer->xferBool( &m_isSupplyBuilding );
	xfer->xferInt( &m_desiredGatherers );
	xfer->xferBool( &m_priorityBuild );
	if (version>=2) {
		xfer->xferInt(&m_currentGatherers);
	}

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BuildListInfo::loadPostProcess( void )
{

}  // end loadPostProcess

/* ********* TeamsInfoRec class ****************************/
TeamsInfoRec::TeamsInfoRec() : 
	m_numTeams(0), m_numTeamsAllocated(0), m_teams(NULL)
{
}

TeamsInfoRec::TeamsInfoRec(const TeamsInfoRec& thatref) : 
	m_numTeams(0), m_numTeamsAllocated(0), m_teams(NULL)
{
	*this = thatref;
}

TeamsInfoRec::~TeamsInfoRec() 
{
	clear();
}

// ug, I hate having to overload stuff, but this makes it a lot easier to make copies safely
TeamsInfoRec& TeamsInfoRec::operator=(const TeamsInfoRec& thatref)
{
	const TeamsInfoRec* that = &thatref;
	if (this != that)
	{
		this->clear();
		for (int i = 0; i < that->m_numTeams; i++) 
		{
			this->addTeam(that->m_teams[i].getDict());
		}
	}
	return *this;
}

void TeamsInfoRec::clear() 
{ 
	Int i;

	for (i = 0; i < m_numTeams; ++i) 
		m_teams[i].clear(); 

	m_numTeams = 0; 
	m_numTeamsAllocated = 0;
	delete [] m_teams;
	m_teams = NULL;
}

TeamsInfo *TeamsInfoRec::findTeamInfo(AsciiString name, Int* index /*= NULL*/)
{
	for (int i = 0; i < m_numTeams; ++i) 
	{
		if (m_teams[i].getDict()->getAsciiString(TheKey_teamName) == name)
		{
			if (index)
				*index = i;
			return &m_teams[i];
		}
	}
	return NULL;
}

void TeamsInfoRec::addTeam(const Dict* d)
{
	enum
	{
		TEAM_ALLOC_CHUNK = 8	///< how many teams to alloc at a time
	};

	DEBUG_ASSERTCRASH(m_numTeams < 2048, ("%d teams have been allocated (so far). This seems excessive.", m_numTeams ));
	if (m_numTeams >= m_numTeamsAllocated)
	{
		// pool[]ify
		const Int newNumTeamsAllocated = m_numTeams + TEAM_ALLOC_CHUNK;
		TeamsInfo* nti = NEW TeamsInfo[newNumTeamsAllocated];
		Int i;

		for (i = 0; i < m_numTeams; ++i)
			nti[i] = m_teams[i];

		delete [] m_teams;
		m_teams = nti;
		m_numTeamsAllocated = newNumTeamsAllocated;
	}

	m_teams[m_numTeams].init(d);

	++m_numTeams;
}

void TeamsInfoRec::removeTeam(Int i)
{
	if (i < 0 || i >= m_numTeams || m_numTeams <= 1)
		return;

	--m_numTeams;

	for ( ; i < m_numTeams; ++i)
		m_teams[i] = m_teams[i+1];

	m_teams[m_numTeams].clear();
}
