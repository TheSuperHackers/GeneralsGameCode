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
// File name: GameSpeech.cpp
//
// Created:   10/30/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes                                                      
//----------------------------------------------------------------------------

#include "wpaudio/attributes.h"

#include "wsys/file.h"
#include "wsys/List.h"
#include "wpaudio/Streamer.h"
#include "wpaudio/Time.h"
#include "wpaudio/Device.h"
#include "wpaudio/Streamer.h"

#define DEFINE_DLG_EVENT_PRIORITY_NAMES
#include "Common/GameAudio.h"
#include "Common/GameSpeech.h"
#include "Common/INI.h"

#include "Common/STLTypedefs.h"


//----------------------------------------------------------------------------
//         Externals                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines                                                         
//----------------------------------------------------------------------------
#define		BASE_DLG_ID		((ID) 5000)
#define		BASE_DLG_DIR	"Data\\Audio\\Sounds"
#define		BASE_DLG_EXT	"wav"
#define		NUM_DLG_PRIORITIES	5

//----------------------------------------------------------------------------
//         Private Types                                                     
//----------------------------------------------------------------------------

//===============================
// Speech
//===============================
/**
  *	The Speech struct holds all information about a line of dialog. While Speech itself is not 
	* accessible from outside SpeechInterface, the embedded struct SpeechInfo is accessible to the game code.
	* Place data in SpeechInfo that is useful to the game code in determining what speech to play.
	*/
//===============================

struct Speech
{

		Speech();
		~Speech();
		
		ID					id;											///< Unquie GDI id
		Int					index;									///< Speech index
		AsciiString	name;										///< Logical name for line of speech
		Real				volume;									///< Mixing level for this line
		SpeechInfo	info;										///< Game info about this line of dialog (public)
		Bool				valid;									///< Is this entry have valid data
		Int					priority;								///< speech priority
		Int					timeout;								///< speech time out in milliseconds
		Int					interrupt;							///< speech interrupts current playing speech

};

static void speechFromInfo(SpeechInfo* pInSpeechInfo, Speech* pOutSpeech);
static AsciiString getNextFilenameFromSpeech(Speech* pSpeechToGetFilenameFrom);

//===============================
// SpeechRequest 
//===============================
/**
  *	This is an internal structure used by the SpeechManager. It holds information 
	* about the playback of a line of dialog.
	*/
//===============================

struct SpeechItem
{
	enum Flags
	{
		PAUSED		= 0x00000001
	};


	LListNode				nd;
	Flags						flags;
	Speech					*speech;								///< What to say
	Int							priority;								///< Priority of this line
	TimeStamp				timeout;								///< Time when this event is stale in milliseconds

};

//===============================
// Speaker 
//===============================
/**
  * Actual implementation of SpeakerInterface
	*/
//===============================

class SpeechManager;

class Speaker : public SpeakerInterface
{
	AsciiString				m_name;								///< name of this speaker
	AudioStreamer			*m_stream;						///< Audio streamer for this speaker
	File							*m_file;							///< file we are streaming
	Int								m_priority;						///< Speaker's priority
	Int								m_paused;							///< paused level
	TimeStamp					m_delay;							///< current delay interval
	TimeStamp					m_delayTime;					///< delay time between speeches
	SpeechManager			*m_manager;						///< Manager that created use

	// info about current playing dialog
	SpeechItem				*m_currentSpeech;			///< currently playing speech

	// list of pending speech items
	LList							m_pending;						///< list of pending SpeechItems

	public:

		Speaker();
		~Speaker();

		virtual void			destroy( void );												///< Delete and free speaker
		/// Submits speech to play																
		virtual void			say (	Speech *speech,										// speech to say
														Int priority,											// priority 
														Int timeout,											// time in which to say this line, 0 = infinite
														Int interrupt);										// whether to interrupt current line
																															
		virtual void			say (	Char *speechName,									// name ofspeech to say
														Int priority,											// priority 
														Int timeout,											// time in which to say this line, 0 = infinite
														Int interrupt );									// whether to interrupt current line

		virtual void			setPriority ( Int priority );						///< Set speaker's priority level
		virtual void			setDelay ( Int delay );									///< Set speaker's delay between speeches( in milliseconds)
		virtual void			setBuffering ( Int buffer_time );				///< Set speaker's amount of buffer (in milliseconds)
		virtual Int				getPriority ( void );										///< Get speaker's priority level
		virtual Int				getDelay ( void );											///< Get speaker's delay between speeches( in milliseconds)
		virtual Int				getBuffering ( void	);									///< Get speaker's amount of buffer (in milliseconds)
		virtual void			pause ( void );													///< pause speaker
		virtual void			resume ( void );												///< resume speaking
		virtual void			stop ( void );													///< stop speaking (flushes queue)
		virtual void			cancel ( Speech *speech );							///< cancel pending line of dialog
		virtual Bool			hasSaid ( Speech *speech );							///< is line of dialog no longer pending
		virtual Bool			isGoingToSay ( Speech *speech );				///< check in line of dialog is pending
		virtual Bool			isTalking ( void );											///< is speaker talking
		virtual Speech*		saying ( void );												///< returns currently spoken speech
		virtual void			update( void );													///< Service speaker


