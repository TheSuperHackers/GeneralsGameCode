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

#include "PreRTS.h"

#include "GameLogic/Damage.h"
#include "GameLogic/Module/ContainModule.h"

#include "Common/ActionManager.h"
// TheSuperHackers @feature for the EasyMilitaryDrag option
#include "Common/GlobalData.h"
#include "Common/ThingTemplate.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"

#include "GameClient/SelectionInfo.h"
#include "GameClient/CommandXlat.h"
#include "GameClient/ControlBar.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameClient/KeyDefs.h"
// TheSuperHackers @feature for the EasyMilitaryDrag ctrl override
#include "GameClient/Keyboard.h"


//-------------------------------------------------------------------------------------------------
SelectionInfo::SelectionInfo() :
	currentCountEnemies(0),
	currentCountCivilians(0),
	currentCountMine(0),
	currentCountMineInfantry(0),
	currentCountMineBuildings(0),
	currentCountFriends(0),
	newCountEnemies(0),
	newCountCivilians(0),
	newCountCrates(0),
	newCountMine(0),
	newCountMineBuildings(0),
	newCountFriends(0),
	newCountGarrisonableBuildings(0),
	selectEnemies(FALSE),
	selectCivilians(FALSE),
	selectMine(FALSE),
	selectMineBuildings(FALSE),
	selectFriends(FALSE)
{ }

//-------------------------------------------------------------------------------------------------
// TheSuperHackers @feature See SelectionInfo.h. A point selection is never an inverted drag, so
// plain Ctrl clicking still force attacks exactly as it always did.
Bool isEasyMilitaryDragInvertedActive( Bool selectionIsPoint )
{
	if (selectionIsPoint)
		return FALSE;

	if (!TheKeyboard || !TheKeyboard->isCtrl())
		return FALSE;

	// Read from the cached GlobalData copy: constructing OptionPreferences reparses
	// Options.ini from disk, far too heavy for the input path.
	return TheGlobalData && TheGlobalData->m_easyMilitaryDrag;
}

