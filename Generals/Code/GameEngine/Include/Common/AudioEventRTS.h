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

// FILE: AudioEventRTS.h ///////////////////////////////////////////////////////////////////////////////
// AudioEventRTS structure
// Author: John K. McDonald, March 2002

#pragma once
#ifndef _H_AUDIOEVENTRTS_
#define _H_AUDIOEVENTRTS_

#include "Common/AsciiString.h"
#include "Common/GameAudio.h"
#include "Common/GameMemory.h"
#include "Common/GameType.h"

// forward declarations ///////////////////////////////////////////////////////////////////////////
struct AudioEventInfo;

enum OwnerType CPP_11(: Int)
{
	OT_Positional,
	OT_Drawable,
	OT_Object,
	OT_Dead,
	OT_INVALID
};

enum PortionToPlay CPP_11(: Int)
{
	PP_Attack,
	PP_Sound,
	PP_Decay,
	PP_Done
};

enum AudioPriority CPP_11(: Int);

// This is called AudioEventRTS because AudioEvent is a typedef in ww3d
// You might want this to be memory pooled (I personally do), but it can't
// because we allocate them on the stack frequently.
class AudioEventRTS
{	
public:
	AudioEventRTS( );
	AudioEventRTS( const AsciiString& eventName );
	AudioEventRTS( const AsciiString& eventName, ObjectID ownerID );
	AudioEventRTS( const AsciiString& eventName, DrawableID drawableID );	// Pass 0 for unused if attaching to drawable
	AudioEventRTS( const AsciiString& eventName, const Coord3D *positionOfAudio );

	virtual ~AudioEventRTS( );

	AudioEventRTS( const AudioEventRTS& right );
	AudioEventRTS& operator=( const AudioEventRTS& right );

	void setEventName( AsciiString name );
	const AsciiString& getEventName( void ) const { return m_eventName; }

	// generateFilename is separate from generatePlayInfo because generatePlayInfo should only be called once
	// per triggered event. generateFilename will be called once per loop, or once to get each filename if 'all' is 
	// specified.
	void generateFilename( void );
	AsciiString getFilename( void );

	// The attack and decay sounds are generated in generatePlayInfo, because they will never be played more
	// than once during a given sound event.
	void generatePlayInfo( void );
	Real getPitchShift( void ) const;
	Real getVolumeShift( void ) const;
	AsciiString getAttackFilename( void ) const;
	AsciiString getDecayFilename( void ) const;
	Real getDelay( void ) const;

	void decrementDelay( Real timeToDecrement );

	PortionToPlay getNextPlayPortion( void ) const;
	void advanceNextPlayPortion( void );
	void setNextPlayPortion( PortionToPlay ptp );
	
	void decreaseLoopCount( void );
	Bool hasMoreLoops( void ) const;
	
	void setAudioEventInfo( const AudioEventInfo *eventInfo ) const;
	const AudioEventInfo *getAudioEventInfo( void ) const;
	
	void setPlayingHandle( AudioHandle handle );	// for ID of this audio piece.
	AudioHandle getPlayingHandle( void ); // for ID of this audio piece

	void setPosition( const Coord3D *pos );
	const Coord3D* getPosition( void );

	void setObjectID( ObjectID objID );
	ObjectID getObjectID( void );

	Bool isDead() const { return m_ownerType == OT_Dead; }
	OwnerType getOwnerType() const { return m_ownerType; }
	
	void setDrawableID( DrawableID drawID );
	DrawableID getDrawableID( void );

	void setTimeOfDay( TimeOfDay tod );
	TimeOfDay getTimeOfDay( void ) const;

	void setHandleToKill( AudioHandle handleToKill );
	AudioHandle getHandleToKill( void ) const;

	void setShouldFade( Bool shouldFade );
	Bool getShouldFade( void ) const;

	void setIsLogicalAudio( Bool isLogicalAudio );
	Bool getIsLogicalAudio( void ) const;

