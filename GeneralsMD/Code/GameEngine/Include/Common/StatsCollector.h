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

// FILE: StatsCollector.h /////////////////////////////////////////////////
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
//	Filename: 	StatsCollector.h
//
//	author:		Chris Huybregts
//
//	purpose:	Convinience class to help with collecting stats.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __STATSCOLLECTOR_H_
#define __STATSCOLLECTOR_H_

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class GameMessage;


//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class StatsCollector
{
public:
	StatsCollector( void );
	~StatsCollector( void );

	void reset( void );							///< Reset's all values and writes the file header

	void collectMsgStats( const GameMessage *msg );			///< collects Msg Stats if
	void collectUnitCountStats( void );									///< cycle through all units and takes count
	void incrementScrollMoveCount( void );
	void incrementBuildCount( void );
	void incrementAttackCount( void );
	void incrementMoveCount( void );
	void startScrollTime( void );		///< Start our logging on the amount of time we're scrolling
	void endScrollTime( void );			///< end our logging on the amount of time we're scrolling

	void update( void );						///< called once a frame to see if we should poll this frame

	void writeFileEnd(void);				///< Write the end of the file
private:

	void createFileName( void );		///< Create a snazzy filename
	AsciiString m_statsFileName;		///< store the snazzy filename

	void writeInitialFileInfo(void );		///< write the header file info
	void writeStatInfo( void );					///< write the stats we're keeping track of

	void zeroOutStats( void );			///< zero out the stats
	UnsignedInt m_buildCommands;		///< count of the build commands the local player issued
	UnsignedInt m_moveCommands;			///< count of the move commands
	UnsignedInt m_attackCommands;		///< attack commands
	UnsignedInt m_scrollMapCommands;///< scroll map commands
	UnsignedInt m_AIUnits;					///< tally of all the AI Units
	UnsignedInt m_playerUnits;			///< tally of all the player Units

	UnsignedInt m_scrollBeginTime;	///< Begin time in frames
	UnsignedInt m_scrollTime;				///< our totals for the scrolltime
	Bool m_isScrolling;							///< flag to make sure we are scrolling

	Int m_timeCount;								///< the current timeframe we're on
	Int m_lastUpdate;								///< last time we updated
	Int m_startFrame;								///< frame we started on
};


//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
extern StatsCollector* TheStatsCollector;			///< we need a singleton

#endif // __STATSCOLLECTOR_H_