		void init ( char *name, Int priority, Int btime, Int delay ); ///< initialized speaker for use
		void deinit( void );
		void setManager( SpeechManager *manager ) { m_manager = manager;};
		SpeechItem* firstItem( TimeStamp now = 0 );
		SpeechItem* nextItem( SpeechItem *item, TimeStamp now = 0 );
		SpeechItem* findItem( Speech *speech );
		void flush( void );
};

typedef std::vector<Speech> VecSpeech;
typedef std::hash_map<const char*, Speech, std::hash<const char*>, rts::equal_to<const char*> > HashSpeech;

//===============================
// SpeechManager: 
//===============================
/**
  *	Actual implementation od SpeechInterface
	*/
//===============================

class SpeechManager: public SpeechInterface
{

	protected:

		struct AudioAttribs		*m_masterAttribs;										///< Attribs for all speech
		struct AudioAttribs		*m_masterFadeAttribs;								///< Fade attribs for all speech
		Real										m_volume;													///< Current speech volume
		Bool										m_on;															///< Is speech turned on?
		Int											m_count;													///< Number of speeches in the table
																															
		struct AudioDeviceTag		*m_device;												///< The audio device we were initilaized with

		LList										m_speakers;												///< list of speakers created by this manager
		HashSpeech							m_speech;													///< This is a hash table of all speeches we currently have loaded.
		VecAsciiString					m_speechNames;										///< The purpose of this is to return to someone asking for the name of some element.
		VecSpeech								m_temporarySpeeches;							///< This is a vector of speeches that are _like_ something in the hash, but differ by priority.
	
		AsciiString getFilenameForPlay( Speech *speech );			///< Return the filename corresponding to this speech

	public:

		SpeechManager();
		virtual ~SpeechManager();

		virtual Bool					init( void );												///< Initlaizes the speech system
		virtual void					deinit( void );											///< De-initlaizes the speech system
		virtual void					update( void );											///< Services speech tasks. Called by AudioInterface
		virtual void					reset( void );											///< Resets the speech system
		virtual void					loseFocus( void );									///< Called when application loses focus
		virtual void					regainFocus( void );								///< Called when application regains focus
	
			// speech info access
		virtual Int						numSpeeches( void );								///< Returns the number of speechs defined
		virtual SpeechInfo*		getSpeechInfo( Speech *speech );		///< Returns speech info by speech
		virtual Int						getSpeechIndex( Speech *speech );		///< Returns speech index
		virtual ID						getSpeechID( Speech *speech );			///< Returns speech ID
		virtual Speech*				getSpeech( Int index );							///< Converts index to speech
		virtual Speech*				getSpeech( ID id );									///< Converts is to speech
		virtual Speech*				getSpeech( const Char *speech_name );///< Converts speech name to speech
		virtual const Char*		getSpeechName( Speech *speech );		///< Converts speech to speech name

		// volume control
		virtual void					fadeIn( void );											///< Fade all speech in
		virtual void					fadeOut( void );										///< Fade all speech out
		virtual void					fade( Real fade_value );						///< Fade all speech by a specified amout (1.0 .. 0.0 )
		virtual Bool					isFading( void );										///< Returns whether or not any speech is in the process of a fade
		virtual void					waitForFade( void );								///< Returns when all fading has finished
		virtual void					setVolume( Real new_volume );				///< Set new volume level for all speech 1.0 (loudest) 0.0 (silent)
		virtual Real					getVolume( void );									///< Return current volume level for all speech

		/// Speaker factory
		virtual SpeakerInterface*			createSpeaker (	Char *name, 
																	Int priority, 
																	Int buffer_time,
																	Int delay);

		virtual Speech*				addNewSpeech( SpeechInfo* pSpeechToAdd);	///< Create a speech specified by pSpeechToAdd
		virtual Speech*				addTemporaryDialog( Speech& temporarySpeech );	///< Speech to be taken over by 
		// playback control																
		virtual void					stop( void );												///< Stops all speech that's currently playing.
		virtual void					pause( void );											///< Pauses all speech
		virtual void					resume( void );											///< Resumes all paused speech
		virtual void					turnOff( void );										///< Turns off all speech. No more speech will play
		virtual void					turnOn( void );											///< Turns speech on.
		virtual Bool					waitToStop( Int milliseconds );			///< Returns when all speech has completely stopped playing
		virtual Bool					say( Speech* pSpeechToSay);					///< Attempts to add a speech to the speach queue.
		virtual Bool					say( AsciiString speechName);				///< Attempts to find and add a speech to the speech queue
																											
		// info																						
		virtual Bool					isOn( void );												///< Returns whether or not speech is turned on at present
		virtual struct AudioAttribs* getMasterAttribs( void );		///< Returns the master attribute control for all speech
		virtual struct AudioAttribs* getMasterFadeAttribs( void );///< Returns the master fade control for all speech
		virtual void					addDialogEvent(const AudioEventRTS *eventRTS, Speech* speech, AudioEventRTS *returnEvent);	///< Add an event. speech can be NULL
		virtual void					removeDialogEvent(const AudioEventRTS *eventRTS, AudioEventRTS *eventToUse);	///< remove an event, eventToUse shouldn't be NULL.

