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

//----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright(C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: GameMusic.cpp
//
// Created:   5/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes                                                      
//----------------------------------------------------------------------------

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameMusic.h"

#include "Common/AudioEventRTS.h"
#include "Common/AudioRequest.h"
#include "Common/GameAudio.h"
#include "Common/INI.h"


//----------------------------------------------------------------------------
//         Externals                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines                                                         
//----------------------------------------------------------------------------
#define MUSIC_PATH "Data\\Audio\\Tracks"  // directory path to the music files


//----------------------------------------------------------------------------
//         Private Types                                                     
//----------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/** The INI data fields for music tracks */
//-------------------------------------------------------------------------------------------------
const FieldParse MusicTrack::m_musicTrackFieldParseTable[] = 
{

	{ "Filename",								INI::parseAsciiString,							NULL, offsetof( MusicTrack, filename ) },
	{ "Volume",									INI::parsePercentToReal,						NULL, offsetof( MusicTrack, volume ) },
	{ "Ambient",								INI::parseBool,											NULL, offsetof( MusicTrack, ambient ) },
	{ NULL,											NULL,																NULL, 0 },
};


//-------------------------------------------------------------------------------------------------
MusicManager::MusicManager()
{	

}

//-------------------------------------------------------------------------------------------------
MusicManager::~MusicManager()
{

}

//-------------------------------------------------------------------------------------------------
void MusicManager::playTrack( AudioEventRTS *eventToUse )
{
	AudioRequest *audioRequest = TheAudio->allocateAudioRequest( true );
	audioRequest->m_pendingEvent = eventToUse;
	audioRequest->m_request = AR_Play;
	TheAudio->appendAudioRequest( audioRequest );
}

//-------------------------------------------------------------------------------------------------
void MusicManager::stopTrack( AudioHandle eventToRemove )
{
	AudioRequest *audioRequest = TheAudio->allocateAudioRequest( false );
	audioRequest->m_handleToInteractOn = eventToRemove;
	audioRequest->m_request = AR_Stop;
	TheAudio->appendAudioRequest( audioRequest );
}

//-------------------------------------------------------------------------------------------------
void MusicManager::addAudioEvent( AudioEventRTS *eventToAdd )
{
	playTrack( eventToAdd );
}

//-------------------------------------------------------------------------------------------------
void MusicManager::removeAudioEvent( AudioHandle eventToRemove )
{
	stopTrack( eventToRemove );
}

