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

// FILE: ProximityCaptureUpdate.cpp ///////////////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, December 2002
// Desc:   Reacts when an enemy is within range
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/PerfTimer.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/Eva.h"
#include "Common/Radar.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/ProximityCaptureUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/GameLogic.h"
#include "Common/GameUtility.h"
#include "Common/GameAudio.h"
#include "Common/AudioEventRTS.h"
#include "GameClient/FXList.h"
#include "Common/MiscAudio.h"
//#include "GameLogic/AI.h"
//#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ContainModule.h"
#include <array>

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ProximityCaptureUpdate::ProximityCaptureUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData ),
  m_capturingPlayer(-1),
	m_captureProgress(1.0),
	m_isContested(false)
{
	// We always start off as captured by the original owner (including neutral)
	m_capturingPlayer = getObject()->getControllingPlayer()->getPlayerIndex();

	// bias a random amount so everyone doesn't spike at once
	UnsignedInt initialDelay = GameLogicRandomValue(0, getProximityCaptureUpdateModuleData()->m_captureTickDelay);
	setWakeFrame(getObject(), UPDATE_SLEEP(initialDelay));
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ProximityCaptureUpdate::~ProximityCaptureUpdate( void )
{
}

//-----------------
// Helper functions
// ----------------
Bool arePlayersAllied(Int player1, Int player2) {
	if (player1 == player2 || player1 < 0 || player1 >= MAX_PLAYER_COUNT || player2 < 0 || player2 >= MAX_PLAYER_COUNT)
		return FALSE;

	return ThePlayerList->getNthPlayer(player1)->getRelationship(ThePlayerList->getNthPlayer(player2)->getDefaultTeam()) == ALLIES;
}


//-------------------------------------------------------------------------------------------------
struct PlayerResult {
	Int playerId = -1;
	Real totalValue = 0.0f;

	bool operator<(const PlayerResult& other) const {
		return totalValue > other.totalValue; // Descending sort
	}
};
//-------------------------------------------------------------------------------------------------
// Look around us for units
//-------------------------------------------------------------------------------------------------
Int ProximityCaptureUpdate::checkDominantPlayer( void )
{
	Object* me = getObject();
	const ProximityCaptureUpdateModuleData* data = getProximityCaptureUpdateModuleData();

	PartitionFilterSameMapStatus filterMapStatus(me);
	PartitionFilterAlive filterAlive;
  //PartitionFilterRejectByKindOf filterRejectKindof(data->m_forbiddenKindOf);
	//Note: we don't filter for accepted KindOfs her because we want to differentiate between ALL and ANY match cases

	PartitionFilter* filters[] = { &filterAlive, &filterMapStatus, NULL };

	// scan objects in our region
	ObjectIterator* iter = ThePartitionManager->iterateObjectsInRange(me->getPosition(),
		data->m_captureRadius,
		FROM_CENTER_2D,
		filters);

	MemoryPoolObjectHolder hold(iter);

	// Get total value per player
	//constexpr size_t SIZE = MAX_PLAYER_COUNT;
	std::array<Real, MAX_PLAYER_COUNT> playerTotals{};
	playerTotals.fill(0.0f);

	for (Object* currentObj = iter->first(); currentObj != NULL; currentObj = iter->next())
	{
		bool match = FALSE;
		if (!data->m_requiresAllKindOfs)
			match = currentObj->isAnyKindOf(data->m_requiredKindOf) && !currentObj->isAnyKindOf(data->m_forbiddenKindOf);
		else
			match = currentObj->isKindOfMulti(data->m_requiredKindOf, data->m_forbiddenKindOf);

		match &= (data->m_isCountAirborne || !currentObj->isAirborneTarget());

		match &= (currentObj != me);
		match &= (!currentObj->isNeutralControlled());

		if (!match)
			continue;

		PlayerIndex pId = currentObj->getControllingPlayer()->getPlayerIndex();
		if (pId > PLAYER_INDEX_INVALID) {
			playerTotals[pId] += getValueForUnit(currentObj);
		}
	}

	// Find player with highest influence
	PlayerResult bestResult;
	PlayerResult secondBestResult;
	//result.totalValue = -std::numeric_limits<float>::infinity();

	for (int i = 0; i < MAX_PLAYER_COUNT; i++) {

		if (playerTotals[i] > bestResult.totalValue) {

			if (bestResult.playerId != -1) {
				secondBestResult.totalValue = bestResult.totalValue;
				secondBestResult.playerId = bestResult.playerId;
			}

			bestResult.totalValue = playerTotals[i];
			bestResult.playerId = i;
		}
		else if (playerTotals[i] > secondBestResult.totalValue) {
			secondBestResult.totalValue = playerTotals[i];
			secondBestResult.playerId = i;
		}
	}

	if (bestResult.totalValue <= 0) {
		// No units at all. No player dominant, but not contested.
		return -1;
	}

	if (!arePlayersAllied(bestResult.playerId, secondBestResult.playerId)) {
		// DEBUG_LOG((">>> ProximityCaptureUpdate:: Is Contested? - best = %f, second = %f", bestResult.totalValue, secondBestResult.totalValue));
		m_isContested = bestResult.totalValue <= (secondBestResult.totalValue + data->m_unitValueContentionDelta);
	}

	if (!m_isContested) {
		// Is the current capturing player an ally?
		if (arePlayersAllied(bestResult.playerId, m_capturingPlayer)) {
			if (playerTotals[m_capturingPlayer] > 0) { // Ally still has units in radius
				return m_capturingPlayer;
			}
		}
		return bestResult.playerId;
	}
	else {
		return -1;
	}

}

//-------------------------------------------------------------------------------------------------

Real ProximityCaptureUpdate::getValueForUnit(const Object* obj) const
{
	const ProximityCaptureUpdateModuleData* data = getProximityCaptureUpdateModuleData();
	Real value = data->m_unitValueCountFactor;

	//if (data->m_unitValueBuildCostFactor > 0.0f) {
	//	value += obj->getTemplate()->calcCostToBuild();
	//}

	return value;
}


//-------------------------------------------------------------------------------------------------
void ProximityCaptureUpdate::handleCaptureProgress(Int dominantPlayer)
{

	Real captureProgressPrev = m_captureProgress;

	if (m_isContested) {
		// No update while contested
		return;
	}

	const ProximityCaptureUpdateModuleData* data = getProximityCaptureUpdateModuleData();
	Object* me = getObject();
	PlayerIndex owningPlayer = me->getControllingPlayer()->getPlayerIndex();

	// If players are allied, we treat it as no dominant player
	if (arePlayersAllied(dominantPlayer, m_capturingPlayer)) {
		dominantPlayer = -1;
	}

	DEBUG_ASSERTCRASH(m_capturingPlayer != -1, ("The capturing player should always have a valid index"));

	// Currently fully captured
	if (m_captureProgress >= 1.0f) {
		// No capture process
		if (dominantPlayer == -1 || dominantPlayer == owningPlayer)
			return;

		if (dominantPlayer != m_capturingPlayer) {
			startUncap(dominantPlayer);
		}
	}

	if (dominantPlayer == m_capturingPlayer)
	{  // Capture is in progress
		m_captureProgress += data->m_captureRate;
	}
	else if (dominantPlayer == -1 && m_capturingPlayer == owningPlayer)
	{ // Recover capture status
		if (data->m_recoverRate >= 0.0f)
			m_captureProgress += data->m_recoverRate;
		else
			m_captureProgress += data->m_captureRate;
	}
	else
	{ // Uncap is in progress
		if (me->isNeutralControlled() && data->m_uncapRateNeutral > 0.0f) {
			m_captureProgress -= data->m_uncapRateNeutral;
		}
		else {
			m_captureProgress -= data->m_uncapRate;
		}
	}

	if (m_captureProgress <= 0) {
		//DEBUG_ASSERTCRASH(dominantPlayer != -1, ("Can't uncap without a dominant player."));
		m_captureProgress = 0;

		// Finished uncapping, reset capturing player
		if (dominantPlayer != -1) {
			finishUncap(dominantPlayer);
			// Change ownership to neutral
			m_capturingPlayer = dominantPlayer;
			startCapture(m_capturingPlayer);
		}
		else
		{  // abort capture, back to the owner (neutral)
			m_capturingPlayer = owningPlayer;
		}
	}
	else if (m_captureProgress >= 1)
	{
		m_captureProgress = 1.0;
		if (m_capturingPlayer == owningPlayer) {
			// finished recovering, nothing to do here really.
		}
		else {
			// finished capturing, change ownership
			finishCapture(m_capturingPlayer);
		}
	}

	m_currentProgressRate = m_captureProgress - captureProgressPrev;
}
// ------------------------------------------------------------------------------------------------
// A player starts capturing (i.e. filling up his progress)
void ProximityCaptureUpdate::startCapture(Int playerId)
{
	DEBUG_ASSERTLOG(getObject()->isNeutralControlled(), ("ProximityCaptureUpdate::startCapture: Warning, current owner should be neutral!"));

	FXList::doFXObj(getProximityCaptureUpdateModuleData()->m_startCaptureFX, getObject());

	m_capturePingDelay = 0;
}
// ------------------------------------------------------------------------------------------------
// A player has finished capturing (i.e. got ownership)
void ProximityCaptureUpdate::finishCapture(Int playerId)
{
	//TODO: Booby trap support maybe later
	// if (getObject()->checkAndDetonateBoobyTrap(getObject())) // We need to store at least one unit of the dominant player ?!

	Object* me = getObject();
	const ProximityCaptureUpdateModuleData* data = getProximityCaptureUpdateModuleData();

	// Just in case we are capturing a building which is already garrisoned by other
	ContainModuleInterface* contain = me->getContain();
	if (contain && contain->isGarrisonable())
	{
		contain->removeAllContained(TRUE);
	}

	DEBUG_ASSERTLOG(me->isNeutralControlled(), ("ProximityCaptureUpdate::finishCapture: Warning, current owner should be neutral!"));

	FXList::doFXObj(data->m_finishCaptureFX, getObject());

	Player* newOwner = ThePlayerList->getNthPlayer(playerId);
	if (newOwner) {
		me->defect(newOwner->getDefaultTeam(), 1); // one frame of flash!

		newOwner->getAcademyStats()->recordBuildingCapture();

		if (data->m_skillPointsForCapture > 0)
		{
			newOwner->addSkillPoints(data->m_skillPointsForCapture);
		}

		if (newOwner == rts::getObservedOrLocalPlayer())
			TheRadar->tryEvent(RADAR_EVENT_INFORMATION, me->getPosition());
	}
}
// ------------------------------------------------------------------------------------------------
// A player starts removing the progress of another
void ProximityCaptureUpdate::startUncap(Int playerId) {
	Object* me = getObject();
	//Warn the victim so he might have a chance to react!
	if (me && me->isLocallyViewed())
	{
		TheEva->setShouldPlay(EVA_BuildingBeingStolen);
	}
	TheRadar->tryInfiltrationEvent(me);

	FXList::doFXObj(getProximityCaptureUpdateModuleData()->m_startUncapFX, getObject());

	m_capturePingDelay = 0;
}
// ------------------------------------------------------------------------------------------------
// A player has finished neutralizing
void ProximityCaptureUpdate::finishUncap(Int playerId) {
	Object* me = getObject();

	//Play the "building stolen" EVA event if the local player is the victim!
	if (me && me->isLocallyViewed())
	{
		TheEva->setShouldPlay(EVA_BuildingStolen);
	}

	me->defect(ThePlayerList->getNeutralPlayer()->getDefaultTeam(), 1); // one frame of flash!
}
// ------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime ProximityCaptureUpdate::update()
{
	const ProximityCaptureUpdateModuleData* data = getProximityCaptureUpdateModuleData();

	Int dominantPlayer = checkDominantPlayer();

	// DEBUG_LOG((">>> ProximityCaptureUpdate::update - dominantPlayer = %d", dominantPlayer));

	handleCaptureProgress(dominantPlayer);

	handleFlashEffects(dominantPlayer);

	m_lastTickFrame = TheGameLogic->getFrame();

	//m_dominantPlayerPrev = dominantPlayer;

	return UPDATE_SLEEP(data->m_captureTickDelay);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool ProximityCaptureUpdate::getProgressBarInfo(Real& progress, Int& type, RGBAColorInt& color, RGBAColorInt& colorBG)
{
	const ProximityCaptureUpdateModuleData* data = getProximityCaptureUpdateModuleData();
	if (!data->m_showProgressBar)
		return false;

	if (m_captureProgress >= 1.0)
		return false;

	progress = getCaptureProgressInterp();

	if (m_isContested)
		colorBG = { 255, 255, 0, 255 };
	else
		colorBG = { 0, 0, 0, 255 };

	RGBColor col;
	col.setFromInt(ThePlayerList->getNthPlayer(m_capturingPlayer)->getPlayerColor());

	color = { (byte)(col.red * 255.0),(byte)(col.green * 255.0), (byte)(col.blue * 255.0), 255 };


	return true;
}


//-------------------------------------------------------------------------------------------------
void ProximityCaptureUpdate::handleFlashEffects(Int dominantPlayer)
{
	if (m_capturePingDelay-- > 0)
		return;

	Object* me = getObject();
	Drawable* draw = me->getDrawable();

	if (m_isContested) {
		FXList::doFXObj(getProximityCaptureUpdateModuleData()->m_capturePingContestedFX, getObject());
	} else if (m_captureProgress < 1.0 && dominantPlayer != -1 && dominantPlayer != me->getControllingPlayer()->getPlayerIndex() && dominantPlayer != ThePlayerList->getNeutralPlayer()->getPlayerIndex()) {
		if (draw) {
			RGBColor myHouseColor;
			myHouseColor.setFromInt(ThePlayerList->getNthPlayer(dominantPlayer)->getPlayerColor());

			Real saturation = TheGlobalData->m_selectionFlashSaturationFactor;
			draw->saturateRGB(myHouseColor, saturation);
			draw->flashAsSelected(&myHouseColor); //In MY house color, not his!

		}
		if (getProximityCaptureUpdateModuleData()->m_playDefectorPingSound) {
			AudioEventRTS defectorTimerSound = TheAudio->getMiscAudio()->m_defectorTimerTickSound;
			defectorTimerSound.setObjectID(me->getID());
			TheAudio->addAudioEvent(&defectorTimerSound);
		}

		FXList::doFXObj(getProximityCaptureUpdateModuleData()->m_capturePingFX, getObject());

	}

	m_capturePingDelay = getProximityCaptureUpdateModuleData()->m_capturePingInterval;
}

// ------------------------------------------------------------------------------------------------
Real ProximityCaptureUpdate::getCaptureProgressInterp()
{
	if (m_captureProgress > 1.0)
		return 1.0;
	if (m_captureProgress < 0.0)
		return 0.0;

	if (m_isContested || getProximityCaptureUpdateModuleData()->m_captureTickDelay == 0)
		return m_captureProgress;

	UnsignedInt frameDiff = TheGameLogic->getFrame() - m_lastTickFrame;
	Real progDiff = INT_TO_REAL(frameDiff) / INT_TO_REAL(getProximityCaptureUpdateModuleData()->m_captureTickDelay);

	Real frameShift = (0.5 / INT_TO_REAL(getProximityCaptureUpdateModuleData()->m_captureTickDelay)) * m_currentProgressRate;  // half a tick offset

	return MAX(0.0, MIN(m_captureProgress + m_currentProgressRate * progDiff - frameShift, 1.0));
}


// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ProximityCaptureUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ProximityCaptureUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	xfer->xferInt(&m_capturingPlayer);
	xfer->xferBool(&m_isContested);
	xfer->xferReal(&m_captureProgress);
	xfer->xferReal(&m_currentProgressRate);
	xfer->xferUnsignedInt(&m_lastTickFrame);
	xfer->xferUnsignedShort(&m_capturePingDelay);

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ProximityCaptureUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
