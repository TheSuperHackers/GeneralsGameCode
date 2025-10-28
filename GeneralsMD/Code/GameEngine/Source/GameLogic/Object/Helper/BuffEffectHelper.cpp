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

// FILE: BuffEffectHelper.h ////////////////////////////////////////////////////////////////////////
// Author: Andi W, Oct 25
// Desc:   Buff Effect Helper - Tracks active buffs
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/Xfer.h"

#include "GameLogic/Module/BuffEffectHelper.h"

#include "GameClient/Drawable.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BuffEffectHelper::BuffEffectHelper(Thing* thing, const ModuleData* modData) : ObjectHelper(thing, modData)
{
	m_buffEffects.clear();
	m_nextTickFrame = UnsignedInt(UPDATE_SLEEP_FOREVER);

	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BuffEffectHelper::~BuffEffectHelper(void)
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime BuffEffectHelper::update()
{
	// TODO: Are there any cases were buffs work on dead objects?
	// Also, should we even get here on a dead object?
	if (getObject()->isEffectivelyDead()) {
		return UPDATE_SLEEP_FOREVER;
	}

	UnsignedInt now = TheGameLogic->getFrame();

	// Loop over all active BuffEffectTrackers and check if they expired or need to do something

	UnsignedInt closestNextTickFrame = UnsignedInt(UPDATE_SLEEP_FOREVER);

	for (auto it = m_buffEffects.begin(); it != m_buffEffects.end(); ) {
		BuffEffectTracker& bet = *it;
		if (bet.m_frameToRemove >= now) {
			// remove buff effect
			// TODO: Handle dynamic "unstacking"
			bet.m_template->removeEffects(getObject());

			it = m_buffEffects.erase(it);
		}
		else {
			// TODO: Handle Continuous effects (DOTS)

			UnsignedInt nextTickFrame = bet.m_template->getNextTickFrame(bet.m_frameCreated, bet.m_frameToRemove);
			if (nextTickFrame < closestNextTickFrame)
				closestNextTickFrame = nextTickFrame;

			++it;
		}
	}

	return UpdateSleepTime(closestNextTickFrame);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BuffEffectHelper::applyBuff(const BuffTemplate* buffTemplate, Object* sourceObj, UnsignedInt duration)
{
	// Note: Should we check if the buff is allowed to be applied here? Or should that happen
	// before, wherever this method is called?

	UnsignedInt now = TheGameLogic->getFrame();

	// Setup BuffEffectTracker entry and add it to the list
	// --------------------------------
	BuffEffectTracker bet;
	bet.m_template = buffTemplate;
	bet.m_frameCreated = now;
	bet.m_frameToRemove = now + duration;
	bet.m_numStacks = 1;  //todo
	bet.m_isActive = TRUE; //todo
	if (sourceObj)
		bet.m_sourceID = sourceObj->getID();

	m_buffEffects.push_back(bet);
	// -------------------------------

	// Calculate our next wake frame
	// -------------------------------
	UnsignedInt nextTick = buffTemplate->getNextTickFrame(now, now + duration);
	if (nextTick < m_nextTickFrame) {
		m_nextTickFrame = nextTick;
		setWakeFrame(getObject(), UPDATE_SLEEP(m_nextTickFrame));
	}
	// -------------------------------

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
//void BuffEffectHelper::clearTempWeaponBonus()
//{
//	if (m_currentBonus != WEAPONBONUSCONDITION_INVALID)
//	{
//		getObject()->clearWeaponBonusCondition(m_currentBonus);
//		m_currentBonus = WEAPONBONUSCONDITION_INVALID;
//		m_frameToRemove = 0;
//
//		if (getObject()->getDrawable())
//		{
//			if (m_currentTint > TINT_STATUS_INVALID && m_currentTint < TINT_STATUS_COUNT) {
//				getObject()->getDrawable()->clearTintStatus(m_currentTint);
//				m_currentTint = TINT_STATUS_INVALID;
//			}
//
//			// getObject()->getDrawable()->clearTintStatus(TINT_STATUS_FRENZY);
//			//      if (getObject()->isKindOf(KINDOF_INFANTRY))
//			//        getObject()->getDrawable()->setSecondMaterialPassOpacity( 0.0f );
//		}
//	}
//}
//
//// ------------------------------------------------------------------------------------------------
//// ------------------------------------------------------------------------------------------------
//void BuffEffectHelper::doTempWeaponBonus(WeaponBonusConditionType status, UnsignedInt duration, TintStatus tintStatus)
//{
//	// Clear any different status we may have.  Re-getting the same status will just reset the timer
//	if (m_currentBonus != status)
//		clearTempWeaponBonus();
//
//	getObject()->setWeaponBonusCondition(status);
//	m_currentBonus = status;
//	m_frameToRemove = TheGameLogic->getFrame() + duration;
//
//	if (getObject()->getDrawable())
//	{
//		if (tintStatus > TINT_STATUS_INVALID && tintStatus < TINT_STATUS_COUNT) {
//			getObject()->getDrawable()->setTintStatus(tintStatus);
//			m_currentTint = tintStatus;
//		}
//
//		// getObject()->getDrawable()->setTintStatus(TINT_STATUS_FRENZY);
//
//		//    if (getObject()->isKindOf(KINDOF_INFANTRY))
//		//      getObject()->getDrawable()->setSecondMaterialPassOpacity( 1.0f );
//	}
//
//	setWakeFrame(getObject(), UPDATE_SLEEP(duration));
//}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BuffEffectHelper::crc(Xfer* xfer)
{

	// object helper crc
	ObjectHelper::crc(xfer);

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info;
	* 1: Initial version */
	// ------------------------------------------------------------------------------------------------
void BuffEffectHelper::xfer(Xfer* xfer)
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion(&version, currentVersion);

	// object helper base class
	ObjectHelper::xfer(xfer);


	// Next tick frame
	xfer->xferUnsignedInt(&m_nextTickFrame);

	// Buff Effect Tracker list
	// ---------
	// count of vector
	UnsignedShort count = m_buffEffects.size();
	xfer->xferUnsignedShort(&count);

	// data
	if (xfer->getXferMode() == XFER_SAVE)
	{
		for (std::vector<BuffEffectTracker>::iterator it = m_buffEffects.begin(); it != m_buffEffects.end(); ++it)
		{
			BuffEffectTracker bet = *it;
			// m_template
			AsciiString templateName = bet.m_template->getName();
			xfer->xferAsciiString(&templateName);
			// --
			xfer->xferUnsignedInt(&bet.m_frameCreated);
			xfer->xferUnsignedInt(&bet.m_frameToRemove);
			xfer->xferUnsignedInt(&bet.m_numStacks);
			xfer->xferObjectID(&bet.m_sourceID);
			xfer->xferBool(&bet.m_isActive);

		}
	}
	else if (xfer->getXferMode() == XFER_LOAD)
	{
		// vector should be empty at this point
		if (m_buffEffects.empty() == FALSE)
		{
			DEBUG_CRASH(("BuffEffectHelper::xfer - vector is not empty, but should be"));
			throw XFER_LIST_NOT_EMPTY;
		}

		for (UnsignedShort i = 0; i < count; ++i)
		{
			AsciiString templateName;
			xfer->xferAsciiString(&templateName);

			BuffTemplate* bt = TheBuffTemplateStore->findBuffTemplate(templateName);
			if (bt == NULL)
			{
				DEBUG_CRASH(("BuffEffectHelper::xfer - template %s not found", templateName.str()));
				throw XFER_UNKNOWN_STRING;
			}

			BuffEffectTracker bet;
			bet.m_template = bt;
			xfer->xferUnsignedInt(&bet.m_frameCreated);
			xfer->xferUnsignedInt(&bet.m_frameToRemove);
			xfer->xferUnsignedInt(&bet.m_numStacks);
			xfer->xferObjectID(&bet.m_sourceID);
			xfer->xferBool(&bet.m_isActive);

			m_buffEffects.push_back(bet);
		}
	}


}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BuffEffectHelper::loadPostProcess(void)
{

	// object helper base class
	ObjectHelper::loadPostProcess();

}  // end loadPostProcess

