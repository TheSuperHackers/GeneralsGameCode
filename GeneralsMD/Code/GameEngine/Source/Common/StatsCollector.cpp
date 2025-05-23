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

// FILE: StatsCollector.cpp /////////////////////////////////////////////////
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
//	Filename: 	StatsCollector.cpp
//
//	author:		Chris Huybregts
//	
//	purpose:	Convinience class to gather player stats
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/StatsCollector.h"
#include "Common/FileSystem.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "Common/GlobalData.h"
#include "Common/Money.h"
#include "GameLogic/Object.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/MapUtil.h"
#include "GameNetwork/networkutil.h"
#include "GameNetwork/LANAPICallbacks.h"
//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
StatsCollector *TheStatsCollector = NULL;

static char statsDir[255] = "Stats\\";
//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

// init all
//=============================================================================
StatsCollector::StatsCollector( void )
{
	//Added By Sadullah Nader
	//Initialization(s) inserted
	m_isScrolling = FALSE;
	m_scrollBeginTime = 0;
	m_scrollTime = 0;

	//
	m_timeCount = 0;
	m_buildCommands = 0;
	m_moveCommands = 0;
	m_attackCommands = 0;
	m_scrollMapCommands = 0;
	m_AIUnits = 0;
	m_playerUnits = 0;
	
	m_lastUpdate = 0;
	m_startFrame = TheGameLogic->getFrame();

}
//Destructor
//=============================================================================
StatsCollector::~StatsCollector( void )
{
	
}

// Reset and create the file header
//=============================================================================
void StatsCollector::reset( void )
{

	// make sure we have a stats Dir.
#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
	if (TheGlobalData->m_saveStats)
	{
		AsciiString playtestDir = TheGlobalData->m_baseStatsDir;
		playtestDir.concat(statsDir);
		if (TheNetwork)
		{
			if (TheLAN)
			{
				TheFileSystem->createDirectory(playtestDir);
			}
		}
	}
#endif
	TheFileSystem->createDirectory(AsciiString(statsDir));
	createFileName();
	writeInitialFileInfo();
	
	// zero out
	zeroOutStats();

	m_lastUpdate = TheGameLogic->getFrame(); // timeGetTime();
}	

// Msgs pass through here so we can track whichever ones we want
//=============================================================================
void StatsCollector::collectMsgStats( const GameMessage *msg )
{
	// We only care about our own messages.
	if(ThePlayerList->getLocalPlayer()->getPlayerIndex() != msg->getPlayerIndex())
		return;
	
	switch (msg->getType()) 
	{
		case GameMessage::MSG_QUEUE_UNIT_CREATE:
		case GameMessage::MSG_DOZER_CONSTRUCT:
		case GameMessage::MSG_DOZER_CONSTRUCT_LINE:
			{
				++m_buildCommands;
				break;
			}
	}

}

//Loop through all objects and count up the ones we want. (Very Slow!!!)
//=============================================================================
void StatsCollector::collectUnitCountStats( void )
{
	
	for(Object *obj =	TheGameLogic->getFirstObject(); obj; obj = obj->getNextObject())
	{
		
		if((!(obj->isKindOf(KINDOF_INFANTRY) || obj->isKindOf(KINDOF_VEHICLE))) || ( obj->isNeutralControlled()) ||(obj->getControllingPlayer()->getSide().compare("Civilian") == 0))
			continue;
		
		if(obj->getControllingPlayer()->isLocalPlayer())
		{
			++m_playerUnits;
		}
		else 
		{
			++m_AIUnits;
		}
	}

}

// call every frame and only do stuff when our time is up
//=============================================================================
void StatsCollector::update( void )
{
	if(m_lastUpdate + (TheGlobalData->m_playStats * LOGICFRAMES_PER_SECOND) > TheGameLogic->getFrame())
		return;

	collectUnitCountStats();

	if(m_isScrolling)
	{
		m_scrollTime += TheGameLogic->getFrame() - m_scrollBeginTime;
		m_scrollBeginTime = TheGameLogic->getFrame();
	}

	m_timeCount += TheGlobalData->m_playStats;
	writeStatInfo();

	zeroOutStats();

	m_lastUpdate = TheGameLogic->getFrame(); //timeGetTime();
	
}

