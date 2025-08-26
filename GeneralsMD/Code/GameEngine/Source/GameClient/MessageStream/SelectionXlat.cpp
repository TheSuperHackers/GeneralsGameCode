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

// SelectionXlat.cpp
// Message stream translator
// Author: Michael S. Booth, January 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/ActionManager.h"
#include "Common/GameAudio.h"
#include "Common/GameEngine.h"
#include "Common/MessageStream.h"
#include "Common/MiscAudio.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingTemplate.h"

#include "GameLogic/Damage.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Squad.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/UpdateModule.h"

#include "GameClient/ControlBar.h"
#include "GameClient/Display.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Keyboard.h"
#include "GameClient/SelectionInfo.h"
#include "GameClient/SelectionXlat.h"
#include "GameClient/TerrainVisual.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Lorenzen changed this to a member of SelectionTranslator, providing external access
// name ly in rebuildholeexposedie, where we decide whether to create GLA Holes when hand-of-Godding
//#if defined(RTS_DEBUG) || defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
//static Bool TheHandOfGodSelectionMode = false;
//#endif

#if defined(RTS_DEBUG)
static Bool TheHurtSelectionMode = false;
static Bool TheDebugSelectionMode = false;
#endif

//-----------------------------------------------------------------------------
static Bool currentlyLookingForSelection( )
{
	// This needs to check if we are currently targetting for special weapons fire.
	return TheInGameUI->getGUICommand() == NULL;
}

