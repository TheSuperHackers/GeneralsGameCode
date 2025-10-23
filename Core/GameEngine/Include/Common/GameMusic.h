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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:    RTS 3
//
// File name:  Common/GameMusic.h
//
// Created:    5/01/01
//
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include "Common/GameAudio.h"
#include "Common/GameMemory.h"


//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------

class AudioEventRTS;
struct FieldParse;

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------


//===============================
// MusicTrack
//===============================

//-------------------------------------------------------------------------------------------------
/** The MusicTrack struct holds all information about a music track.
	* Place data in TrackInfo that is useful to the game code in determining
	* what tracks to play. */
//-------------------------------------------------------------------------------------------------

class MusicTrack : public MemoryPoolObject
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( MusicTrack, "MusicTrack" )

public:

		MusicTrack();
		// virtual destructor prototype defined by memory pool object

		const FieldParse *getFieldParse( void ) const { return m_musicTrackFieldParseTable; }

		Int					index;									///< Track index
		AsciiString name;										///< Logical name of track
		AsciiString filename;								///< Filename with extension of music track
		Real				volume;									///< Mixing level for this track
		Bool				ambient;								///< Game info about this track(public)

		MusicTrack *next;
		MusicTrack *prev;

	static const FieldParse m_musicTrackFieldParseTable[];		///< the parse table for INI definition

};

class MusicManager
{
	public:
		MusicManager();
		virtual ~MusicManager();

		void playTrack( AudioEventRTS *eventToUse );
		void stopTrack( AudioHandle eventToRemove );

		virtual void addAudioEvent(AudioEventRTS *eventToAdd);	// pre-copied
		virtual void removeAudioEvent( AudioHandle eventToRemove );

		void setVolume( Real m_volume );
};