void StatsCollector::incrementScrollMoveCount( void )
{
	++m_scrollMapCommands;
}

void StatsCollector::incrementAttackCount( void )
{
	++m_attackCommands;
}

void StatsCollector::incrementBuildCount( void )
{
	++m_buildCommands;
}
void StatsCollector::incrementMoveCount( void )
{
	++m_moveCommands;
}

void StatsCollector::writeFileEnd( void )
{
	//open the file
	FILE *f = fopen(m_statsFileName.str(), "a");
	if(!f)
	{
		DEBUG_ASSERTCRASH(f, ("Unable to open file %s to write", m_statsFileName.str()));
		return;
	}
	
	m_timeCount += (TheGameLogic->getFrame() - m_lastUpdate) / LOGICFRAMES_PER_SECOND;
	writeStatInfo();
	fprintf(f, "---------------------------------------------------\n");
	
		// Time
	struct tm *newTime;
	time_t aclock;
  time( &aclock );
  newTime = localtime( &aclock ); 
	fprintf(f, "End Time:\t%s\n",asctime(newTime) );

	fprintf(f, "=KEY===============================================\n");
	fprintf(f, "Time* = The Time Interval\n");
	fprintf(f, "BC = Build Commands\n");
	fprintf(f, "MC = Move Commands\n");
	fprintf(f, "AC = Attack Commands\n");
	fprintf(f, "SMC = Scroll Map Commands\n");
	fprintf(f, "ST* = Scroll Time in Seconds\n");
	fprintf(f, "OC = Other Commands (N/A)\n");
	fprintf(f, "$$$ = Local Player's Cash Amount\n");
	fprintf(f, "#PU = # of Player's Units\n");
	fprintf(f, "#AIU = # of AI's Units\n");
	fprintf(f, "===================================================\n");
	fprintf(f, "* Times are in Game Seconds which are based off of frames. Current fps is set to %d\n", LOGICFRAMES_PER_SECOND);
	
#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
	if (TheGlobalData->m_benchmarkTimer > 0)
	{
		fprintf(f, "\n*** BENCHMARK MODE STATS ***\n");
		fprintf(f, " Frames = %d\n", TheGameLogic->getFrame()-m_startFrame);
		fprintf(f, "Seconds = %d\n", TheGlobalData->m_benchmarkTimer);
		fprintf(f, "    FPS = %.2f\n", ((Real)TheGameLogic->getFrame()-(Real)m_startFrame)/(Real)TheGlobalData->m_benchmarkTimer);
	}
#endif

	fclose(f);

}

void StatsCollector::startScrollTime( void )
{
	m_isScrolling = TRUE;
	m_scrollBeginTime = TheGameLogic->getFrame();
	++m_scrollMapCommands;
}

void StatsCollector::endScrollTime( void )
{
	if(!m_isScrolling)
		return;
	
	m_isScrolling = FALSE;

	m_scrollTime += TheGameLogic->getFrame() - m_scrollBeginTime;
}

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

void StatsCollector::zeroOutStats( void )
{
	m_buildCommands = 0;
	m_moveCommands = 0;
	m_attackCommands = 0;
	m_scrollMapCommands = 0;
	m_AIUnits = 0;
	m_playerUnits = 0;
	m_scrollTime = 0;
}