		void removeSpeaker ( Speaker *speaker );
		void addSpeaker ( Speaker *speaker );

		// empty string means that this sound wasn't found or some error occurred. CHECK FOR EMPTY STRING.
		virtual AsciiString getFilenameForPlayFromAudioEvent( const AudioEventRTS *eventToGetFrom );
};

//----------------------------------------------------------------------------
//         Private Data                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data                                                      
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//         Private Prototypes                                               
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions                                               
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Functions                                                
//----------------------------------------------------------------------------

//============================================================================
// createSpeechInterface
//============================================================================

SpeechInterface* CreateSpeechInterface( void )
{
	return NEW SpeechManager;
}

//============================================================================
// Speech::Speech
//============================================================================

Speech::Speech()
: id(INVALID_ID),
	valid(FALSE),
	volume(TheAudio->VOLUME_MIN)
{

}

//============================================================================
// Speech::~Speech
//============================================================================

Speech::~Speech()
{
}

//============================================================================
// SpeechManager::SpeechManager
//============================================================================

SpeechManager::SpeechManager()
: m_on(FALSE),
	m_device(NULL),
	m_masterAttribs(NULL),
	m_masterFadeAttribs(NULL),
	m_speech(NULL),
	m_count(0)
{
	if( (m_masterFadeAttribs = NEW AudioAttribs) != 0 )		// poolify
	{
		AudioAttribsInit( m_masterFadeAttribs );
	}

	if( (m_masterAttribs = NEW AudioAttribs) != 0 )
	{
		AudioAttribsInit( m_masterAttribs );
	}

}

//============================================================================
// SpeechManager::~SpeechManager
//============================================================================

SpeechManager::~SpeechManager()
{
	deinit();

	delete m_masterAttribs;
	delete m_masterFadeAttribs;

}

//============================================================================
// SpeechManager::init
//============================================================================

Bool SpeechManager::init( void )
{
	deinit();
	if ( m_device )
	{
		return TRUE;
	}

	m_device = TheAudio->device();

	if( m_device )
	{
		AudioDeviceAttribsAdd( m_device, m_masterAttribs );
		AudioDeviceAttribsAdd( m_device, m_masterFadeAttribs );
	}

	for (int i = 0; i < NUM_DLG_PRIORITIES; ++i) {
		createSpeaker(dlgPriorityNames[i], (i + 1), 3000, 300);
	}
	
	m_on = TRUE;

	return TRUE;
}

//============================================================================
// SpeechManager::deinit
//============================================================================

void SpeechManager::deinit( void )
{
	m_count = 0;
	m_speech.clear();
	m_speechNames.clear();
	
	for (int i = 0; i < NUM_DLG_PRIORITIES && m_speakers.firstNode(); ++i) {
		((Speaker*) m_speakers.firstNode()->item())->destroy();
	}

	stop();

	if( m_device )
	{
		AudioDeviceAttribsRemove( m_device, m_masterAttribs );
		AudioDeviceAttribsRemove( m_device, m_masterFadeAttribs );
		m_device = NULL;
	}
}

//============================================================================
// SpeechManager::update
//============================================================================

void SpeechManager::update( void )
{
	LListNode *node = m_speakers.firstNode();

	while ( node )
	{
		Speaker *speaker = (Speaker *) node->item();

		if ( speaker )
		{
			speaker->update();
		}

		node = node->next();
	}

}

//============================================================================
// SpeechManager::reset
//============================================================================

void SpeechManager::reset( void )
{
}

//============================================================================
// SpeechManager::loseFocus
//============================================================================

void SpeechManager::loseFocus( void )
{

}

//============================================================================
// SpeechManager::regainFocus
//============================================================================

void SpeechManager::regainFocus( void )
{

}

//============================================================================
// SpeechManager::numSpeeches
//============================================================================

Int	SpeechManager::numSpeeches( void )
{
	return m_count ;
}

//============================================================================
// SpeechManager::getSpeechInfo
//============================================================================

SpeechInfo* SpeechManager::getSpeechInfo( Speech *speech )
{
	if( speech )
	{
		return &speech->info ;
	}

	return NULL;
}

//============================================================================
// SpeechManager::getSpeechIndex
//============================================================================

Int SpeechManager::getSpeechIndex( Speech *speech )
{
	if( speech )
	{
		return speech->index ;
	}

	return SpeechInterface::INVALID_SPEECH;
}

//============================================================================
// SpeechManager::getSpeechID
//============================================================================

ID SpeechManager::getSpeechID( Speech *speech )
{
	if( speech )
	{
		return speech->id ;
	}

	return INVALID_ID;
}

//============================================================================
// SpeechManager::getSpeech
//============================================================================

Speech* SpeechManager::getSpeech( const Char *name )
{
	HashSpeech::iterator it = m_speech.find(name);
	if (it != m_speech.end()) {
		return &(*it).second;
	}
	return NULL;
}

//============================================================================
// SpeechManager::getSpeech
//============================================================================

Speech* SpeechManager::getSpeech( ID id )
{
	if ( m_count == 0 )
	{
		return NULL;
	}
	for (int i = 0; i < m_speechNames.size(); ++i) {
		Speech* pSpeech = getSpeech(m_speechNames[i].str());
		if (pSpeech && pSpeech->id == id) {
			return pSpeech;
		}
	}

	return NULL;
}

