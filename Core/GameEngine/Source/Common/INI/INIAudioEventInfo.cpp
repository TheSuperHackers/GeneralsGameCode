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

// FILE: INIAudioEventInfo.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, July 2002
// Desc:   Parsing AudioEvent, MusicTrack and DialogEvent INI entries
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/INI.h"
#include "Common/GameAudio.h"
#include "Common/AudioEventInfo.h"

AudioEventInfo::~AudioEventInfo()
{

}
// @todo: Get these functions unified.

//-------------------------------------------------------------------------------------------------
void INI::parseMusicTrackDefinition( INI* ini )
{
	AsciiString name;
	AudioEventInfo *track;

	// read the name
	const char* c = ini->getNextToken();
	name.set( c );

	track = TheAudio->newAudioEventInfo( name );
	if (!track) {
		return;
	}

	AudioEventInfo *defaultInfo = TheAudio->findAudioEventInfo("DefaultMusicTrack");
	if (defaultInfo) {
		(*track) = (*defaultInfo);
		TheAudio->addTrackName( name );
	}

	track->m_audioName = name;
	track->m_soundType = AT_Music;

	// parse the ini definition
	ini->initFromINI( track, track->getFieldParse() );
}

//-------------------------------------------------------------------------------------------------
void INI::parseAudioEventDefinition( INI* ini )
{
	AsciiString name;
	AudioEventInfo *track;

	// read the name
	const char* c = ini->getNextToken();
	name.set( c );

	track = TheAudio->newAudioEventInfo( name );
	if (!track) {
		return;
	}

	AudioEventInfo *defaultInfo = TheAudio->findAudioEventInfo("DefaultSoundEffect");
	if (defaultInfo) {
		(*track) = (*defaultInfo);
	}

	track->m_audioName = name;
	track->m_soundType = AT_SoundEffect;

	// parse the ini definition
	ini->initFromINI( track, track->getFieldParse() );
}

//-------------------------------------------------------------------------------------------------
void INI::parseDialogDefinition( INI* ini )
{
	AsciiString name;
	AudioEventInfo *track;

	// read the name
	const char* c = ini->getNextToken();
	name.set( c );

	track = TheAudio->newAudioEventInfo( name );
	if (!track) {
		return;
	}

	AudioEventInfo *defaultInfo = TheAudio->findAudioEventInfo("DefaultDialog");
	if (defaultInfo) {
		(*track) = (*defaultInfo);
	}

	track->m_audioName = name;
	track->m_soundType = AT_Streaming;

	// parse the ini definition
	ini->initFromINI( track, track->getFieldParse() );
}


//-------------------------------------------------------------------------------------------------
static void parseDelay( INI* ini, void *instance, void *store, const void* /*userData*/ );
static void parsePitchShift( INI* ini, void *instance, void *store, const void* /*userData*/ );

//-------------------------------------------------------------------------------------------------
const FieldParse AudioEventInfo::m_audioEventInfo[] =
{
	{ "Filename",							INI::parseAsciiString,		NULL,								offsetof( AudioEventInfo, m_filename) },
	{ "Volume",								INI::parsePercentToReal,	NULL,								offsetof( AudioEventInfo, m_volume ) },
	{ "VolumeShift",					INI::parsePercentToReal,	NULL,								offsetof( AudioEventInfo, m_volumeShift ) },
	{ "MinVolume",						INI::parsePercentToReal,	NULL,								offsetof( AudioEventInfo, m_minVolume ) },
	{ "PitchShift",						parsePitchShift,					NULL,								0 },
	{ "Delay",								parseDelay,								NULL,								0 },
	{ "Limit",								INI::parseInt,						NULL,								offsetof( AudioEventInfo, m_limit ) },
	{ "LoopCount",						INI::parseInt,						NULL,								offsetof( AudioEventInfo, m_loopCount ) },
	{ "Priority",							INI::parseIndexList,			theAudioPriorityNames,	offsetof( AudioEventInfo, m_priority ) },
	{ "Type",									INI::parseBitString32,		theSoundTypeNames,			offsetof( AudioEventInfo, m_type ) },
	{ "Control",							INI::parseBitString32,		theAudioControlNames,	offsetof( AudioEventInfo, m_control ) },
	{ "Sounds",								INI::parseSoundsList,			NULL,								offsetof( AudioEventInfo, m_sounds ) },
	{ "SoundsNight",					INI::parseSoundsList,			NULL,								offsetof( AudioEventInfo, m_soundsNight ) },
	{ "SoundsEvening",				INI::parseSoundsList,			NULL,								offsetof( AudioEventInfo, m_soundsEvening ) },
	{ "SoundsMorning",				INI::parseSoundsList,			NULL,								offsetof( AudioEventInfo, m_soundsMorning ) },
	{ "Attack",								INI::parseSoundsList,			NULL,								offsetof( AudioEventInfo, m_attackSounds ) },
	{ "Decay",								INI::parseSoundsList,			NULL,								offsetof( AudioEventInfo, m_decaySounds ) },
	{ "MinRange",							INI::parseReal,						NULL,								offsetof( AudioEventInfo, m_minDistance) },
	{ "MaxRange",							INI::parseReal,						NULL,								offsetof( AudioEventInfo, m_maxDistance) },
	{ "LowPassCutoff",				INI::parsePercentToReal,	NULL,								offsetof( AudioEventInfo, m_lowPassFreq) },
};

//-------------------------------------------------------------------------------------------------
static void parseDelay( INI* ini, void *instance, void *store, const void* /*userData*/ )
{
	AudioEventInfo *attribs = (AudioEventInfo*) store;

	Int min = ini->scanInt(ini->getNextToken());
	Int max = ini->scanInt(ini->getNextToken());

	DEBUG_ASSERTCRASH( min >= 0 && max >= min, ("Bad delay values for audio event %s", attribs->m_audioName.str()));
	attribs->m_delayMax = max;
	attribs->m_delayMin = min;
}

//-------------------------------------------------------------------------------------------------
static void parsePitchShift( INI* ini, void *instance, void *store, const void* /*userData*/ )
{
	AudioEventInfo *attribs = (AudioEventInfo*) store;

	Real min = ini->scanReal(ini->getNextToken());
	Real max = ini->scanReal(ini->getNextToken());

	DEBUG_ASSERTCRASH( min > -100 && max >= min, ("Bad pitch shift values for audio event %s", attribs->m_audioName.str()));
	attribs->m_pitchShiftMin = 1.0f + min/100;
	attribs->m_pitchShiftMax = 1.0f + max/100;
}

// STATIC DEFINIITIONS ////////////////////////////////////////////////////////////////////////////

const char *const theAudioPriorityNames[] =
{
	"LOWEST",
	"LOW",
	"NORMAL",
	"HIGH",
	"CRITICAL",
	NULL
};
static_assert(ARRAY_SIZE(theAudioPriorityNames) == AP_COUNT + 1, "Incorrect array size");

const char *const theSoundTypeNames[] =
{
	"UI",
	"WORLD",
	"SHROUDED",
	"GLOBAL",
	"VOICE",
	"PLAYER",
	"ALLIES",
	"ENEMIES",
	"EVERYONE",
	NULL
};

const char *const theAudioControlNames[] =
{
	"LOOP",
	"RANDOM",
	"ALL",
	"POSTDELAY",
	"INTERRUPT",
	NULL
};

