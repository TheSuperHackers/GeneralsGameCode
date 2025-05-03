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

// GameClient/Eva.cpp /////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "GameClient/Eva.h"

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameLogic/GameLogic.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-------------------------------------------------------------------------------------------------
const char *TheEvaMessageNames[] = 
{
	"LOWPOWER",
	"INSUFFICIENTFUNDS",
	"SUPERWEAPONDETECTED_PARTICLECANNON",
	"SUPERWEAPONDETECTED_NUKE",
	"SUPERWEAPONDETECTED_SCUDSTORM",
	"SUPERWEAPONLAUNCHED_PARTICLECANNON",
	"SUPERWEAPONLAUNCHED_NUKE",
	"SUPERWEAPONLAUNCHED_SCUDSTORM",
	"BUILDINGLOST",
	"BASEUNDERATTACK",
	"ALLYUNDERATTACK",
	"BEACONDETECTED",
	"UNITLOST",
	"GENERALLEVELUP",
	"VEHICLESTOLEN",
	"BUILDINGSTOLEN",
	"CASHSTOLEN",
	"UPGRADECOMPLETE",
	"BUILDINGBEINGSTOLEN",
	"EVA_INVALID",
};

//------------------------------------------------------------------------------ INI::parseEvaEvent
void INI::parseEvaEvent( INI* ini )
{
	AsciiString name;

	// read the name
	const char* c = ini->getNextToken();
	name.set( c );	
	
	EvaCheckInfo *check = TheEva->newEvaCheckInfo( name );
	if (!check) {
		// could be null because it already exists.
		return;
	}

	// parse the ini definition
	ini->initFromINI( check, check->getFieldParse() );
}

//----------------------------------------------------------------------------------- EvaSideSounds
static void parseSideSoundsList( INI *ini, void *instance, void *store, const void* userData )
{
	std::vector<EvaSideSounds> *sounds = (std::vector<EvaSideSounds>*) store;
	EvaSideSounds newSounds;

	ini->initFromINI( &newSounds, newSounds.getFieldParse() );

	// This could be made more efficient, but to be honest, it shouldn't be that slow.
	sounds->push_back(newSounds);
}

//----------------------------------------------------------------------------------- EvaSideSounds
const FieldParse EvaSideSounds::s_evaSideSounds[] =
{
	{ "Side",									INI::parseAsciiString,					NULL,			offsetof( EvaSideSounds, m_side) },
	{ "Sounds",								INI::parseSoundsList,						NULL,			offsetof( EvaSideSounds, m_soundNames) },
	{ 0, 0, 0, 0 },
};

//------------------------------------------------------------------------------------ EvaCheckInfo
EvaCheckInfo::EvaCheckInfo() :
	m_message(EVA_COUNT),
	m_priority(0), // lowest of all priorities
	m_framesBetweenChecks(900),	// 30 seconds at 30 fps
	m_framesToExpire(150) // 5 seconds at 30 fps
{

	
}

//-------------------------------------------------------------------------------------------------
const FieldParse EvaCheckInfo::s_evaEventInfo[] = 
{
	{ "Priority",							INI::parseUnsignedInt,					NULL,			offsetof( EvaCheckInfo, m_priority ) },
	{ "TimeBetweenChecksMS",	INI::parseDurationUnsignedInt,	NULL,			offsetof( EvaCheckInfo, m_framesBetweenChecks ) },
	{ "ExpirationTimeMS",			INI::parseDurationUnsignedInt,	NULL,			offsetof( EvaCheckInfo, m_framesToExpire) },
	{ "SideSounds",						parseSideSoundsList,						NULL,			offsetof( EvaCheckInfo, m_evaSideSounds ) },
	{ 0, 0, 0, 0 },

};


//-------------------------------------------------------------------------------------------------
const ShouldPlayFunc Eva::s_shouldPlayFuncs[] = 
{
	Eva::shouldPlayLowPower,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,
	Eva::shouldPlayGenericHandler,	
	Eva::shouldPlayGenericHandler,	
	NULL,
};

//-------------------------------------------------------------------------------------------------
EvaCheck::EvaCheck() : 
	m_evaInfo(NULL), 
	m_triggeredOnFrame(TRIGGEREDON_NOT),
	m_timeForNextCheck(NEXT_CHECK_NOW), 
	m_alreadyPlayed(FALSE)
{

}

//-------------------------------------------------------------------------------------------------
Eva::Eva() : 
	m_localPlayer(NULL),
	m_previousBuildingCount(0),
	m_previousUnitCount(0),
	m_enabled(TRUE)
{

	for (Int i = 0; i < EVA_COUNT; ++i) {
		m_shouldPlay[i] = FALSE;
	}
}

//-------------------------------------------------------------------------------------------------
Eva::~Eva()
{
	EvaCheckInfoPtrVecIt it;
	for (it = m_allCheckInfos.begin(); it != m_allCheckInfos.end(); ++it) {
		if (*it)
			(*it)->deleteInstance();
	}
}