//============================================================================
// SpeechManager::getSpeech
//============================================================================

Speech* SpeechManager::getSpeech( int index )
{
	if (index < 0 || index > m_count) {
		return NULL;
	}
	return getSpeech(m_speechNames[index].str());
}

//============================================================================
// SpeechManager::getSpeechName
//============================================================================

const Char* SpeechManager::getSpeechName( Speech *speech )
{
	if( speech )
	{
		return speech->name.str();
	}
	return "(null)";
}

//============================================================================
// SpeechManager::fadeIn
//============================================================================

void SpeechManager::fadeIn( void )
{
	if(m_masterFadeAttribs )
	{
		AudioAttribsAdjustVolume( m_masterFadeAttribs, AUDIO_VOLUME_MAX );
	}
}

//============================================================================
// SpeechManager::fadeOut
//============================================================================

void SpeechManager::fadeOut( void )
{
	if(m_masterFadeAttribs )
	{
		AudioAttribsAdjustVolume( m_masterFadeAttribs, AUDIO_VOLUME_MIN );
	}
}

//============================================================================
// SpeechManager::fade
//============================================================================

void SpeechManager::fade( Real new_volume )
{
	if(m_masterFadeAttribs )
	{
		AudioAttribsAdjustVolume( m_masterFadeAttribs, TheAudio->convertRealVolume( new_volume) );
	}
}

//============================================================================
// SpeechManager::isFading
//============================================================================

Bool SpeechManager::isFading( void )
{
	if( m_masterFadeAttribs )
	{
		return !AudioAttribsVolumeAdjusted( m_masterFadeAttribs );
	}
	return FALSE;
}

//============================================================================
// SpeechManager::waitForFade
//============================================================================

void SpeechManager::waitForFade( void )
{

}

//============================================================================
// SpeechManager::setVolume
//============================================================================

void SpeechManager::setVolume( Real new_volume )
{
	m_volume = TheAudio->validateVolume(new_volume);

	if( m_masterAttribs )
	{
		AudioAttribsSetVolume( m_masterAttribs, TheAudio->convertRealVolume( new_volume ));
	}
}

//============================================================================
// SpeechManager::getVolume
//============================================================================

Real SpeechManager::getVolume( void )
{
	return m_volume;
}

void SpeechManager::stop( void )
{
	LListNode *node = m_speakers.firstNode();

	while ( node )
	{
		Speaker *speaker = (Speaker *) node->item();

		if ( speaker )
		{
			speaker->stop();
		}

		node = node->next();
	}
}

//============================================================================
// SpeechManager::pause
//============================================================================

void SpeechManager::pause( void )
{
	LListNode *node = m_speakers.firstNode();

	while ( node )
	{
		Speaker *speaker = (Speaker *) node->item();

		if ( speaker )
		{
			speaker->pause();
		}

		node = node->next();
	}
}

//============================================================================
// SpeechManager::resume
//============================================================================

void SpeechManager::resume( void )
{
	LListNode *node = m_speakers.firstNode();

	while ( node )
	{
		Speaker *speaker = (Speaker *) node->item();

		if ( speaker )
		{
			speaker->resume();
		}

		node = node->next();
	}
}

//============================================================================
// SpeechManager::turnOff
//============================================================================

void SpeechManager::turnOff( void )
{
	stop();
	m_on = FALSE;
}

//============================================================================
// SpeechManager::turnOn
//============================================================================

void SpeechManager::turnOn( void )
{
	m_on = TRUE;
}

//============================================================================
// SpeechManager::waitToStop
//============================================================================
Bool SpeechManager::waitToStop( Int milliseconds )
{

	return TRUE;

}

//============================================================================
// SpeechManager::say
//============================================================================
Bool SpeechManager::say( Speech* pSpeechToSay)
{
	// TBD: Find the other conditions
	if (m_speakers.isEmpty() || !pSpeechToSay) {
		return false;
	}
	
	LListNode *node = m_speakers.firstNode();
	for (int i = 0; i < pSpeechToSay->priority && node; ++i) {
		node = node->next();
	}
	
	if (!node) {
		return false;
	}
	
	Speaker* currentSpeaker = (Speaker*) node->item();
	currentSpeaker->say(pSpeechToSay, pSpeechToSay->priority, pSpeechToSay->timeout, pSpeechToSay->interrupt);
	return true;
}

//============================================================================
// SpeechManager::say
//============================================================================
Bool SpeechManager::say( AsciiString speechName)
{
	Speech* pSpeech = getSpeech(speechName.str());
	if (pSpeech) {
		return false;
	}

	return say(pSpeech);
}


//============================================================================
// SpeechManager::isOn
//============================================================================

Bool SpeechManager::isOn( void )
{ 
	return m_on;
}

//============================================================================
// SpeechManager::removeSpeaker 
//============================================================================

void SpeechManager::removeSpeaker ( Speaker *speaker )
{
	LListNode *node = m_speakers.findItem( speaker );

	if ( node )
	{
		node->remove();
		node->destroy();
	}
}

