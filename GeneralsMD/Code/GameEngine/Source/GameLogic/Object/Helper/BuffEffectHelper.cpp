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

// #include <unordered_set>

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
	clearAllBuffs();
	// Do we need to actually delete items from m_buffEffects?
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BuffEffectHelper::clearAllBuffs()
{
	for (BuffEffectTracker& buff : m_buffEffects) {
		if (buff.m_isActive) {
			buff.m_template->removeEffects(getObject(), &buff);
		}
	}

	m_buffEffects.clear();
	m_nextTickFrame = UnsignedInt(UPDATE_SLEEP_FOREVER);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime BuffEffectHelper::update()
{
	// TODO: Are there any cases were buffs work on dead objects?
	// Also, should we even get here on a dead object?
	if (getObject()->isEffectivelyDead()) {
		clearAllBuffs();
		return UPDATE_SLEEP_FOREVER;
	}

	UnsignedInt now = TheGameLogic->getFrame();

	DEBUG_LOG(("BuffEffectHelper::update 0 - (frame = %d)", now));

	// Loop over all active BuffEffectTrackers and check if they expired or need to do something

	Bool removed = FALSE;
	std::set<const BuffTemplate*> templatesToRemove;
	UnsignedInt closestNextTickFrame = UnsignedInt(UPDATE_SLEEP_FOREVER);

	for (auto it = m_buffEffects.begin(); it != m_buffEffects.end(); ) {
		BuffEffectTracker& bet = *it;
		if (bet.m_frameToRemove <= now) {
			// remove buff effect
			// TODO: Handle dynamic "unstacking"

			// Only remove effects if we were active
			if (bet.m_isActive) {
				bet.m_template->removeEffects(getObject(), &bet);
				templatesToRemove.insert(bet.m_template);
				removed = TRUE;
			}

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

	// If we removed a buff, we need to re-evaluate all priorities
	if (removed) {
		std::set<AsciiString> tmplSet;  // Template names that any current buff has priority over
		for (BuffEffectTracker& bet : m_buffEffects) {
			if (bet.m_template->getPriorityTemplates().size() > 0)
				tmplSet.insert(bet.m_template->getPriorityTemplates().begin(), bet.m_template->getPriorityTemplates().end());
		}

		// Now loop again to check active/inactive status
		for (BuffEffectTracker& bet : m_buffEffects) {

			// Any flag that was removed, but is still set by an active status needs to be reapplied
			if (bet.m_isActive) {
				for (const BuffTemplate* tmplToRemove : templatesToRemove) {
					bet.m_template->reApplyFlags(getObject(), tmplToRemove);
				}
			}

			bool isLowerPriority = tmplSet.count(bet.m_template->getName());

			// Note: after removal, we can only activate, never deactivate existing buffs!
			//if (isLowerPriority && bet.m_isActive) {
			//	bet.m_isActive = FALSE;
			//	bet.m_template->removeEffects(getObject(), &bet);
			//}

			// Set inactive buffs to active
			if (!isLowerPriority && !bet.m_isActive) {
				bet.m_isActive = TRUE;
				Object* sourceObj = TheGameLogic->findObjectByID(bet.m_sourceID);
				bet.m_template->applyEffects(getObject(), sourceObj, &bet);
			}

		}

	}

	return frameToSleepTime(closestNextTickFrame);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BuffEffectHelper::applyBuff(const BuffTemplate* buffTemplate, Object* sourceObj, UnsignedInt duration)
{
	DEBUG_LOG(("BuffEffectHelper::applyBuff 0"));

	// Note: Should we check if the buff is allowed to be applied here? Or should that happen
	// before, wherever this method is called?

	UnsignedInt now = TheGameLogic->getFrame();

	Bool addBuff = TRUE;
	Bool setActive = TRUE;
	//TODO: also keep track of next tick frame here

	// Check if we have existing buffs with the same template

	std::set<const BuffTemplate*> templatesToRemove;

	for (BuffEffectTracker& buff : m_buffEffects) {

		if (buff.m_template == buffTemplate)
		{ // Check if buff with same template exists

			if (!buffTemplate->isStackPerSource() || // And if we stack with the same source only
				((sourceObj->getID() == buff.m_sourceID) && (buff.m_sourceID != INVALID_ID)))
			{

				// buff exists -> refresh duration; don't add a new buff
				buff.m_frameToRemove = now + duration;
				if (sourceObj)  // always use the last sourceID
					buff.m_sourceID = sourceObj->getID();
				addBuff = FALSE;

				// If more stacks are allowed, apply effects again
				if (buffTemplate->getMaxStackSize() > buff.m_numStacks) {
					buff.m_numStacks += 1;
					buffTemplate->applyEffects(getObject(), sourceObj, &buff);
				}
			}

		}
		else  // check other buffs
		{
			// An existing buff has priority over the one we are adding.
			// This case should never occur if we are already active and increase the stackSize.
			if (buff.m_template->hasPriorityOver(buffTemplate->getName())) {
				setActive = FALSE;
			}

			// Our new buff has priority over an existing one.
			// This case should not matter if we are just refreshing.
			if (buffTemplate->hasPriorityOver(buff.m_template->getName())) {
				// Only disable it if it was previously active.
				// At this point we do not know yet if our new buff will be active or not.
				// But we should never have cyclic priorities so this is ok.
				if (buff.m_isActive) {
					buff.m_template->removeEffects(getObject(), &buff);
					buff.m_isActive = FALSE;

					// When we remove an effect, we need to re-evaluate all flags.
					templatesToRemove.insert(buff.m_template);
				}
			}
		}

		// We are looping through all buff effects, so we might as well get our next tickFrame
		UnsignedInt nextTick = buff.m_template->getNextTickFrame(buff.m_frameCreated, buff.m_frameToRemove);
		if (nextTick < m_nextTickFrame)
			m_nextTickFrame = nextTick;
	}

	// Looping again to evaluate flags
	for (BuffEffectTracker& bet : m_buffEffects) {
		// Any flag that was removed, but is still set by an active status needs to be reapplied
		if (bet.m_isActive) {
			for (const BuffTemplate* tmplToRemove : templatesToRemove) {
				bet.m_template->reApplyFlags(getObject(), tmplToRemove);
			}
		}
	}

	// We add a new buff to our list
	if (addBuff) {
		// Setup BuffEffectTracker entry and add it to the list; TODO: Make this a proper class with memoryPool?
		BuffEffectTracker bet;
		bet.m_template = buffTemplate;
		bet.m_frameCreated = now;
		bet.m_frameToRemove = now + duration;
		bet.m_numStacks = 1;  //todo
		bet.m_isActive = setActive; //todo
		if (sourceObj)
			bet.m_sourceID = sourceObj->getID();

		m_buffEffects.push_back(bet);

		// Apply the actual buff:
		if (setActive)
			buffTemplate->applyEffects(getObject(), sourceObj, &m_buffEffects.back());  //the vector has a copy, use that one.

		// Calculate our next wake frame
		UnsignedInt nextTick = buffTemplate->getNextTickFrame(now, now + duration);
		if (nextTick < m_nextTickFrame) {
			m_nextTickFrame = nextTick;
		}
		// -------------------------------
	}

	DEBUG_LOG(("BuffEffectHelper::applyBuff 1 -- m_nextTickFrame = %d", m_nextTickFrame));
	setWakeFrame(getObject(), frameToSleepTime(m_nextTickFrame));
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

			// particle system vector count and data
			UnsignedShort particleSystemCount = bet.m_particleSystemIDs.size();
			xfer->xferUnsignedShort(&particleSystemCount);
			ParticleSystemID systemID;

			std::vector<ParticleSystemID>::const_iterator it2;
			for (it2 = bet.m_particleSystemIDs.begin(); it2 != bet.m_particleSystemIDs.end(); ++it2)
			{
				systemID = *it2;
				xfer->xferUser(&systemID, sizeof(ParticleSystemID));
			}
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

			// particle system vector count and data
			UnsignedShort particleSystemCount;
			xfer->xferUnsignedShort(&particleSystemCount);

			for (UnsignedShort i = 0; i < particleSystemCount; ++i)
			{
				ParticleSystemID systemID;
				xfer->xferUser(&systemID, sizeof(ParticleSystemID));
				bet.m_particleSystemIDs.push_back(systemID);
			}

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