	Bool isPositionalAudio( void ) const;
	Bool isCurrentlyPlaying( void ) const;

	AudioPriority getAudioPriority( void ) const;
	void setAudioPriority( AudioPriority newPriority );

	Real getVolume( void ) const;
	void setVolume( Real vol );

	Int getPlayerIndex( void ) const;
	void setPlayerIndex( Int playerNdx );

	Int getPlayingAudioIndex( void ) { return m_playingAudioIndex; };
	void setPlayingAudioIndex( Int pai )  { m_playingAudioIndex = pai; };

	Bool getUninterruptable( ) const { return m_uninterruptable; }
	void setUninterruptable( Bool uninterruptable ) { m_uninterruptable = uninterruptable; } 


	// This will retrieve the appropriate position based on type.	
	const Coord3D *getCurrentPosition( void );	

	// This will return the directory leading up to the appropriate type, including the trailing '\\'
	// If localized is true, we'll append a language specifc directory to the end of the path.
	AsciiString generateFilenamePrefix( AudioType audioTypeToPlay, Bool localized );
	AsciiString generateFilenameExtension( AudioType audioTypeToPlay );
protected:
	void adjustForLocalization( AsciiString &strToAdjust );

protected:
	AsciiString m_filenameToLoad;
	mutable const AudioEventInfo *m_eventInfo;	// Mutable so that it can be modified even on const objects
	AudioHandle m_playingHandle;
	
	AudioHandle m_killThisHandle;		///< Sometimes sounds will canabilize other sounds in order to take their handle away.
																	///< This is one of those instances.

	AsciiString m_eventName;				///< This should correspond with an entry in Dialog.ini, Speech.ini, or Audio.ini
	AsciiString m_attackName;				///< This is the filename that should be used during the attack.
	AsciiString m_decayName;				///< This is the filename that should be used during the decay.

	AudioPriority m_priority;				///< This should be the priority as given by the event info, or the overrided priority.
	Real m_volume;									///< This is the override for the volume. It will either be the normal 
	TimeOfDay m_timeOfDay;					///< This should be the current Time Of Day.

	Coord3D m_positionOfAudio;			///< Position of the sound if no further positional updates are necessary
	union	// These are now unioned.
	{
		ObjectID m_objectID;						///< ObjectID of the object that this sound is tied to. Position can be automatically updated from this.
		DrawableID m_drawableID;				///< DrawableID of the drawable that owns this sound
	};
	OwnerType m_ownerType;

	Bool m_shouldFade;							///< This should fade in or out (if it is starting or stopping)
	Bool m_isLogicalAudio;					///< Should probably only be true for scripted sounds
	Bool m_uninterruptable;

	// Playing attributes
	Real m_pitchShift;							///< Pitch shift that should occur on this piece of audio
	Real m_volumeShift;							///< Volume shift that should occur on this piece of audio
	Real m_delay;										///< Amount to delay before playing this sound
	Int m_loopCount;								///< The current loop count value. Only valid if this is a looping type event or the override has been set.
	Int m_playingAudioIndex;				///< The sound index we are currently playing. In the case of non-random, we increment this to move to the next sound
	Int m_allCount;									///< If this sound is an ALL type, then this is how many sounds we have played so far.
	
	Int m_playerIndex;							///< The index of the player who owns this sound. Used for sounds that should have an owner, but don't have an object, etc.

	PortionToPlay m_portionToPlayNext;	///< Which portion (attack, sound, decay) should be played next?
};

class DynamicAudioEventRTS : public MemoryPoolObject
{	
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(DynamicAudioEventRTS, "DynamicAudioEventRTS" )		
public:

	DynamicAudioEventRTS() { }
	DynamicAudioEventRTS(const AudioEventRTS& a) : m_event(a) { }

	AudioEventRTS	m_event;
};
EMPTY_DTOR(DynamicAudioEventRTS)

#endif /* _H_AUDIOEVENTRTS_ */