//============================================================================
// SpeechManager::addSpeaker 
//============================================================================

void SpeechManager::addSpeaker ( Speaker *speaker )
{
	// remove it if it is already in the list
	removeSpeaker( speaker );	
	m_speakers.addItem( speaker->getPriority(), speaker );
}

//============================================================================
// SpeechManager::CreateSpeaker 
//============================================================================

SpeakerInterface* SpeechManager::createSpeaker (	Char *name, Int priority, Int btime, Int delay )
{
	Speaker *speaker = NEW Speaker;	// poolify

	if ( speaker )
	{
		speaker->setManager( this );
		speaker->init( name, priority, btime, delay );
		m_speakers.addItem( priority, speaker );
	}
	return speaker;
}

//============================================================================
// SpeechManager::addNewSpeech
//============================================================================
Speech* SpeechManager::addNewSpeech( SpeechInfo* pSpeechToAdd )
{
	if (!pSpeechToAdd) {
		return NULL;
	}

	const char* entryKey = pSpeechToAdd->m_dialogEvent.str();

	if (m_speech.find(entryKey) == m_speech.end()) {
		++m_count;
		m_speechNames.push_back(pSpeechToAdd->m_dialogEvent);
	}

	Speech newSpeech;
	speechFromInfo(pSpeechToAdd, &newSpeech);
	newSpeech.id = (ID) (m_count + BASE_DLG_ID);

	m_speech[entryKey] = newSpeech;
	return &m_speech[entryKey];
}

//============================================================================
// SpeechManager::addTemporaryDialog
//============================================================================
Speech* SpeechManager::addTemporaryDialog( Speech& temporarySpeech )
{
		m_temporarySpeeches.push_back(temporarySpeech);
		return &(m_temporarySpeeches.back());
}


//============================================================================
// SpeechManager::getMasterAttribs
//============================================================================

struct AudioAttribs* SpeechManager::getMasterAttribs( void )
{ 
	return m_masterAttribs;
};

//============================================================================
// SpeechManager::getMasterFadeAttribs
//============================================================================

struct AudioAttribs* SpeechManager::getMasterFadeAttribs( void )
{ 
	return m_masterFadeAttribs;
};

//============================================================================
// SpeechManager::getMasterFadeAttribs
//============================================================================

void SpeechManager::addDialogEvent(const AudioEventRTS *eventRTS, Speech* speech, AudioEventRTS *returnEvent)
{
	if (!eventRTS) {
		return;
	}

	Speech *useSpeech = speech;
	if (!useSpeech) {
		useSpeech = getSpeech(eventRTS->m_eventName.str());
	}
	
	if (!useSpeech) {
		return;
	}
	
	if (1 || (eventRTS->m_priorityBoost == 0 && 
			eventRTS->m_overrideLoopingBehavior == 0 &&
			eventRTS->m_timeOfDay == useSpeech->info.m_timeOfDay)) {
		// We can use the default sound, so no need to create and add a new one.
		say(useSpeech);
		if (returnEvent) {
			(*returnEvent) = (*eventRTS);
			(*returnEvent).m_speechToPlay = useSpeech;
			(*returnEvent).m_isCurrentlyPlaying = true;
		}
		return;
	}

	Speech tmpSpeech = (*useSpeech);
	tmpSpeech.info.m_priority += eventRTS->m_priorityBoost;
	// Can't go less than 0 priority or > NUM_DLG_PRIORITIES
	if (tmpSpeech.info.m_priority < 0) {
		tmpSpeech.info.m_priority = 0;
	} else if (tmpSpeech.info.m_priority > NUM_DLG_PRIORITIES) {
		tmpSpeech.info.m_priority = NUM_DLG_PRIORITIES;
	}

	tmpSpeech.info.m_timeOfDay = eventRTS->m_timeOfDay;
#if 0
	// CRASHING BUG IN HERE. NEED TO FIX.
	Speech* playSpeech = addTemporaryDialog(tmpSpeech);
	if (returnEvent) {
			(*returnEvent) = (*eventRTS);
			returnEvent->m_speechToPlay = NULL;
	}

	say(playSpeech);
#endif
}

//============================================================================
// SpeechManager::removeDialogEvent
//============================================================================

void SpeechManager::removeDialogEvent(const AudioEventRTS *eventRTS, AudioEventRTS* eventToUse)
{
	if (!eventRTS) {
		return;
	}

	Speech *useSpeech = eventRTS->m_speechToPlay;

	if (!useSpeech) {
		useSpeech = getSpeech(eventRTS->m_eventName.str());
	}
	
	if (!useSpeech) {
		return;
	}

	if (eventToUse) {
		(*eventToUse) = (*eventRTS);
		eventToUse->m_speechToPlay = useSpeech;
	}

	Bool isPlaying = true;
	LListNode *node = m_speakers.firstNode();
	while (node && isPlaying) {
		Speaker *pCurrSpeaker = (Speaker*) node->item();		
		if (pCurrSpeaker->saying() == useSpeech || pCurrSpeaker->isGoingToSay(useSpeech)) {
			pCurrSpeaker->cancel(useSpeech);
			isPlaying = false;
		}
	}
	
	if (eventToUse) {
		(*eventToUse).m_isCurrentlyPlaying = isPlaying;
	}
}

