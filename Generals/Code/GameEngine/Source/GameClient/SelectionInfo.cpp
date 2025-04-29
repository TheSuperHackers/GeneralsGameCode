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

#include "PreRTS.h"
#include "GameClient/SelectionInfo.h"

#include "Common/ActionManager.h"
#include "GameLogic/Damage.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingTemplate.h"

#include "GameClient/CommandXlat.h"
#include "GameClient/ControlBar.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameClient/KeyDefs.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

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
PickDrawableStruct::PickDrawableStruct() : drawableListToFill(NULL)
{
	//Added By Sadullah Nader
	//Initializations inserted
	drawableListToFill = FALSE;
	//
	forceAttackMode = TheInGameUI->isInForceAttackMode();
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

	Drawable *newMine = NULL;
	Drawable *newFriendly = NULL;
	Drawable *newEnemy = NULL;
	Drawable *newCivilian = NULL;

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

	if (command != NULL) {
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

}  // end getPickTypesForContext

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
#if defined(RTS_DEBUG) || defined(_INTERNAL)
	if (TheGlobalData->m_allowUnselectableSelection) {
		pds->drawableListToFill->push_back(draw);
		return TRUE;
	}
#endif

	if (!pds->drawableListToFill)
		return FALSE;

	if (!draw->getTemplate()->isAnyKindOf(pds->kindofsToMatch))
		return FALSE;

	if (!draw->isSelectable())
		return FALSE;

	pds->drawableListToFill->push_back(draw);
	return TRUE;
}