//-----------------------------------------------------------------------------
static Bool areAllSelected( const DrawableList& listToCheck )
{
	DrawableListCIt it;
	for ( it = listToCheck.begin(); it != listToCheck.end(); ++it ) {
		if (!*it)
			continue;

		if (!(*it)->isSelected())
			return FALSE;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
struct SFWRec
{
	SelectionTranslator *translator;
	GameMessage *createTeamMsg;
	Bool dragSelecting;
};

//-----------------------------------------------------------------------------
/*friend*/ Bool selectFriendsWrapper( Drawable *draw, void *userData )
{
	SFWRec *info = (SFWRec *)userData;
	return info->translator->selectFriends(draw, info->createTeamMsg, info->dragSelecting) != 0;
}  // end selectFriendsWrapper

/*friend*/ Bool killThemKillThemAllWrapper( Drawable *draw, void *userData )
{
	SFWRec *info = (SFWRec *)userData;
	info->translator->killThemKillThemAll( draw, info->createTeamMsg );
	return true;
}

//-----------------------------------------------------------------------------
/**
 * Returns true if the drawable can be selected under the current rules
 * of the system
 */
Bool CanSelectDrawable( const Drawable *draw, Bool dragSelecting )
{

	if(!draw || !draw->getObject())
	{
		return FALSE;  // can't select
	}
	const Object *obj = draw->getObject();

	if( obj->isEffectivelyDead() && !obj->isKindOf(KINDOF_ALWAYS_SELECTABLE))
	{
		//Don't select dead/dying units.
		return FALSE;
	}

	//Added this to support attacking cargo planes without being able to select them.
	//I added the KINDOF_FORCEATTACKABLE to them, but unsure if it's possible to select
	//something without the KINDOF_SELECTABLE -- so doing a LATE code change. My gut
	//says we should simply have the KINDOF_SELECTABLE check only... but best to be safe.
	if( !obj->isKindOf( KINDOF_SELECTABLE ) && obj->isKindOf( KINDOF_FORCEATTACKABLE ) )
	{
		return FALSE;
	}

	// hidden objects cannot be selected
	if( draw->isDrawableEffectivelyHidden() )
	{
		return FALSE;  // can't select
	}

	// ignore objects obscured by the GUI
	GameWindow *window = NULL;
	if (TheWindowManager)
	{
		const Coord3D *c = draw->getPosition();
		ICoord2D c2;
		TheTacticalView->worldToScreen(c, &c2);
		window = TheWindowManager->getWindowUnderCursor(c2.x, c2.y);
	}

	while (window)
	{
		// check to see if it or any of its parents are opaque.  If so, we can't select anything.
		if (!BitIsSet( window->winGetStatus(), WIN_STATUS_SEE_THRU ))
		{
			return FALSE;
		}

		window = window->winGetParent();
	}

	//
	// structures cannot be selected by a drag select, you must individually pick them
	// NOTE that this is really a convenience for the multi select context sensitive UI,
	// later we might want to allow you to drag select buildings if only one building is
	// actually in the selection area, but don't forget complications like holding down
	// a key to "add" to an already existing selection list
	//
	// not allowing you to have multiple buildings selected drastically simplifies the
	// user interface ... including all those context sensitive commands that we
	// can just assume are for a single building selected.
	//
	if( dragSelecting && draw->isKindOf( KINDOF_STRUCTURE ) )
	{
		return FALSE;
	}

	// You cannot select something that has a logic override of unselectability or masked
	if( obj->getStatusBits().testForAny( MAKE_OBJECT_STATUS_MASK2( OBJECT_STATUS_UNSELECTABLE, OBJECT_STATUS_MASKED ) ) )
	{
		return FALSE;
	}

	if (!obj->isSelectable())
	{
		return false;
	}
	//Now allowing the selection of everything including enemies... but only if not drag selecting.
	//In fact the only way you can drag select is if the unit is on your team.
	if( dragSelecting && !obj->isLocallyControlled() )
	{
		return FALSE;
	}

	//Now we can select anything that is selectable.
	return TRUE;

}  // end canSelect

//-----------------------------------------------------------------------------
static Bool canSelectWrapper( Drawable *draw, void *userData )
{
	Bool dragSelecting = *((Bool *)userData);
	return CanSelectDrawable( draw, dragSelecting );
}

//-----------------------------------------------------------------------------
/**
 * Deselect all drawables, and emit a "TEAM_DESTROY" message, since
 * the "team" was the group of currently selected units.
 */
static void deselectAll()
{

	// deselect it all
	TheInGameUI->deselectAllDrawables();
}

//-----------------------------------------------------------------------------
/**
 * Select the given drawable, without playing its sound.
 * Returns true.
 */
static Bool selectSingleDrawableWithoutSound( Drawable *draw )
{

	// since we are single selecting a drawable, unselect everything else
	deselectAll();

	// do the drawble selection
	TheInGameUI->selectDrawable( draw );

	Object *obj = draw->getObject();
	if (obj != NULL) {
		GameMessage *msg = TheMessageStream->appendMessage(GameMessage::MSG_CREATE_SELECTED_GROUP_NO_SOUND);
		msg->appendBooleanArgument(TRUE);
		msg->appendObjectIDArgument(obj->getID());
	}

	return true;

}

SelectionTranslator *TheSelectionTranslator = NULL;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
SelectionTranslator::SelectionTranslator()
{
	m_leftMouseButtonIsDown = FALSE;
	m_dragSelecting = FALSE;
	m_lastGroupSelTime = 0;
	m_lastGroupSelGroup = -1;
	m_selectFeedbackAnchor.x = 0;
	m_selectFeedbackAnchor.y = 0;
	m_deselectFeedbackAnchor.x = 0;
	m_deselectFeedbackAnchor.y = 0;
	m_lastClick = 0;
	//Added By Sadullah Nader
	//Initializtion(s) inserted
	m_deselectDownCameraPosition.zero();
	m_displayedMaxWarning = FALSE;
	//
	m_selectCountMap.clear();

	TheSelectionTranslator = this;

#if defined(RTS_DEBUG) || defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
  m_HandOfGodSelectionMode = FALSE;
#endif
}

//-----------------------------------------------------------------------------
SelectionTranslator::~SelectionTranslator()
{
}

//-----------------------------------------------------------------------------
/**
 * If this drawable is a 'friend' of mine, select it.
 */
Bool SelectionTranslator::selectFriends( Drawable *draw, GameMessage *createTeamMsg,
																				 Bool dragSelecting )
{
	if (CanSelectDrawable( draw, dragSelecting ))
	{
		// enforce an optional selection size limit
		if (TheInGameUI->getMaxSelectCount() > 0 && TheInGameUI->getSelectCount() >= TheInGameUI->getMaxSelectCount())
		{
			if (!m_displayedMaxWarning)
			{
				m_displayedMaxWarning = TRUE;
				UnicodeString msg;
				msg.format(TheGameText->fetch("GUI:MaxSelectionSize").str(), TheInGameUI->getMaxSelectCount());
				TheInGameUI->message(msg);
			}
			return false;
		}

		TheInGameUI->selectDrawable( draw );

		m_selectCountMap[draw->getTemplate()]++;

		// add to message's argument list if an object is present
		if( draw->getObject() && createTeamMsg )
			createTeamMsg->appendObjectIDArgument( draw->getObject()->getID() );

		return true;  // selected

	}  // end if

	return false;  // not selected

}  // end selectFriends


//-----------------------------------------------------------------------------
Bool SelectionTranslator::killThemKillThemAll( Drawable *draw, GameMessage *killThemAllMsg )
{
	if( draw )
	{
		Object *obj = draw->getObject();
		if( obj )
		{
			// enforce an optional selection size limit
			if (TheInGameUI->getMaxSelectCount() > 0 && TheInGameUI->getSelectCount() >= TheInGameUI->getMaxSelectCount())
			{
				if (!m_displayedMaxWarning)
				{
					m_displayedMaxWarning = TRUE;
					UnicodeString msg;
					msg.format(TheGameText->fetch("GUI:MaxSelectionSize").str(), TheInGameUI->getMaxSelectCount());
					TheInGameUI->message(msg);
				}
				return false;
			}

			// add to message's argument list if an object is present
			if( killThemAllMsg )
			{
				killThemAllMsg->appendObjectIDArgument( draw->getObject()->getID() );
			}

			return true;  // selected
		}
	}
	return false;
}  // end selectFriends

//-----------------------------------------------------------------------------
/**
 * The SelectionTranslator is responsible for all selection semantics,
 * including click selection, area drag selection, right-click de-selection,
 * and CTRL-key group selection.
 * NOTE: This handler changes the event semantics for mouse buttons from
 * LEFT_DOWN -> LEFT_UP  to  LEFT_DOWN -> { LEFT_UP, AREA_SELECTION, or DRAWABLE_PICKED }
 */
GameMessageDisposition SelectionTranslator::translateGameMessage(const GameMessage *msg)
{
	GameMessageDisposition disp = KEEP_MESSAGE;

	if(	!TheInGameUI->getInputEnabled() )
	{
		//Keep the message so the other translaters (WindowXlat) can handle.
		if( m_dragSelecting )
		{
			//Turn off drag select
			m_dragSelecting = FALSE;
			TheInGameUI->setSelecting( FALSE );
			TheInGameUI->endAreaSelectHint(NULL);
			TheTacticalView->setMouseLock( FALSE );
		}
		return KEEP_MESSAGE;
	}

	GameMessage::Type t = msg->getType();
	switch (t)
	{
		case GameMessage::MSG_META_BEGIN_FORCEATTACK:
			TheInGameUI->setForceAttackMode( true );
			break;

		case GameMessage::MSG_META_END_FORCEATTACK:
			TheInGameUI->setForceAttackMode( false );
			break;

		//-----------------------------------------------------------------------------
		case GameMessage::MSG_RAW_MOUSE_POSITION:
		{
			ICoord2D pixel;
			pixel = msg->getArgument( 0 )->pixel;


			// modifier appears to be unused, and the argument doesn't exist.  jba.
			//Int modifier = msg->getArgument( 1 )->integer;

			if (m_leftMouseButtonIsDown)
			{
				ICoord2D delta;

				delta.x = abs(pixel.x - m_selectFeedbackAnchor.x);
				delta.y = abs(pixel.y - m_selectFeedbackAnchor.y);

				// if mouse has moved while left button is down, begin drag selection
				if (delta.x > TheMouse->m_dragTolerance || delta.y > TheMouse->m_dragTolerance)
				{
					if (m_dragSelecting == false)
					{
						m_dragSelecting = true;
						TheTacticalView->setMouseLock( TRUE );
						TheInGameUI->setSelecting( TRUE );
					}
				}

				// create "hint" messages defining selection region under construction
				if (m_dragSelecting)
				{
					// insert area selection "hint" message into stream
					GameMessage *hintMsg = TheMessageStream->appendMessage( GameMessage::MSG_AREA_SELECTION_HINT );

					// build rectangular region defined by the drag selection
					IRegion2D pixelRegion;
					buildRegion( &m_selectFeedbackAnchor, &pixel, &pixelRegion );
					hintMsg->appendPixelRegionArgument( pixelRegion );
				}
			}
			else //left button is not down (not drag select)
			{
				// insert Mouseover hint into stream for CommandTranslator and HintSpy to see.
				GameMessage *mouseoverMessage;

				//Kris: We want to show information such as the popup text on objects that are forceattackable even
				//      when we're not in force attackable mode!
				UnsignedInt pickType = getPickTypesForContext( true /*TheInGameUI->isInForceAttackMode()*/ );

				Drawable *underCursor = TheTacticalView->pickDrawable( &pixel, TheInGameUI->isInForceAttackMode(), (PickType) pickType );
				Object *objUnderCursor = underCursor ? underCursor->getObject() : NULL;

				if( objUnderCursor && (!objUnderCursor->isEffectivelyDead() || objUnderCursor->isKindOf( KINDOF_ALWAYS_SELECTABLE )) )
				{
					mouseoverMessage = TheMessageStream->appendMessage( GameMessage::MSG_MOUSEOVER_DRAWABLE_HINT );
					mouseoverMessage->appendDrawableIDArgument( underCursor->getID() );
				}
				else// else this is a mouseover terrain
				{
					Coord3D position;

					TheTacticalView->screenToTerrain( &pixel, &position );
					mouseoverMessage = TheMessageStream->appendMessage( GameMessage::MSG_MOUSEOVER_LOCATION_HINT );
					mouseoverMessage->appendLocationArgument( position );
				}
			}

			break;
		}

		//-----------------------------------------------------------------------------
		case GameMessage::MSG_MOUSE_LEFT_DOUBLE_CLICK:
		{
			Int modifiers = msg->getArgument(1)->integer;

			// Pressing ctrl is disallowed for double clicking
			if (TheInGameUI->isInForceAttackMode())
				break;

			const IRegion2D& region = msg->getArgument(0)->pixelRegion;

			// Single point. If there's a unit in there, double click will select all of them.
			if (region.height() == 0 && region.width() == 0)
			{
				Bool selectAcrossMap = (BitIsSet(modifiers, KEY_STATE_ALT) ? TRUE : FALSE);

				// only allow things that are selectable. Also, we aren't allowed to
				Drawable *picked = TheTacticalView->pickDrawable( &region.lo, FALSE, PICK_TYPE_SELECTABLE);

				// If there wasn't anyone to pick, then we want to propagate this double click.
				if (picked == NULL)
					break;

				if (!picked->isMassSelectable())
					break;

				Object *pickedObj = picked->getObject();

				// We have to have an object in order to be able to do interesting double click stuff on
				// him. Also, if it is a structure, it is already selected, so don't select all the units
				// like him.
				if (pickedObj == NULL || !pickedObj->isLocallyControlled())
					break;

				// Ok. The logic is a little bit weird here. What we need to do is deselect everything
				// except for this one picked thing. Store off the old selection, pick the single clicked thing.
				// Then if
				DrawableList listOfSelectedDrawables;
				if (TheInGameUI->isInPreferSelectionMode()) {
					listOfSelectedDrawables	= *TheInGameUI->getAllSelectedDrawables();
				}

				// Pick just that one guy.
				selectSingleDrawableWithoutSound(picked);

				// Yay. Either select across the screen or the world depending on selectAcrossMap
				if (selectAcrossMap)
					TheInGameUI->selectMatchingAcrossMap();
				else
					TheInGameUI->selectMatchingAcrossScreen();

				// emit "picked" message
				GameMessage *pickMsg = TheMessageStream->appendMessage( GameMessage::MSG_AREA_SELECTION );
				pickMsg->appendDrawableIDArgument( picked->getID() );  /// note we are putting in a drawable id

				if (TheInGameUI->isInPreferSelectionMode() && !listOfSelectedDrawables.empty()) {
					GameMessage *selectMore = TheMessageStream->appendMessage( GameMessage::MSG_CREATE_SELECTED_GROUP_NO_SOUND );
					selectMore->appendBooleanArgument(FALSE);
					for (DrawableListIt it = listOfSelectedDrawables.begin(); it != listOfSelectedDrawables.end(); ++it) {
						Drawable *draw = *it;
						if (draw && draw->isSelectable()) {
							TheInGameUI->selectDrawable(draw);
							selectMore->appendObjectIDArgument(draw->getObject()->getID());
						}
					}
				}

				disp = DESTROY_MESSAGE;
			}
			break;
		}

		//-----------------------------------------------------------------------------
		case GameMessage::MSG_MOUSEOVER_DRAWABLE_HINT:
		{
			if (TheInGameUI->isScrolling()) {
				// dont show this now.
				break;
			}

			DrawableID id = msg->getArgument(0)->drawableID;
			Drawable *draw = TheGameClient->findDrawableByID(id);
			if (!draw) {
				break;
			}

			GameMessage::Type msgType = TheGameClient->evaluateContextCommand(draw, draw->getPosition(), CommandTranslator::EVALUATE_ONLY);
			if( msgType == GameMessage::MSG_INVALID )
			{
				TheInGameUI->createMouseoverHint(msg); // this sets the cursor
				disp = DESTROY_MESSAGE;
				const CommandButton *command = TheInGameUI->getGUICommand();

				Bool ignoreCommand = FALSE;
				if( command )
				{
					if( command->getCommandType() == GUI_COMMAND_ATTACK_MOVE ||
							command->getCommandType() == GUI_COMMAND_GUARD ||
							command->getCommandType() == GUI_COMMAND_GUARD_WITHOUT_PURSUIT ||
							command->getCommandType() == GUI_COMMAND_GUARD_FLYING_UNITS_ONLY )
					{
						//These GUI commands can take care of themselves -- don't let
						//the selection translator meddle.
						ignoreCommand = TRUE;
					}
				}
				if( !ignoreCommand && !draw->getTemplate()->isKindOf( KINDOF_SHRUBBERY ) )
				{
					if( CanSelectDrawable( draw, FALSE ) )
					{
						TheMouse->setCursor(Mouse::SELECTING);
					}
					else
					{
						TheMouse->setCursor( Mouse::ARROW );
					}
				}
			}

			break;
		}

		//-----------------------------------------------------------------------------
		case GameMessage::MSG_MOUSE_LEFT_CLICK:
		{
			// If the quit menu is visible, we need to not process left clicks through the selection translator.
			if (TheInGameUI->isQuitMenuVisible())
			{
				disp = DESTROY_MESSAGE;
				break;
			}

			// Basically, we need to first determine if there are any drawables in the region of interest.
			// If there aren't then this click should move forward.
			IRegion2D selectionRegion = msg->getArgument(0)->pixelRegion;
			Bool isPoint = (selectionRegion.height() == 0 && selectionRegion.width() == 0);

			DrawableList drawablesThatWillSelect;
			PickDrawableStruct pds;
			pds.drawableListToFill = &drawablesThatWillSelect;
			TheTacticalView->iterateDrawablesInRegion(&selectionRegion, addDrawableToList, &pds);

			if (drawablesThatWillSelect.empty())
			{
				break;
			}

			// if there were drawables in the region, then we should determine if there is a context
			// sensitive command that should take place. If there is, then this isn't a selection thing
			const DrawableList *currentList = TheInGameUI->getAllSelectedDrawables();
			if (!currentlyLookingForSelection())
			{
				break;
			}

			SelectionInfo si;
			if (contextCommandForNewSelection(currentList, &drawablesThatWillSelect, &si, isPoint))
			{
				break;
			}

			// There isn't a context command, so this is a selection thing. Now, based on the keys,
			// determine whether or not we should create a new group, or append these guys to our existing
			// group.

			Bool addToGroup = TheInGameUI->isInPreferSelectionMode();

			if (si.currentCountEnemies > 0 ||
					si.currentCountCivilians > 0 ||
					si.currentCountFriends > 0 ||
					si.currentCountMineBuildings > 0)
			{
				// force a new group creation
				addToGroup = FALSE;
			}

			// If there are any of my units, then select those.
			if (si.newCountMine > 0)
			{
				si.selectMine = TRUE;

        // EXACTLY ONE CLICKED OR DRAGGED BUILDING
				if ( si.newCountMineBuildings == 1 && si.newCountMine == 1 )
				{
					addToGroup = FALSE;
					si.selectMineBuildings = TRUE;
        }
        else if ( si.newCountMineBuildings > 0 )////////////// SO SORRY, I KNOW THIS IS MICKEY MOUSE ///////////////////
        { // What we are after here is to allow the drag select to get the building,
          // if the other things in the list are going to be ignored anyway
          // so we find out whether the other things are not selectible
          // this came up with the new AmericaBuildingFireBase, which shows its contained
          // but does not let you select them. The selection is propagated to the container
          // in new code in SelectionInfo.cpp, in the static addDrawableToList();
          // -Mark Lorenzen, 6/12/03
          Bool onlyTheOneBuildingIsSelectableAnyway = TRUE;
          DrawableID buildingID = INVALID_DRAWABLE_ID;
          for (DrawableListIt it = drawablesThatWillSelect.begin(); it != drawablesThatWillSelect.end(); ++it)
				  {
            const Drawable *d = *it;
            if ( d->isKindOf( KINDOF_STRUCTURE ) )
            {// make sure there is really only the one building in the list, as it may be multiply listed

              if ( buildingID == INVALID_DRAWABLE_ID ) // this is the first building
                buildingID = d->getID();
              else if ( buildingID != d->getID() )//oops, more than one building!
                onlyTheOneBuildingIsSelectableAnyway = FALSE;
            }
					  else if ( d->isSelectable() )
              onlyTheOneBuildingIsSelectableAnyway = FALSE;

            if ( ! onlyTheOneBuildingIsSelectableAnyway )
              break;
          }
          if ( onlyTheOneBuildingIsSelectableAnyway )
          {
					  addToGroup = FALSE;
					  si.selectMineBuildings = TRUE;
          }
				}

			}
			else if (si.newCountEnemies > 0 && si.newCountCivilians > 0 && si.newCountFriends > 0)
			{
				// No go here
				break;
			}
			else if (si.newCountEnemies == 1)
			{
				addToGroup = FALSE;
				si.selectEnemies = TRUE;
			}
			else if (si.newCountCivilians == 1)
			{
				addToGroup = FALSE;
				si.selectCivilians = TRUE;
			}
			else if (si.newCountFriends == 1)
			{
				addToGroup = FALSE;
				si.selectFriends = TRUE;
			}

			// If we're not going to select anything, just bail now.
			if (!(si.selectMine || si.selectEnemies || si.selectCivilians || si.selectFriends))
			{
				break;
			}

			// If we've made it here, its time to do some selecting.
			disp = DESTROY_MESSAGE;

			// Whenever we manually select something, reset the last selected group.
			m_lastGroupSelGroup = -1;

			if (TheInGameUI->isInPreferSelectionMode() && isPoint && areAllSelected(drawablesThatWillSelect))
			{
				// If this was a point, shift was pressed and we already have that unit selected, then we
				// need to deselect those units.
				GameMessage *newMsg = TheMessageStream->appendMessage(GameMessage::MSG_REMOVE_FROM_SELECTED_GROUP);
				Drawable *draw = NULL;
				DrawableListIt it;
				for (it = drawablesThatWillSelect.begin(); it != drawablesThatWillSelect.end(); ++it)
				{
					draw = *it;
					if (!draw)
					{
						continue;
					}

					Object *objToDeselect = draw->getObject();
					if (!objToDeselect)
					{
						continue;
					}

					newMsg->appendObjectIDArgument(objToDeselect->getID());
					TheInGameUI->deselectDrawable(draw);
				}
			}
			else
			{
				if (!addToGroup)
				{
					deselectAll();
				}

				GameMessage *newMsg = TheMessageStream->appendMessage(GameMessage::MSG_CREATE_SELECTED_GROUP);
				newMsg->appendBooleanArgument(!addToGroup);

				Player *localPlayer = ThePlayerList->getLocalPlayer();

				Int newDrawablesSelected = 0;
				Drawable *draw = NULL;
				DrawableListIt it;
				for (it = drawablesThatWillSelect.begin(); it != drawablesThatWillSelect.end(); ++it)
				{
					draw = *it;
					if (!draw)
					{
						continue;
					}

					Object *obj = draw->getObject();
					if (!obj)
					{
						continue;
					}

					if (obj && obj->getContainedBy() != NULL)
					{
						// we're contained, and so we shouldn't be selectable.
						continue;
					}

					Drawable *drawToSelect = NULL;
					ObjectID objToAppend = INVALID_ID;
					if (si.selectMine && obj->isLocallyControlled())
					{
						if (!obj->isKindOf(KINDOF_STRUCTURE) || si.selectMineBuildings)
						{
							drawToSelect = draw;
							objToAppend = obj->getID();
						}
					}
					else
					{
						Relationship rel = localPlayer->getRelationship(obj->getTeam());
						if (si.selectEnemies && rel == ENEMIES)
						{
							drawToSelect = draw;
							objToAppend = obj->getID();
						}
						else if (si.selectCivilians && rel == NEUTRAL)
						{
							drawToSelect = draw;
							objToAppend = obj->getID();
						}
						else if (si.selectFriends && rel == ALLIES)
						{
							drawToSelect = draw;
							objToAppend = obj->getID();
						}
					}

					if (drawToSelect && objToAppend != INVALID_ID)
					{
						newMsg->appendObjectIDArgument(objToAppend);
						TheInGameUI->selectDrawable(drawToSelect);
						++newDrawablesSelected;
					}
				}

				if( newDrawablesSelected > 1 )
				{
					localPlayer->getAcademyStats()->recordDragSelection();
				}

				if (newDrawablesSelected == 1 && draw)
				{


#if defined(RTS_DEBUG)


          if (m_HandOfGodSelectionMode && draw)
					{
						Object* obj = draw->getObject();
						if (obj)
						{
							TheAudio->addAudioEvent(&TheAudio->getMiscAudio()->m_noCanDoSound);
							GameMessage* msg = TheMessageStream->appendMessage( GameMessage::MSG_DEBUG_KILL_OBJECT );
							msg->appendObjectIDArgument(obj->getID());
						}
						disp = DESTROY_MESSAGE;
						break;
					}
					else

          if (TheHurtSelectionMode && draw)
					{
						Object* obj = draw->getObject();
						if (obj)
						{
							TheAudio->addAudioEvent(&TheAudio->getMiscAudio()->m_noCanDoSound);
							GameMessage* msg = TheMessageStream->appendMessage( GameMessage::MSG_DEBUG_HURT_OBJECT );
							msg->appendObjectIDArgument(obj->getID());
						}
						disp = DESTROY_MESSAGE;
						break;
					}

  #ifdef DEBUG_OBJECT_ID_EXISTS
					if (TheDebugSelectionMode && draw && draw->getObject())
					{
						if (TheObjectIDToDebug == 0)
						{
							TheObjectIDToDebug = draw->getObject()->getID();
							AsciiString msg;
							msg.format("Item %s %08x selected for debugging",draw->getTemplate()->getName().str(),TheObjectIDToDebug);
							UnicodeString msgu;
							msgu.translate(msg);
							TheInGameUI->message(msgu);
							disp = DESTROY_MESSAGE;
							break;
						}
					}
  #endif

#endif


#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
          if (m_HandOfGodSelectionMode && draw)
					{
						Object* obj = draw->getObject();
						if (obj)
						{
							TheAudio->addAudioEvent(&TheAudio->getMiscAudio()->m_noCanDoSound);
							GameMessage* msg = TheMessageStream->appendMessage( GameMessage::MSG_DEBUG_KILL_OBJECT );
							msg->appendObjectIDArgument(obj->getID());
						}
						disp = DESTROY_MESSAGE;
						break;
					}
#endif


        }
			}

			if (disp == DESTROY_MESSAGE)
				TheInGameUI->clearAttackMoveToMode();

			break;
		}

		//-----------------------------------------------------------------------------
		// Note that the raw left messages are only used to draw feedback now when
		// appropriate. All actual selection code takes place in
		// MSG_MOUSE_LEFT_CLICK & MSG_MOUSE_LEFT_DOUBLE_CLICK
		case GameMessage::MSG_RAW_MOUSE_LEFT_BUTTON_DOWN:
		{
			// cannot actually start area selection yet - have to wait for cursor to move a bit
			m_leftMouseButtonIsDown = true;
			m_selectFeedbackAnchor = msg->getArgument( 0 )->pixel;
			break;
		}

		//-----------------------------------------------------------------------------
		// Note that the raw left messages are only used to draw feedback now when
		// appropriate. All actual selection code takes place in
		// MSG_MOUSE_LEFT_CLICK & MSG_MOUSE_LEFT_DOUBLE_CLICK
		case GameMessage::MSG_RAW_MOUSE_LEFT_BUTTON_UP:
		{
			m_leftMouseButtonIsDown = FALSE;

			if (m_dragSelecting) {
				// Stop drag selecting now, thanks.
				m_dragSelecting = FALSE;

				TheTacticalView->setMouseLock( FALSE );
				TheInGameUI->setSelecting( FALSE );
				TheInGameUI->endAreaSelectHint(NULL);

				// insert area selection message into stream
				GameMessage *dragMsg = TheMessageStream->appendMessage( GameMessage::MSG_AREA_SELECTION );

				IRegion2D selectionRegion;
				buildRegion( &m_selectFeedbackAnchor, &msg->getArgument(0)->pixel, &selectionRegion );
				dragMsg->appendPixelRegionArgument( selectionRegion );
			}
			else
			{
				// left click behavior (not right drag)

				//Added support to cancel the GUI command without deselecting the unit(s) involved
				//when you right click.
				if( !TheInGameUI->getGUICommand() && !TheKeyboard->isShift() && !TheKeyboard->isCtrl() && !TheKeyboard->isAlt() )
				{
					//No GUI command mode, so deselect everyone if we're in alternate mouse mode.
					if( TheGlobalData->m_useAlternateMouse && TheInGameUI->getPendingPlaceSourceObjectID() == INVALID_ID )
					{
						if( !TheInGameUI->getPreventLeftClickDeselectionInAlternateMouseModeForOneClick() )
						{
							deselectAll();
							m_lastGroupSelGroup = -1;
						}
						else
						{
							//Prevent deselection of unit if it just issued some type of UI order such as attack move, guard,
							//initiating construction of a new structure.
							TheInGameUI->setPreventLeftClickDeselectionInAlternateMouseModeForOneClick( FALSE );
						}
					}
				}
			}

			break;
		}

		//-----------------------------------------------------------------------------
		case GameMessage::MSG_RAW_MOUSE_RIGHT_BUTTON_DOWN:
		{
			// There are three ways in which we can ignore this as a deselect:
			// 1) 2-D position on screen
			// 2) Time has exceeded the time which we allow for this to be a click.
			// 3) 3-D camera position has changed
			m_deselectFeedbackAnchor = msg->getArgument( 0 )->pixel;
			m_lastClick = (UnsignedInt) msg->getArgument( 2 )->integer;
			TheTacticalView->getPosition(&m_deselectDownCameraPosition);

			break;
		}

		//-----------------------------------------------------------------------------
		case GameMessage::MSG_RAW_MOUSE_RIGHT_BUTTON_UP:
		{
			Coord3D cameraPos;
			TheTacticalView->getPosition(&cameraPos);
			cameraPos.sub(&m_deselectDownCameraPosition);

			ICoord2D pixel = msg->getArgument( 0 )->pixel;
			UnsignedInt currentTime = (UnsignedInt) msg->getArgument( 2 )->integer;

			Bool isClick = TheMouse->isClick(&m_deselectFeedbackAnchor, &pixel, m_lastClick, currentTime);

			if (isClick &&
					cameraPos.length() > TheMouse->m_dragTolerance3D)
			{
				isClick = FALSE;
			}

			// right click behavior (not right drag)
			if (isClick)
			{
				//Added support to cancel the GUI command without deselecting the unit(s) involved
				//when you right click.
				if( TheInGameUI->getGUICommand() )
				{
					//Cancel GUI command mode... don't deselect units.
					TheInGameUI->setGUICommand( NULL );

					//With a GUI command cancel, we want no other behavior.
					disp = DESTROY_MESSAGE;
					TheInGameUI->setScrolling( FALSE );
				}
				else
				{
					//In alternate mouse mode, right click still cancels building placement.
					// TheSuperHackers @tweak Stubbjax 08/08/2025 Cancelling building placement no longer deselects the builder.
					if (TheInGameUI->getPendingPlaceSourceObjectID() != INVALID_ID)
					{
						TheInGameUI->placeBuildAvailable(NULL, NULL);
						TheInGameUI->setPreventLeftClickDeselectionInAlternateMouseModeForOneClick(FALSE);
						disp = DESTROY_MESSAGE;
						TheInGameUI->setScrolling(FALSE);
					}
					else if (!TheGlobalData->m_useAlternateMouse)
					{
						//No GUI command mode, so deselect everyone if we're in regular mouse mode.
						deselectAll();
					}
				}
			}

			break;
		}

		//-----------------------------------------------------------------------------
		case GameMessage::MSG_META_CREATE_TEAM0:
		case GameMessage::MSG_META_CREATE_TEAM1:
		case GameMessage::MSG_META_CREATE_TEAM2:
		case GameMessage::MSG_META_CREATE_TEAM3:
		case GameMessage::MSG_META_CREATE_TEAM4:
		case GameMessage::MSG_META_CREATE_TEAM5:
		case GameMessage::MSG_META_CREATE_TEAM6:
		case GameMessage::MSG_META_CREATE_TEAM7:
		case GameMessage::MSG_META_CREATE_TEAM8:
		case GameMessage::MSG_META_CREATE_TEAM9:
		{
			Int group = t - GameMessage::MSG_META_CREATE_TEAM0;
			if ( group >= 0 && group < 10 )
			{
				DEBUG_LOG(("META: create team %d",group));
				// Assign selected items to a group
				GameMessage *newmsg = TheMessageStream->appendMessage((GameMessage::Type)(GameMessage::MSG_CREATE_TEAM0 + group));
				Drawable *drawable = TheGameClient->getDrawableList();
				while (drawable != NULL)
				{
					if (drawable->isSelected() && drawable->getObject() && drawable->getObject()->isLocallyControlled())
					{
						newmsg->appendObjectIDArgument(drawable->getObject()->getID());
					}
					drawable = drawable->getNextDrawable();
				}
			}
			disp = DESTROY_MESSAGE;
			break;
		}

		//-----------------------------------------------------------------------------
		case GameMessage::MSG_META_SELECT_TEAM0:
		case GameMessage::MSG_META_SELECT_TEAM1:
		case GameMessage::MSG_META_SELECT_TEAM2:
		case GameMessage::MSG_META_SELECT_TEAM3:
		case GameMessage::MSG_META_SELECT_TEAM4:
		case GameMessage::MSG_META_SELECT_TEAM5:
		case GameMessage::MSG_META_SELECT_TEAM6:
		case GameMessage::MSG_META_SELECT_TEAM7:
		case GameMessage::MSG_META_SELECT_TEAM8:
		case GameMessage::MSG_META_SELECT_TEAM9:
		{
			Int group = t - GameMessage::MSG_META_SELECT_TEAM0;
			if ( group >= 0 && group < 10 )
			{
				DEBUG_LOG(("META: select team %d",group));

				UnsignedInt now = TheGameLogic->getFrame();
				if ( m_lastGroupSelTime == 0 )
				{
					m_lastGroupSelTime = now;
				}

				Bool performSelection = TRUE;

				// check for double-press to jump view
				if ( now - m_lastGroupSelTime < 20 && group == m_lastGroupSelGroup )
				{
					DEBUG_LOG(("META: DOUBLETAP select team %d",group));
					// TheSuperHackers @bugfix Stubbjax 26/05/2025 Perform selection on double-press
					// if the group or part of it is somehow deselected between presses.
					performSelection = FALSE;
					Player *player = ThePlayerList->getLocalPlayer();
					if (player)
					{
						Squad *selectedSquad = player->getHotkeySquad(group);
						if (selectedSquad != NULL)
						{
							VecObjectPtr objlist = selectedSquad->getLiveObjects();
							Int numObjs = objlist.size();
							if (numObjs > 0)
							{
								// if theres someone in the group, center the camera on them.
								Drawable* drawable = objlist[numObjs - 1]->getDrawable();
								TheTacticalView->lookAt( drawable->getPosition() );
								performSelection = !TheInGameUI->areAllObjectsSelected( objlist );
							}
						}
					}
				}

				if ( performSelection )
				{
					TheInGameUI->deselectAllDrawables( false ); //No need to post message because we're just creating a new group!

					// no need to send two messages for selecting the same group.
					TheMessageStream->appendMessage((GameMessage::Type)(GameMessage::MSG_SELECT_TEAM0 + group));
					Player *player = ThePlayerList->getLocalPlayer();
					if (player)
					{
						Squad *selectedSquad = player->getHotkeySquad(group);
						if (selectedSquad != NULL)
						{
							VecObjectPtr objlist = selectedSquad->getLiveObjects();
							Int numObjs = objlist.size();
							for (Int i = 0; i < numObjs; ++i)
							{
								if( objlist[i]->getControllingPlayer() == player )
								{
									TheInGameUI->selectDrawable(objlist[i]->getDrawable());
								}
							}
						}
					}
				}
				m_lastGroupSelTime = now;
				m_lastGroupSelGroup = group;
			}
			disp = DESTROY_MESSAGE;
			break;
		}

		case GameMessage::MSG_META_ADD_TEAM0:
		case GameMessage::MSG_META_ADD_TEAM1:
		case GameMessage::MSG_META_ADD_TEAM2:
		case GameMessage::MSG_META_ADD_TEAM3:
		case GameMessage::MSG_META_ADD_TEAM4:
		case GameMessage::MSG_META_ADD_TEAM5:
		case GameMessage::MSG_META_ADD_TEAM6:
		case GameMessage::MSG_META_ADD_TEAM7:
		case GameMessage::MSG_META_ADD_TEAM8:
		case GameMessage::MSG_META_ADD_TEAM9:
		{
			Int group = t - GameMessage::MSG_META_ADD_TEAM0;
			if ( group >= 0 && group < 10 )
			{
				DEBUG_LOG(("META: select team %d",group));

				UnsignedInt now = TheGameLogic->getFrame();
				if ( m_lastGroupSelTime == 0 )
				{
					m_lastGroupSelTime = now;
				}

				// check for double-press to jump view

				if ( now - m_lastGroupSelTime < 20 && group == m_lastGroupSelGroup )
				{
					DEBUG_LOG(("META: DOUBLETAP select team %d",group));
					Player *player = ThePlayerList->getLocalPlayer();
					if (player)
					{
						Squad *selectedSquad = player->getHotkeySquad(group);
						if (selectedSquad != NULL)
						{
							VecObjectPtr objlist = selectedSquad->getLiveObjects();
							Int numObjs = objlist.size();
							if (numObjs > 0)
							{
								// if theres someone in the group, center the camera on them.
								TheTacticalView->lookAt( objlist[numObjs-1]->getDrawable()->getPosition() );
							}
						}
					}

				}
				else
				{

					Drawable *draw = TheInGameUI->getFirstSelectedDrawable();
					if( draw && draw->isKindOf( KINDOF_STRUCTURE ) )
					{
						//Kris: Jan 12, 2005
						//Can't select other units if you have a structure selected. So deselect the structure to prevent
						//group force attack exploit.
						TheInGameUI->deselectAllDrawables();
					}

					// no need to send two messages for selecting the same group.
					TheMessageStream->appendMessage((GameMessage::Type)(GameMessage::MSG_ADD_TEAM0 + group));
					Player *player = ThePlayerList->getLocalPlayer();
					if (player)
					{
						Squad *selectedSquad = player->getHotkeySquad(group);
						if (selectedSquad != NULL)
						{
							VecObjectPtr objlist = selectedSquad->getLiveObjects();
							Int numObjs = objlist.size();
							for (Int i = 0; i < numObjs; ++i)
							{
								TheInGameUI->selectDrawable(objlist[i]->getDrawable());
							}
						}
					}
				}
				m_lastGroupSelTime = now;
				m_lastGroupSelGroup = group;
			}
			disp = DESTROY_MESSAGE;
			break;
		}

		//-----------------------------------------------------------------------------
		case GameMessage::MSG_META_VIEW_TEAM0:
		case GameMessage::MSG_META_VIEW_TEAM1:
		case GameMessage::MSG_META_VIEW_TEAM2:
		case GameMessage::MSG_META_VIEW_TEAM3:
		case GameMessage::MSG_META_VIEW_TEAM4:
		case GameMessage::MSG_META_VIEW_TEAM5:
		case GameMessage::MSG_META_VIEW_TEAM6:
		case GameMessage::MSG_META_VIEW_TEAM7:
		case GameMessage::MSG_META_VIEW_TEAM8:
		case GameMessage::MSG_META_VIEW_TEAM9:
		{
			Int group = t - GameMessage::MSG_META_VIEW_TEAM0;
			if ( group >= 1 && group <= 10 )
			{
				DEBUG_LOG(("META: view team %d",group));
				Player *player = ThePlayerList->getLocalPlayer();
				if (player)
				{
					Squad *selectedSquad = player->getHotkeySquad(group);
					if (selectedSquad != NULL)
					{
						VecObjectPtr objlist = selectedSquad->getLiveObjects();
						Int numObjs = objlist.size();
						if (numObjs > 0)
						{
							// if theres someone in the group, center the camera on them.
							TheTacticalView->lookAt( objlist[ numObjs-1 ]->getDrawable()->getPosition() );
						}
					}
				}
			}
			disp = DESTROY_MESSAGE;
			break;
		}

		//-----------------------------------------------------------------------------------------
		case GameMessage::MSG_META_OPTIONS:
		{
			// stop drawing selection feedback, as we're going to ignore the selection.
			m_leftMouseButtonIsDown = FALSE;
			// let this message drop through, the commandXLat will show the options screen itself.
			break;
		}


#if defined(RTS_DEBUG)
		//-----------------------------------------------------------------------------------------
		case GameMessage::MSG_META_DEMO_TOGGLE_HAND_OF_GOD_MODE:
		{
			if ( !TheGameLogic->isInMultiplayerGame() )
			{
				m_HandOfGodSelectionMode = !m_HandOfGodSelectionMode;
				TheInGameUI->message( UnicodeString( L"Meta Hand-Of-God Mode is %s" ), m_HandOfGodSelectionMode ? L"ON" : L"OFF" );
				disp = DESTROY_MESSAGE;
			}
			break;
		}
#endif

#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
		//-----------------------------------------------------------------------------------------
		case GameMessage::MSG_CHEAT_TOGGLE_HAND_OF_GOD_MODE://NOTICE THE DIFFERENT NAME!!!!!!!!!!!!!!!!!!!!!!!!!!ML
		{
			if ( !TheGameLogic->isInMultiplayerGame() )
			{
				m_HandOfGodSelectionMode = !m_HandOfGodSelectionMode;
				TheInGameUI->message( UnicodeString( L"Hand-Of-God Mode is %s" ), m_HandOfGodSelectionMode ? L"ON" : L"OFF" );
				disp = DESTROY_MESSAGE;
			}
			break;
		}
#endif

#if defined(RTS_DEBUG)
		//-----------------------------------------------------------------------------------------
		case GameMessage::MSG_META_DEMO_TOGGLE_HURT_ME_MODE:
		{
			if ( !TheGameLogic->isInMultiplayerGame() )
			{
				TheHurtSelectionMode = !TheHurtSelectionMode;
				TheInGameUI->message( UnicodeString( L"Hurt-Me Mode is %s" ), TheHurtSelectionMode ? L"ON" : L"OFF" );
				disp = DESTROY_MESSAGE;
			}
			break;
		}
#endif

#if defined(RTS_DEBUG)
		//-----------------------------------------------------------------------------------------
		case GameMessage::MSG_META_DEMO_DEBUG_SELECTION:
		{
			TheDebugSelectionMode = !TheDebugSelectionMode;
			TheInGameUI->message( UnicodeString( L"Debug-Selected-Item Mode is %s" ), TheDebugSelectionMode ? L"ON" : L"OFF" );
		#ifdef DEBUG_OBJECT_ID_EXISTS
			TheObjectIDToDebug = INVALID_ID;
		#endif
			disp = DESTROY_MESSAGE;
			break;
		}
#endif
	}

	return disp;
}


//Added By Sadullah Nader

//setDragSelecting(Bool dragSelect)
//Added to fix the drag selection problem in control bar
////////////////////////////////////////////////////////////////////////
void SelectionTranslator::setDragSelecting(Bool dragSelect)
{
	m_dragSelecting = dragSelect;
}

//setLeftMouseButton(Bool state)
//Added to turn of Left button down when left button goes up
////////////////////////////////////////////////////////////////////////
void SelectionTranslator::setLeftMouseButton(Bool state)
{
	m_leftMouseButtonIsDown = state;
}