// create the filename based off of map time and date
//=============================================================================
void StatsCollector::createFileName( void )
{
	m_statsFileName.clear();
	// Date and Time
	char datestr[256] = "";
	time_t longTime;
	struct tm *curtime;
	time(&longTime);
	curtime = localtime(&longTime);
	strftime(datestr, 256, "_%b%d_%I%M%p", curtime);
//	const MapMetaData *m =  TheMapCache->findMap(TheGlobalData->m_mapName); 
	AsciiString name = TheGlobalData->m_mapName;
	const char *fname = name.reverseFind('\\');
	if (fname)
		name = fname+1;
	name.removeLastChar(); // p
	name.removeLastChar(); // a
	name.removeLastChar(); // m
	name.removeLastChar(); // .
	m_statsFileName.clear();
#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
	if (TheGlobalData->m_saveStats)
	{
		m_statsFileName.set(TheGlobalData->m_baseStatsDir);
		m_statsFileName.concat(statsDir);
		
		if (TheNetwork)
		{
			if (TheLAN)
			{
				GameInfo *game = TheLAN->GetMyGame();
				AsciiString players;
				AsciiString full;
				AsciiString fullPlusNum;
				for (Int i=0; i<MAX_SLOTS; ++i)
				{
					GameSlot *slot = game->getSlot(i);
					if (slot && slot->isHuman())
					{
						AsciiString player;
						player.format("%ls_", slot->getName().str());
						players.concat(player);
					}
				}
				full.format("%s%s_%d_%d", players.str(), name.str(), game->getSeed(), game->getLocalSlotNum());
				AsciiString testString;
				testString.format("%s%s.txt", m_statsFileName.str(), full.str());
				m_statsFileName = testString;
			}
		}
		else
		{
			m_statsFileName.format("%s%s%s.txt",statsDir, name.str(),datestr);
		}
	}
	else
#endif
	{
		m_statsFileName.format("%s%s%s.txt",statsDir, name.str(),datestr);
	}
}

// create the header of the file
//=============================================================================
void StatsCollector::writeInitialFileInfo()
{
	//open the file
	FILE *f = fopen(m_statsFileName.str(), "w");
	if(!f)
	{
		DEBUG_ASSERTCRASH(f, ("Unable to open file %s to write", m_statsFileName.str()));
		return;
	}

	fprintf(f, "---------------------------------------------------\n");
	// Time
	struct tm *newTime;
	time_t aclock;
  time( &aclock );
  newTime = localtime( &aclock ); 
	fprintf(f, "Date:\t%s",asctime(newTime) );

	// Map
	fprintf(f, "Map:\t%s\n", TheGlobalData->m_mapName.str());

	// Side
	fprintf(f, "Side:\t%s\n", ThePlayerList->getLocalPlayer()->getSide().str());
	fprintf(f, "---------------------------------------------------\n\n");

	fprintf(f, "Time*\tBC\tMC\tAC\tSMC\tST*\tOC\t$$$\t#PU\t#AIU\n");
	collectUnitCountStats();
	Money *m = ThePlayerList->getLocalPlayer()->getMoney();
	fprintf(f, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", 0, m_buildCommands, m_moveCommands, m_attackCommands, 
					m_scrollMapCommands, 0 ,/*other commands*/0, m->countMoney(), 
					m_playerUnits, m_AIUnits );
	// initial stats
	// we don't want a file pointer open for seconds on end... we'll open it each time.
	fclose(f);
}

// Write out the stats
//=============================================================================
void StatsCollector::writeStatInfo()
{
	//open the file
	FILE *f = fopen(m_statsFileName.str(), "a");
	if(!f)
	{
		DEBUG_ASSERTCRASH(f, ("Unable to open file %s to write", m_statsFileName.str()));
		return;
	}
	Money *m = ThePlayerList->getLocalPlayer()->getMoney();
	
	fprintf(f, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", m_timeCount, m_buildCommands, m_moveCommands, m_attackCommands, 
					m_scrollMapCommands, m_scrollTime / LOGICFRAMES_PER_SECOND, /*other commands*/0,m->countMoney() , 
					m_playerUnits, m_AIUnits  );


	fclose(f);
	
}