//-------------------------------------------------------------------------------------------------
void Eva::init()
{
	// parse the INI here, etc.
	INI ini;
	ini.load( AsciiString( "Data\\INI\\Eva.ini" ), INI_LOAD_OVERWRITE, NULL);
}

//-------------------------------------------------------------------------------------------------
void Eva::reset()
{
	m_previousUnitCount = 0;
	m_previousBuildingCount = 0;

	// remove all pending counters, etc, here.
	EvaCheckVecIt it;
	for (it = m_checks.begin(); it != m_checks.end(); /* empty */) {
		it = m_checks.erase(it);
	}

	// remove all things flagged as "need to play"
	for (Int i = 0; i < EVA_COUNT; ++i) {
		m_shouldPlay[i] = FALSE;
	}	

	// If we were previously disabled, re-enable ourselves.
	m_enabled = TRUE;
}

//-------------------------------------------------------------------------------------------------
void Eva::update()
{
	if (!m_enabled) {
		return;
	}

	m_localPlayer = ThePlayerList->getLocalPlayer();
	UnsignedInt frame = TheGameLogic->getFrame();

	// Don't update for the first few frames. This way, we don't have to deal with our initial power
	// being 0, etc.
	if (frame < 2) {
		return;
	}

	for (Int mesg = (Int)EVA_FIRST; mesg < (Int)EVA_COUNT; ++mesg) {
		if (isTimeForCheck((EvaMessage)mesg, frame)) {
			if (messageShouldPlay((EvaMessage)mesg, frame)) {
				playMessage((EvaMessage)mesg, frame);
			}
		}
	}

	processPlayingMessages(frame);	
	m_localPlayer = NULL;

	// Reset all of the flags that have been set to true that haven't actually been probed, because
	// they will need to trigger again to be valid messages.
	for (Int i = EVA_FIRST; i < EVA_COUNT; ++i) {
		m_shouldPlay[i] = FALSE;
	}
}

//-------------------------------------------------------------------------------------------------
EvaMessage Eva::nameToMessage(const AsciiString& name)
{
	for (Int i = EVA_FIRST; i < EVA_COUNT; ++i) {
		if (name.compareNoCase(TheEvaMessageNames[i]) == 0) {
			return (EvaMessage) i;
		}
	}

	DEBUG_CRASH(("Invalid requested Eva message translation :%s: jkmcd", name.str()));
	return EVA_COUNT;
}

//-------------------------------------------------------------------------------------------------
AsciiString Eva::messageToName(EvaMessage message)
{
	if (message >= EVA_FIRST && message < EVA_COUNT)
		return TheEvaMessageNames[message];

	DEBUG_CRASH(("Invalid requested Eva message translation. jkmcd"));
	return AsciiString::TheEmptyString;
}

//-------------------------------------------------------------------------------------------------
EvaCheckInfo *Eva::newEvaCheckInfo(AsciiString name)
{
	EvaMessage mesg = nameToMessage(name);

	// Only return a new one if there isn't an existing one.
	EvaCheckInfoPtrVecIt it;
	for (it = m_allCheckInfos.begin(); it != m_allCheckInfos.end(); ++it) {
		if (*it && (*it)->m_message == mesg)
			return NULL;
	}

	EvaCheckInfo *checkInfo = newInstance(EvaCheckInfo);
	m_allCheckInfos.push_back(checkInfo);
	checkInfo->m_message = mesg;
	return checkInfo;
}