//============================================================================
// SpeechManager::getFilenameForPlayFromAudioEvent
//============================================================================

AsciiString SpeechManager::getFilenameForPlayFromAudioEvent( const AudioEventRTS *eventToGetFrom )
{
	if (!eventToGetFrom) {
		return AsciiString::TheEmptyString;
	}

	if (eventToGetFrom->m_eventName.isEmpty()) {
		return AsciiString::TheEmptyString;
	}

	Speech* speech = getSpeech(eventToGetFrom->m_eventName.str());
	return getFilenameForPlay(speech);
}

//============================================================================
// SpeechManager::getFilenameForPlay
//============================================================================
AsciiString SpeechManager::getFilenameForPlay( Speech *speech )
{
	if (!speech) {
		return AsciiString::TheEmptyString;
	}

	SpeechInfo* speechInfo = getSpeechInfo(speech);
	
	if (!speechInfo) {
		return AsciiString::TheEmptyString;
	}

	Int regularSamples = speechInfo->m_dialogFiles.size();
	Int eveningSamples = speechInfo->m_dialogFilesEvening.size();
	Int morningSamples = speechInfo->m_dialogFilesMorning.size();
	Int nightSamples = speechInfo->m_dialogFilesNight.size();
	Int numSamples = regularSamples + eveningSamples + morningSamples + nightSamples;
	
	// using a random number generator, select a random sample from all the samples
	// that correspond to this sound name
	Int soundToPlay = GameClientRandomValue(0, numSamples - 1);
	char name[_MAX_PATH];

	if (soundToPlay < regularSamples) {
		strcpy(name, speechInfo->m_dialogFiles[soundToPlay].str());
	}	else if (soundToPlay < (regularSamples + eveningSamples)) {
		strcpy(name, speechInfo->m_dialogFilesEvening[soundToPlay - regularSamples].str());
	} else if (soundToPlay < (regularSamples + eveningSamples + morningSamples)) {
		strcpy(name, speechInfo->m_dialogFilesMorning[soundToPlay - regularSamples - eveningSamples].str());
	} else if (soundToPlay < numSamples) {
		strcpy(name, speechInfo->m_dialogFilesNight[soundToPlay - regularSamples - eveningSamples - morningSamples].str());
	}	else {
		return AsciiString::TheEmptyString;
	}


	
	// @todo this should all go into a function "generateLocalizedSoundName"
	Bool localized = FALSE;
	if ( name[0] == '$' )
	{
		localized = TRUE;
		char nameTemp[_MAX_PATH];
		strcpy(nameTemp, name + 1);
		strcpy(name, nameTemp);
	}

	if ( name[0] == 0 )
	{
		return NULL;
	}

	AsciiString path;
	char *localDir = "";

	if ( localized )
	{
		localDir = "english\\";
	}

	path.format( "data\\audio\\sounds\\%s%s.wav", localDir, name);
	return path;
}

//============================================================================
//============================================================================
//============================================================================
// Speaker Code
//============================================================================
//============================================================================
//============================================================================

//============================================================================
// Speaker::Speaker
//============================================================================

Speaker::Speaker()
: m_delayTime(300),
	m_priority(0),
	m_currentSpeech(NULL),
	m_file(NULL),
	m_stream(NULL)
{
	m_name.set("Unnamed");
}

//============================================================================
// Speaker::~Speaker
//============================================================================

Speaker::~Speaker()
{
	deinit();
}

//============================================================================
// Speaker::init 
//============================================================================

void Speaker::init( char *name, Int priority, Int btime, Int delay )
{
	deinit();
	m_name.set( name );
	m_priority = priority;
	m_delayTime = delay;
	m_stream = AudioStreamerCreate( TheAudio->device(), btime );

	if ( m_stream )
	{
		AudioStreamerSetName ( m_stream, name );
		AudioStreamerSetAttribs ( m_stream, m_manager->getMasterAttribs());
		AudioStreamerSetFadeAttribs ( m_stream, m_manager->getMasterFadeAttribs() );
		AudioStreamerSetPriority ( m_stream, m_priority );
	}
}

//============================================================================
// Speaker::deinit
//============================================================================

void Speaker::deinit( void )
{
	stop();

	if ( m_stream != NULL )
	{
		AudioStreamerDestroy ( m_stream );
		m_stream = NULL;
	}

}

//============================================================================
// Speaker::firstItem
//============================================================================

SpeechItem* Speaker::firstItem( TimeStamp now )
{
	LListNode *node = m_pending.firstNode();
	SpeechItem *item = NULL;

	while ( node )
	{
		item = (SpeechItem*) node->item();

		if ( !now || !item->timeout || item->timeout >= now )
		{
			break;
		}

		// this dialog item has timed out so remove it
		node = item->nd.next();
		item->nd.remove();
		delete item;
		item = NULL;
	}

	return item;

}

//============================================================================
// Speaker::nextItem 
//============================================================================

