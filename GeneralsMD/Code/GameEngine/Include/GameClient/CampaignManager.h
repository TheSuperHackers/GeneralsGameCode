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

// FILE: CampaignManager.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Jul 2002
//
//	Filename: 	CampaignManager.h
//
//	author:		Chris Huybregts
//
//	purpose:
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/Snapshot.h"
#include "Common/AudioEventRTS.h"
//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class AsciiString;
class Image;

enum{ MAX_OBJECTIVE_LINES = 5	};
enum{ MAX_DISPLAYED_UNITS = 3 };
//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class Mission : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( Mission, "Mission" )
public:
	Mission( void );
	//~Mission( void );

public:
	AsciiString m_name;
	AsciiString m_mapName;
	AsciiString m_nextMission;
	AsciiString m_movieLabel;
	AsciiString m_missionObjectivesLabel[MAX_OBJECTIVE_LINES];
	AudioEventRTS m_briefingVoice;
	AsciiString m_locationNameLabel;
	AsciiString m_unitNames[MAX_DISPLAYED_UNITS];
	Int m_voiceLength;
	AsciiString m_generalName;
};

class Campaign : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( Campaign, "Campaign" )
public:
	Campaign( void );
	//~Campaign( void );

	Mission *newMission( AsciiString name );
	Mission *getNextMission( Mission *current);
	Mission *getMission( AsciiString missionName);
	AsciiString getFinalVictoryMovie( void );
	Bool isChallengeCampaign( void ) { return m_isChallengeCampaign; }

public:
	typedef std::list< Mission* > MissionList;			///< list of Shell Menu schemes
	typedef MissionList::iterator MissionListIt;

	AsciiString m_name;
	AsciiString m_firstMission;
	AsciiString m_campaignNameLabel;		///< campaign name label from string manager
	MissionList m_missions;
	AsciiString m_finalMovieName;
	Bool m_isChallengeCampaign;
	AsciiString m_playerFactionName;
};

class CampaignManager : public Snapshot
{
public:
	CampaignManager( void );
	~CampaignManager( void );

	// snapshot methods
	virtual void crc( Xfer *xfer ) { }
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	void init( void );
	Campaign *getCurrentCampaign( void );		///< Returns a point to the current Campaign
	Mission *getCurrentMission( void );			///< Returns a point to the current mission
	Mission *gotoNextMission( void );				///< Set the next mission as the current Mission, and returns a point to it
	void setCampaignAndMission( AsciiString campaign, AsciiString mission );		///< Sets the campaing and Mission we're on
	void setCampaign( AsciiString campaign );																		///< sets the campaign and set's it's first mission
	AsciiString getCurrentMap( void );			///< Get the map located in m_currentMission;
	enum { INVALID_MISSION_NUMBER = -1 };
	Int getCurrentMissionNumber( void );		///< get mission number for the currently loaded level if we are in a campaign

	const FieldParse *getFieldParse( void ) const { return m_campaignFieldParseTable; }								///< returns the parsing fields
	static const FieldParse m_campaignFieldParseTable[];																				///< the parse table
	static void parseMissionPart( INI* ini, void *instance, void *store, const void *userData );					///< Parse the Mission Part

	Campaign *newCampaign(AsciiString name);
	Bool isVictorious( void ) { return m_victorious; }
	void SetVictorious( Bool victory ) { m_victorious = victory;	}

	void setRankPoints( Int rankPoints ) { m_currentRankPoints = rankPoints; }
	Int getRankPoints() const {
		// All campaign missions, regular and generals' challenge now start each map at rank 0.
		// This is because they weren't designed with rank persistence in mind, and were primarily
		// tested in a debug context, with no notion of previous rank.
		// NOTE: the assumption is being made that this method will never be used for any purpose
		// other than setting initial starting rank on map load, as it is currently.  08/09/03
		return 0;
	}

	GameDifficulty getGameDifficulty() const { return m_difficulty; }
	void setGameDifficulty(GameDifficulty d) { m_difficulty = d; }

private:
	typedef std::list< Campaign* > CampaignList;			///< list of Shell Menu schemes
	typedef CampaignList::iterator CampaignListIt;
	CampaignList m_campaignList;											///< Our List of Campaigns
	Campaign *m_currentCampaign;											///< Our current Campaign
	Mission *m_currentMission;												///< our Current Mission
	Bool m_victorious;
	Int m_currentRankPoints;
	GameDifficulty m_difficulty;
	Int m_xferChallengeGeneralsPlayerTemplateNum;			///< Need a place to stick this naughty singleton's important bit.

};
//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
extern CampaignManager *TheCampaignManager;