//-------------------------------------------------------------------------------------------------
const EvaCheckInfo *Eva::getEvaCheckInfo(AsciiString name)
{
	EvaMessage mesg = nameToMessage(name);

	// Only return a new one if there isn't an existing one.
	EvaCheckInfoPtrVecIt it;
	for (it = m_allCheckInfos.begin(); it != m_allCheckInfos.end(); ++it) {
		if (*it && (*it)->m_message == mesg)
			return *it;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------------------
void Eva::setShouldPlay(EvaMessage messageToPlay)
{
	m_shouldPlay[messageToPlay] = TRUE;
}

//-------------------------------------------------------------------------------------------------
void Eva::setEvaEnabled(Bool enabled)
{
	// clear out any waiting messages.
	for (Int i = EVA_FIRST; i < EVA_COUNT; ++i) {
		m_shouldPlay[i] = FALSE;
	}
	m_enabled = enabled; 
}

//-------------------------------------------------------------------------------------------------
Bool Eva::isTimeForCheck(EvaMessage messageToTest, UnsignedInt currentFrame) const
{
	EvaCheckVec::const_iterator it;
	for (it = m_checks.begin(); it != m_checks.end(); ++it) {
		if (it->m_evaInfo->m_message == messageToTest) {
			return FALSE;
		}
	}

	return TRUE;
}

//-------------------------------------------------------------------------------------------------
Bool Eva::messageShouldPlay(EvaMessage messageToTest, UnsignedInt currentFrame) const
{
	if (m_localPlayer == NULL) {
		return FALSE;
	}

	m_messageBeingTested = messageToTest;
	return s_shouldPlayFuncs[messageToTest](m_localPlayer);
}

//-------------------------------------------------------------------------------------------------
Bool Eva::shouldPlayLowPower( Player *localPlayer )
{

	// @todo make eva sensitive to whether player can do anything about it...
	// "Low power, Low power, Low power, yadda yadda yadda..."
	//const ThingTemplate *chinaReactorTemplate = findTemplate(;
	//const ThingTemplate *americanReactorTemplate;
	//if ( chinaReactorTemplate && americanReactorTemplate )
	//{
	//	if ( ! (localPlayer->canBuild(chinaReactorTemplate) || localPlayer->canBuild(americanReactorTemplate)) )
	//		return FALSE
	//}


	return !localPlayer->getEnergy()->hasSufficientPower();
}

//-------------------------------------------------------------------------------------------------
Bool Eva::shouldPlayGenericHandler( Player * )
{
	if (TheEva->m_shouldPlay[TheEva->m_messageBeingTested]) {
		TheEva->m_shouldPlay[TheEva->m_messageBeingTested] = FALSE;
		return TRUE;
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
void Eva::playMessage(EvaMessage messageToTest, UnsignedInt currentFrame)
{
	EvaCheck check;
	check.m_evaInfo = getEvaCheckInfo(Eva::messageToName(messageToTest));
	if (!check.m_evaInfo) {
		return;
	}

	check.m_timeForNextCheck = currentFrame + check.m_evaInfo->m_framesBetweenChecks;
	check.m_triggeredOnFrame = currentFrame;
	check.m_alreadyPlayed = FALSE;

	m_checks.push_back(check);
}

//-------------------------------------------------------------------------------------------------
void Eva::processPlayingMessages(UnsignedInt currentFrame)
{
	// First pass, remove all the objects that can check after this frame.
	EvaCheckVecIt it;
	for (it = m_checks.begin(); it != m_checks.end(); /* empty */) {
		// These are requests that will be available next frame because they've played.
		if (it->m_timeForNextCheck <= currentFrame + 1 && it->m_alreadyPlayed) {
			it = m_checks.erase(it);
			continue;
		}

		// These are requests that never got a chance to play and have since expired.
		if (it->m_triggeredOnFrame + it->m_evaInfo->m_framesToExpire <= currentFrame && !it->m_alreadyPlayed) {
			it = m_checks.erase(it);
			continue;
		}

		++it;
	}

	// It's possible, although unlikely, that we removed everything in the list.
	if (m_checks.begin() == m_checks.end()) {
		return;
	}

	// If we're currently playing some audio, we're done.
	if (m_evaSpeech.isCurrentlyPlaying()) {
		return;
	}

	// Okay. No one is currently playing anything, so lets find an event and trigger it.
	EvaCheckVecIt storedIt = m_checks.end();

	UnsignedInt highestPriority = 0;
	for (it = m_checks.begin(); it != m_checks.end(); ++it) {
		if (it->m_evaInfo->m_priority > highestPriority && !it->m_alreadyPlayed) {
			storedIt = it;
			highestPriority = it->m_evaInfo->m_priority;
		}
	}

	// There wasn't anything waiting to play.
	if (storedIt == m_checks.end()) {
		return;
	}

	// We've got a winner!
	AsciiString side = ThePlayerList->getLocalPlayer()->getSide();
	Int numSides = storedIt->m_evaInfo->m_evaSideSounds.size();

	for (Int i = 0; i < numSides; ++i) {
		if (side.compareNoCase(storedIt->m_evaInfo->m_evaSideSounds[i].m_side) == 0) {
			// Its this one.
			if (storedIt->m_evaInfo->m_evaSideSounds[i].m_soundNames.size() > 0) {
				Int soundToPlay = GameClientRandomValue(0, storedIt->m_evaInfo->m_evaSideSounds[i].m_soundNames.size() - 1);
				m_evaSpeech.setEventName(storedIt->m_evaInfo->m_evaSideSounds[i].m_soundNames[soundToPlay]);
			} else {
				// clear it.
				m_evaSpeech.setEventName(AsciiString::TheEmptyString);
			}
		}
	}

	// Update the entry
	storedIt->m_alreadyPlayed = true;
	storedIt->m_timeForNextCheck = currentFrame + storedIt->m_evaInfo->m_framesBetweenChecks;
	
	// Now that we correctly filter messages, we need to set the player index for who should hear the
	// sound to the local player.
	m_evaSpeech.setPlayerIndex(m_localPlayer->getPlayerIndex());

	m_evaSpeech.setPlayingHandle(TheAudio->addAudioEvent(&m_evaSpeech));
}

//-------------------------------------------------------------------------------------------------
Eva *TheEva = NULL;