SpeechItem* Speaker::nextItem ( SpeechItem *item, TimeStamp now )
{
	LListNode *node = NULL;
	SpeechItem *next = NULL;

	if ( item )
	{
		node = item->nd.next();
	}

	while ( node )
	{
		next = (SpeechItem*) node->item();

		if ( !now || !next->timeout || next->timeout >= now )
		{
			break;
		}

		// this dialog item has timed out so remove it
		node = next->nd.next();
		next->nd.remove();
		delete next;
		next = NULL;
	}

	return next;

}

//============================================================================
// Speaker::findItem
//============================================================================

SpeechItem* Speaker::findItem( Speech *speech )
{
	TimeStamp now = AudioGetTime();

	SpeechItem *item = firstItem ( now );

	while ( item )
	{
		if ( item->speech == speech )
		{
			break;
		}
		item  = nextItem( item, now );
	}

	return item;
}

//============================================================================
// Speaker::flush
//============================================================================

void Speaker::flush( void )
{
	SpeechItem *item;

	while ( ( item = firstItem()) != 0 )
	{
		item->nd.remove();
		delete item;
	}
}

//============================================================================
// Speaker::destroy
//============================================================================

void Speaker::destroy( void )
{
	if ( m_manager )
	{
		m_manager->removeSpeaker( this );
	}
	delete this;
}

//============================================================================
// Speaker::say 
//============================================================================

void Speaker::say (	char *speechName, Int priority,	Int timeout, Int interrupt)
{
	Speech *speech = TheAudio->Speeches->getSpeech( speechName );
	say( speech, priority, timeout, interrupt );
}

//============================================================================
// Speaker::say 
//============================================================================

void Speaker::say (	Speech *speech, Int priority,	Int timeout, Int interrupt)
{
	
	if ( speech == NULL || !m_stream || !speech->valid || !m_manager->isOn())
	{
		return;
	}

	if ( saying() == speech || isGoingToSay( speech ))
	{
		return ;
	}

	SpeechItem *item = NEW SpeechItem;	// poolify

	if ( item == NULL )
	{
		return;
	}
	if ( timeout == 0 )
	{
		timeout = speech->timeout;
	}

	if ( priority == 0 )
	{
		priority = speech->priority;
	}

	if ( interrupt == 0 )
	{
		interrupt = speech->interrupt;
	}

	item->speech = speech;
	item->priority = priority;

	if ( m_paused )
	{
		item->flags = SpeechItem::Flags((Int)item->flags | (Int)SpeechItem::PAUSED);
	}

	if ( timeout )
	{
		item->timeout = m_paused ? MSECONDS(timeout) : AudioGetTime() + MSECONDS(timeout);
	}

	item->nd.setItem( item ) ;
	item->nd.setPriority( item->priority);
	m_pending.add( &item->nd );

	if ( interrupt && m_currentSpeech )
	{
			if ( m_currentSpeech->priority < item->priority )
			{
				AudioStreamerStop ( m_stream );
				m_delay = 0;
			}
	}
}

//============================================================================
// Speaker::update
//============================================================================

void Speaker::update( void )
{

	if ( m_stream == NULL )
	{
		stop();
		return;
	}

	if ( !AudioStreamerActive ( m_stream ))
	{
		if ( m_currentSpeech )
		{
			delete m_currentSpeech;
			m_currentSpeech = NULL;
		}
	}

	TimeStamp now = AudioGetTime();

	if ( !m_paused && m_currentSpeech == NULL && (AudioStreamerEndTimeStamp ( m_stream ) + m_delay) <= now )
	{
		SpeechItem *next = firstItem ( now );

		if ( next )
		{
			// ok we can start the next one
			next->nd.remove();
			m_currentSpeech = next;

			AudioStreamerSetMaxVolume ( m_stream, TheAudio->convertRealVolume(next->speech->volume) );
			AudioStreamerSetVolume ( m_stream, TheAudio->convertRealVolume(next->speech->volume) );

			AudioStreamerOpen( m_stream, getNextFilenameFromSpeech(next->speech).str());
			if (AudioStreamerStart( m_stream ) ) {
				m_delay = m_delayTime;
			}

///@todo write an INI compatible dialog player
/*
// REMOVE_GDF --> this must be converted to INI or something
			GDI_Asset *asset = TheGameData->openAsset( next->speech->handle );
			
			if( asset )
			{
				if ( (m_file = asset->open()))
				{
					if ( AudioStreamerOpen( m_stream, m_file ))
					{
						if ( AudioStreamerStart ( m_stream ) )
						{
							m_delay = m_delayTime;
						}
					}
				}
				asset->close();
			}
*/
		}
	}
}

//============================================================================
// Speaker::setPriority 
//============================================================================

void Speaker::setPriority ( Int priority )
{
	m_priority = priority;

	if ( m_manager )
	{
		m_manager->removeSpeaker( this );
		m_manager->addSpeaker( this );
	}
	if ( m_stream )
	{
		AudioStreamerSetPriority( m_stream, priority );
	}
}

//============================================================================
// Speaker::getPriority 
//============================================================================

Int Speaker::getPriority ( void )
{
	return m_priority;
}

//============================================================================
// Speaker::setDelay 
//============================================================================

void Speaker::setDelay ( Int delay )
{
	m_delay = MSECONDS(delay);
}

//============================================================================
// Speaker::getDelay 
//============================================================================

Int Speaker::getDelay ( void )
{
	return IN_MSECONDS(m_delay);
}