//-------------------------------------------------------------------------------------------------
PickDrawableStruct::PickDrawableStruct() : drawableListToFill(nullptr), isPointSelection(FALSE)
{
	forceAttackMode = TheInGameUI->isInForceAttackMode();

	// TheSuperHackers @feature Read once per selection, not once per drawable. Holding Ctrl
	// inverts the filter for this one drag, so the builders it normally skips are exactly what
	// gets boxed -- the escape hatch for grabbing them deliberately.
	//
	// Ctrl is also what puts the game into force attack mode, so an inverted drag has to cancel
	// that for this selection. Otherwise the click is treated as attack targeting and nothing is
	// selected at all. Force attack has nothing to act on here anyway: this only engages while
	// dragging a box, and what it selects is your own builders.
	easyMilitaryDrag = FALSE;
	easyMilitaryDragInverted = FALSE;
	easyMilitaryDragDisabled = FALSE;
	if (TheGlobalData && TheGlobalData->m_easyMilitaryDrag)
	{
		if (TheKeyboard && TheKeyboard->isCtrl())
			easyMilitaryDragInverted = TRUE;
		else
			easyMilitaryDrag = TRUE;
	}

	// isPointSelection is set by the caller after construction, so this uses the flag resolved
	// above rather than isEasyMilitaryDragInvertedActive. The two agree for drags, which is the
	// only case that reaches the filter.
	if (easyMilitaryDragInverted)
		forceAttackMode = FALSE;

	UnsignedInt pickType = getPickTypesForContext(forceAttackMode);
	translatePickTypesToKindof(pickType, kindofsToMatch);
	if (!forceAttackMode)
	{
		kindofsToMatch.set(KINDOF_ALWAYS_SELECTABLE);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * Given a list of currently selected things and a list of things that are currently under
 * the selection (pointer or drag), generate some useful information about each.
 */
extern Bool contextCommandForNewSelection(const DrawableList *currentlySelectedDrawables,
																					const DrawableList *newlySelectedDrawables,
																					SelectionInfo *outSelectionInfo,
																					Bool selectionIsPoint)
{
	if (!(currentlySelectedDrawables && newlySelectedDrawables && outSelectionInfo))
		return FALSE;

	Bool forceFire = TheInGameUI->isInForceAttackMode();
	Bool forceMove = TheInGameUI->isInForceMoveToMode();

	// TheSuperHackers @feature An inverted EasyMilitaryDrag is a selection, not force attack
	// targeting, even though Ctrl is what triggers both.
	if (isEasyMilitaryDragInvertedActive(selectionIsPoint))
		forceFire = FALSE;

	if (forceFire || forceMove) {
		return FALSE;
	}


	Player *localPlayer = ThePlayerList->getLocalPlayer();
	DrawableListCIt it;
	for (it = currentlySelectedDrawables->begin(); it != currentlySelectedDrawables->end(); ++it) {
		if (!(*it)) {
			continue;
		}

		Object *obj = (*it)->getObject();
		if (!obj) {
			continue;
		}

		if (obj->isLocallyControlled()) {
			++outSelectionInfo->currentCountMine;
			if (obj->isKindOf(KINDOF_INFANTRY)) {
				++outSelectionInfo->currentCountMineInfantry;
			} else if (obj->isKindOf(KINDOF_STRUCTURE)) {
				++outSelectionInfo->currentCountMineBuildings;
			}
		} else {
			Relationship rel = localPlayer->getRelationship(obj->getTeam());
			if (rel == ALLIES) {
				++outSelectionInfo->currentCountFriends;
			} else if (rel == ENEMIES) {
				++outSelectionInfo->currentCountEnemies;
			} else if (rel == NEUTRAL) {
				++outSelectionInfo->currentCountCivilians;
			}
		}
	}

	Drawable *newMine = nullptr;
	Drawable *newFriendly = nullptr;
	Drawable *newEnemy = nullptr;
	Drawable *newCivilian = nullptr;

	for (it = newlySelectedDrawables->begin(); it != newlySelectedDrawables->end(); ++it) {
		if (!(*it)) {
			continue;
		}

		Object *obj = (*it)->getObject();
		if (!obj) {
			continue;
		}

		if (TheActionManager->canPlayerGarrison(localPlayer, obj, CMD_FROM_PLAYER)) {
			++outSelectionInfo->newCountGarrisonableBuildings;
		}
		if (obj->isKindOf(KINDOF_CRATE)) {
			++outSelectionInfo->newCountCrates;
		}

		if (obj->isLocallyControlled()) {
			++outSelectionInfo->newCountMine;
			newMine = *it;
			if (obj->isKindOf(KINDOF_STRUCTURE)) {
				++outSelectionInfo->newCountMineBuildings;
			}
		} else {
			Relationship rel = localPlayer->getRelationship(obj->getTeam());
			if (rel == ALLIES) {
				newFriendly = *it;
				++outSelectionInfo->newCountFriends;
			} else if (rel == ENEMIES) {
				newEnemy = *it;
				++outSelectionInfo->newCountEnemies;
			} else if (rel == NEUTRAL) {
				newCivilian = *it;
				++outSelectionInfo->newCountCivilians;
			}
		}
	}

	DEBUG_ASSERTCRASH(outSelectionInfo->currentCountEnemies <= 1, ("Selection bug. jkmcd"));
	DEBUG_ASSERTCRASH(outSelectionInfo->currentCountFriends <= 1, ("Selection bug. jkmcd"));
	DEBUG_ASSERTCRASH(outSelectionInfo->currentCountCivilians <= 1, ("Selection bug. jkmcd"));

	if (outSelectionInfo->currentCountEnemies > 0) {
		// If we have an enemy selected, there are no context sensitive commands
		return FALSE;
	}

	if (outSelectionInfo->currentCountFriends > 0) {
		return FALSE;
	}

	if (outSelectionInfo->currentCountCivilians > 0) {
		return FALSE;
	}

	if (TheGlobalData->m_useAlternateMouse) {
		// context sensitive commands never apply when selecting in alternate mouse mode
		return FALSE;
	}

	if (outSelectionInfo->currentCountMine > 0) {
		if (outSelectionInfo->newCountEnemies > 0) {
			if (outSelectionInfo->newCountEnemies == 1 && selectionIsPoint) {
				return TheGameClient->evaluateContextCommand(newEnemy, newEnemy->getPosition(), CommandTranslator::EVALUATE_ONLY) != GameMessage::MSG_INVALID;
			}

			return selectionIsPoint;
		}

		if (outSelectionInfo->newCountMine > 0) {
			if (outSelectionInfo->newCountMine == 1 && selectionIsPoint && !TheInGameUI->isInPreferSelectionMode()) {
				return TheGameClient->evaluateContextCommand(newMine, newMine->getPosition(), CommandTranslator::EVALUATE_ONLY) != GameMessage::MSG_INVALID;
			}

			return FALSE;
		}

		if (outSelectionInfo->newCountFriends > 0) {
			if (outSelectionInfo->newCountFriends == 1 && selectionIsPoint) {
				return TheGameClient->evaluateContextCommand(newFriendly, newFriendly->getPosition(), CommandTranslator::EVALUATE_ONLY) != GameMessage::MSG_INVALID;
			}
			return FALSE;
		}

		if (outSelectionInfo->currentCountMineInfantry > 0 && outSelectionInfo->newCountGarrisonableBuildings == 1) {
			return TRUE;
		}

		if (outSelectionInfo->newCountCivilians > 0) {
			if (outSelectionInfo->newCountCivilians == 1 && selectionIsPoint) {
				return TheGameClient->evaluateContextCommand(newCivilian, newCivilian->getPosition(), CommandTranslator::EVALUATE_ONLY) != GameMessage::MSG_INVALID;
			}
			return FALSE;
		}

		if (outSelectionInfo->newCountCrates > 0) {
			return (outSelectionInfo->newCountCrates == 1 && selectionIsPoint);
		}
	}

	if (outSelectionInfo->currentCountMine == 0) {
		return FALSE;
	}

	return selectionIsPoint;
}

//-------------------------------------------------------------------------------------------------
UnsignedInt getPickTypesForContext( Bool forceAttackMode )
{
	UnsignedInt types = PICK_TYPE_SELECTABLE;

	if (forceAttackMode)
		types |= PICK_TYPE_FORCEATTACKABLE;

	//
	// if we have a gui context command that allows for a shrubbery target then we want to
	// pick that type too (generally shrubbery aren't pickable cause it would get in
	// the way with movement and general selection)
	//
	const CommandButton *command = TheInGameUI->getGUICommand();

	if (command != nullptr) {
		if (BitIsSet( command->getOptions(), ALLOW_MINE_TARGET)) {
			types |= PICK_TYPE_MINES;
		}

		if (BitIsSet( command->getOptions(), ALLOW_SHRUBBERY_TARGET ) ) {
			types |= PICK_TYPE_SHRUBBERY;
		}
	} else {
		types |= getPickTypesForCurrentSelection(forceAttackMode);
	}

	return types;

}

//-------------------------------------------------------------------------------------------------
UnsignedInt getPickTypesForCurrentSelection( Bool forceAttackMode )
{
	UnsignedInt retVal = 0;
	if (!TheInGameUI->areSelectedObjectsControllable()) {
		return retVal;
	}

	const DrawableList *allSelectedDrawables = TheInGameUI->getAllSelectedDrawables();

	for (DrawableListCIt cit = allSelectedDrawables->begin(); cit != allSelectedDrawables->end(); ++cit) {
		Drawable *draw = *cit;
		if (!draw) {
			continue;
		}

		Object *obj = draw->getObject();
		if (!obj) {
			continue;
		}

// srj sez: thanks to new, area-effect disarming, we NO LONGER want to do this...
//		if (obj->hasWeaponToDealDamageType(DAMAGE_DISARM)) {
//			retVal |= PICK_TYPE_MINES;
//		}

		if (obj->hasWeaponToDealDamageType(DAMAGE_FLAME) && forceAttackMode ) {
			retVal |= PICK_TYPE_SHRUBBERY;
		}

		// For efficiency.
		if (BitIsSet(retVal, PICK_TYPE_MINES | PICK_TYPE_SHRUBBERY)) {
			break;
		}
	}

	return retVal;

}

//-------------------------------------------------------------------------------------------------
void translatePickTypesToKindof(UnsignedInt pickTypes, KindOfMaskType& outMask)
{
	if (BitIsSet(pickTypes, PICK_TYPE_SELECTABLE)) {
		outMask.set(KINDOF_SELECTABLE);
	}

	if (BitIsSet(pickTypes, PICK_TYPE_SHRUBBERY)) {
		outMask.set(KINDOF_SHRUBBERY);
	}

	if (BitIsSet(pickTypes, PICK_TYPE_MINES)) {
		outMask.set(KINDOF_MINE);
	}

	if (BitIsSet(pickTypes, PICK_TYPE_FORCEATTACKABLE)) {
		outMask.set(KINDOF_FORCEATTACKABLE);
	}
}

//-------------------------------------------------------------------------------------------------
// Given a drawable, add it to an stl list specified by userData.
// Useful for iterateDrawablesInRegion.
Bool addDrawableToList( Drawable *draw, void *userData )
{
	PickDrawableStruct *pds = (PickDrawableStruct *) userData;
#if defined(RTS_DEBUG)
	if (TheGlobalData->m_allowUnselectableSelection) {
		pds->drawableListToFill->push_back(draw);
		return TRUE;
	}
#endif

	if (!pds->drawableListToFill)
		return FALSE;

#if !RTS_GENERALS || !PRESERVE_OCCUPANT_DETECTION_VIA_DRAG_SELECTION
	// TheSuperHackers @info
	// In retail, drag-selecting allows the player to select stealthed objects and objects through the
	// fog. Some players exploit this bug to determine where an opponent's units are and consider this
	// an important feature and an advanced skill to pull off, so we must leave the exploit.
	if (draw->getFullyObscuredByShroud())
		return FALSE;

	if (draw->isDrawableEffectivelyHidden())
		return FALSE;
#endif

	if (!draw->getTemplate()->isAnyKindOf(pds->kindofsToMatch))
		return FALSE;

	// TheSuperHackers @feature EasyMilitaryDrag leaves builders out of a drag selection, so boxing
	// over your own base picks up the army without dragging them off their work. Only drags are
	// affected -- a point selection still picks them normally, as do double click, control groups
	// and the select-matching hotkeys.
	//
	// The two kinds mirror what Select All already disqualifies: KINDOF_DOZER, and the explicit
	// KINDOF_IGNORES_SELECT_ALL marker a mod can put on anything else it wants left alone. Note
	// KINDOF_DOZER also catches GLA Workers, which carry it alongside KINDOF_INFANTRY and
	// KINDOF_HARVESTER.
	if (!pds->easyMilitaryDragDisabled &&
			!pds->isPointSelection && (pds->easyMilitaryDrag || pds->easyMilitaryDragInverted))
	{
		const Bool isBuilder =
				draw->isKindOf(KINDOF_DOZER) || draw->isKindOf(KINDOF_IGNORES_SELECT_ALL);

		// normally drop the builders; with Ctrl held, drop everything else instead
		const Bool wantBuilders = pds->easyMilitaryDragInverted;
		if (isBuilder != wantBuilders)
			return FALSE;
	}

	if (!draw->isSelectable())
  {
    const Object *obj = draw->getObject();
    if ( obj && obj->getContainedBy() )//hmm, interesting... he is not selectable but he is contained
    {// What we are after here is to propagate the selection the selection ti the container
      // if the container is non-enclosing... see also SelectionXlat, in the left_click case

      ContainModuleInterface *contain = obj->getContainedBy()->getContain();
      Drawable *containDraw = obj->getContainedBy()->getDrawable();
      if (contain && ! contain->isEnclosingContainerFor( obj ) && containDraw )
        return addDrawableToList( containDraw, userData );
    }
    else
      return FALSE;
  }

#if !RTS_GENERALS && PRESERVE_OCCUPANT_DETECTION_VIA_DRAG_SELECTION
	// TheSuperHackers @info
	// In retail, hidden objects such as passengers are included here when drag-selected, which causes
	// enemy selection logic to exit early (only 1 enemy unit can be selected at a time). Some players
	// exploit this bug to determine if a transport contains passengers and consider this an important
	// feature and an advanced skill to pull off, so we must leave the exploit.
	if (!pds->isPointSelection)
	{
		const Object *obj = draw->getObject();
		if (obj)
			if (!obj->isLocallyControlled())
				if (obj->getContain() && draw->getObject()->getContain()->getContainCount() > 0)
					return FALSE;
	}
#endif

	pds->drawableListToFill->push_back(draw);
	return TRUE;
}