//============================================================================
// Speaker::setBuffering 
//============================================================================

void Speaker::setBuffering ( Int buffer_time )
{
	DEBUG_ASSERTCRASH( FALSE, ("setBuffering not implemented"));
}

//===============================
// Speaker::getBuffering 
//===============================

Int Speaker::getBuffering ( void )
{
	DEBUG_ASSERTCRASH( FALSE, ("setBuffering not implemented"));
	return 0;
}

//============================================================================
// Speaker::pause 
//============================================================================

void Speaker::pause ( void )
{

	if ( !m_paused )
	{
		if ( m_stream )
		{
			AudioStreamerPause ( m_stream );
		}

		TimeStamp now = AudioGetTime();

		SpeechItem *item = firstItem( now );

		while ( item )
		{
			if ( !( item->flags & SpeechItem::PAUSED ))
			{
				item->flags = SpeechItem::Flags( (Int) item->flags | (Int) SpeechItem::PAUSED);
				if ( item->timeout )
				{
					item->timeout = item->timeout - now;
				}
			}
			item = nextItem ( item, now );
		}
	}

	m_paused++;
}

//============================================================================
// Speaker::resume 
//============================================================================

void Speaker::resume ( void )
{
	if ( m_paused == 1 )
	{
		TimeStamp now = AudioGetTime();

		SpeechItem *item = firstItem ( now );
	
		while ( item )
		{
			if ( ( item->flags & SpeechItem::PAUSED ))
			{
				item->flags = SpeechItem::Flags( (Int) item->flags & ~((Int) SpeechItem::PAUSED));
				if ( item->timeout )
				{
					item->timeout = now + item->timeout;
				}
			}
			item = nextItem ( item, now );
		}

		if ( m_stream )
		{
			AudioStreamerResume ( m_stream );
		}
	}

	m_paused--;

	if ( m_paused < 0 )
	{
		m_paused = 0;
	}
}

//============================================================================
// Speaker::stop 
//============================================================================

void Speaker::stop ( void )
{

	if ( m_stream )
	{
		AudioStreamerClose( m_stream );
	}

	if ( m_file )
	{
		m_file->close();
		m_file = NULL;
	}

	flush();
	if ( m_currentSpeech )
	{
		delete m_currentSpeech;
		m_currentSpeech = NULL;
	}
}

//============================================================================
// Speaker::cancel 
//============================================================================

void Speaker::cancel ( Speech *speech )
{
	SpeechItem *item;

	while ( ( item = findItem ( speech )) != 0 )
	{
		item->nd.remove();
		delete item;
	}

}

//============================================================================
// Speaker::hasSaid 
//============================================================================

Bool Speaker::hasSaid ( Speech *speech )
{
	return findItem ( speech ) == NULL && ( m_currentSpeech == NULL || m_currentSpeech->speech != speech);
}

//============================================================================
// Speaker::isGoingToSay 
//============================================================================

Bool Speaker::isGoingToSay ( Speech *speech )
{
	return findItem ( speech ) != NULL;
}

//============================================================================
// Speaker::isTalking 
//============================================================================

Bool Speaker::isTalking ( void )
{
	return m_currentSpeech != NULL;
}

//============================================================================
// Speaker::saying 
//============================================================================

Speech*	Speaker::saying ( void )
{
	if ( m_currentSpeech )
	{
		return m_currentSpeech->speech;
	}

	return NULL;
}


static void speechFromInfo(SpeechInfo *pInSpeechInfo, Speech *pOutSpeech)
{
	if (!pInSpeechInfo || !pOutSpeech) {
		return;
	}
	
	// TBD: Do we need these fields:
	// index? info?

	pOutSpeech->name = pInSpeechInfo->m_dialogEvent;
	pOutSpeech->id = (ID) 1;	
	pOutSpeech->volume = pInSpeechInfo->m_volume;
	pOutSpeech->timeout = 65000;
	pOutSpeech->interrupt = pInSpeechInfo->m_interruptable;
	pOutSpeech->valid = true;
	pOutSpeech->priority = pInSpeechInfo->m_priority;
	
	// Some housekeeping for pInSpeechInfo
	pInSpeechInfo->m_internalPlayCount = 0;

	pOutSpeech->info = *pInSpeechInfo;
	

}

static AsciiString getNextFilenameFromSpeech(Speech *pSpeechToPlay)
{
	AsciiString returnString;
	if (!pSpeechToPlay) {
		return returnString;
	}
	
	++pSpeechToPlay->info.m_internalPlayCount;

	Int indexToPlay = 0;
	if (pSpeechToPlay->info.m_sequentialStartIndex < 0) {
		indexToPlay = GameClientRandomValue(pSpeechToPlay->info.m_randomStartIndex,
																				pSpeechToPlay->info.m_dialogFiles.size() - 1);
	}

	if (pSpeechToPlay->info.m_dialogFiles.size() > 0) {
		returnString = AsciiString(BASE_DLG_DIR);
		returnString.concat('\\');
		returnString.concat(pSpeechToPlay->info.m_dialogFiles[indexToPlay]);
		returnString.concat('.');
		returnString.concat(BASE_DLG_EXT);
	}
	

	return returnString;
}
