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

// FILE: GUIUtil.cpp //////////////////////////////////////////////////////
// Author: Matthew D. Campbell, Sept 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameNetwork/GUIUtil.h"
#include "GameNetwork/NetworkDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/MapUtil.h"
#include "Common/NameKeyGenerator.h"

#include "Common/MultiplayerSettings.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GameText.h"
#include "GameLogic/GameLogic.h" // SUPERWEAPON_RESTRICT_COUNT
#include "GameNetwork/GameInfo.h"
#include "Common/PlayerTemplate.h"
#include "GameNetwork/LANAPICallbacks.h" // for acceptTrueColor, etc
#include "GameClient/ChallengeGenerals.h"


// -----------------------------------------------------------------------------

static Bool winInitialized = FALSE;

void EnableSlotListUpdates( Bool val )
{
	winInitialized = val;
}

Bool AreSlotListUpdatesEnabled( void )
{
	return winInitialized;
}

// -----------------------------------------------------------------------------

void EnableAcceptControls(Bool Enabled, GameInfo *myGame, GameWindow *comboPlayer[],
										GameWindow *comboColor[], GameWindow *comboPlayerTemplate[],
										GameWindow *comboTeam[], GameWindow *buttonAccept[], GameWindow *buttonStart,
										GameWindow *buttonMapStartPosition[], Int slotNum)
{
	if(slotNum == -1 || slotNum >= MAX_SLOTS )
		slotNum = myGame->getLocalSlotNum();

	Bool isObserver = myGame->getConstSlot(slotNum)->getPlayerTemplate() == PLAYERTEMPLATE_OBSERVER;

	if( !myGame->amIHost() && (buttonStart != NULL) )
		buttonStart->winEnable(Enabled);
	if(comboColor[slotNum])
	{
		if (isObserver)
		{
			GadgetComboBoxHideList(comboColor[slotNum]);
		}
		comboColor[slotNum]->winEnable(Enabled && !isObserver);
	}
	if(comboPlayerTemplate[slotNum])
		comboPlayerTemplate[slotNum]->winEnable(Enabled);
	if(comboTeam[slotNum])
	{
		if (isObserver)
		{
			GadgetComboBoxHideList(comboTeam[slotNum]);
		}
		comboTeam[slotNum]->winEnable(Enabled && !isObserver);
	}

	Bool canChooseStartSpot = FALSE;
	if (!isObserver)
		canChooseStartSpot = TRUE;
	for (Int i=0; i<MAX_SLOTS && !canChooseStartSpot && myGame->amIHost(); ++i)
	{
		if (myGame->getConstSlot(i) && myGame->getConstSlot(i)->isAI())
			canChooseStartSpot = TRUE;
	}

	if (slotNum == myGame->getLocalSlotNum())
	{
		if (myGame->getConstSlot(myGame->getLocalSlotNum())->hasMap())
		{
			for (Int i=0; i<MAX_SLOTS; ++i)
			{
				if (buttonMapStartPosition[i])
				{
					buttonMapStartPosition[i]->winEnable(Enabled && canChooseStartSpot);
				}
			}
		}
		else
		{
			for (Int i=0; i<MAX_SLOTS; ++i)
			{
				if (buttonMapStartPosition[i])
					buttonMapStartPosition[i]->winEnable(FALSE);
			}
		}
	}
}

// -----------------------------------------------------------------------------

void ShowUnderlyingGUIElements( Bool show, const char *layoutFilename, const char *parentName,
															 const char **gadgetsToHide, const char **perPlayerGadgetsToHide )
{
	AsciiString parentNameStr;
	parentNameStr.format("%s:%s", layoutFilename, parentName);
	NameKeyType parentID = NAMEKEY(parentNameStr);
	GameWindow *parent = TheWindowManager->winGetWindowFromId( NULL, parentID );
	if (!parent)
	{
		DEBUG_CRASH(("Window %s not found\n", parentNameStr.str()));
		return;
	}

	// hide some GUI elements of the screen underneath
	GameWindow *win;

	Int player;
	const char **text;

	text = gadgetsToHide;
	while (*text)
	{
		AsciiString gadgetName;
		gadgetName.format("%s:%s", layoutFilename, *text);
		win	= TheWindowManager->winGetWindowFromId( parent, NAMEKEY(gadgetName) );
		//DEBUG_ASSERTCRASH(win, ("Cannot find %s to show/hide it", gadgetName.str()));
		if (win)
		{
			win->winHide( !show );
		}
		++text;
	}

	text = perPlayerGadgetsToHide;
	while (*text)
	{
		for (player = 0; player < MAX_SLOTS; ++player)
		{
			AsciiString gadgetName;
			gadgetName.format("%s:%s%d", layoutFilename, *text, player);
			win	= TheWindowManager->winGetWindowFromId( parent, NAMEKEY(gadgetName) );
			//DEBUG_ASSERTCRASH(win, ("Cannot find %s to show/hide it", gadgetName.str()));
			if (win)
			{
				win->winHide( !show );
			}
		}
		++text;
	}
}

// -----------------------------------------------------------------------------

void PopulateColorComboBox(Int comboBox, GameWindow *comboArray[], GameInfo *myGame, Bool isObserver)
{
	Int numColors = TheMultiplayerSettings->getNumColors();
	UnicodeString colorName;
	std::vector<bool> availableColors;

	Int i = 0;
	for (; i < numColors; i++)
		availableColors.push_back(true);

	for (i = 0; i < MAX_SLOTS; i++)
	{
		GameSlot *slot = myGame->getSlot(i);	
		if( slot && (i != comboBox) && (slot->getColor() >= 0 )&& (slot->getColor() < numColors))
		{
			DEBUG_ASSERTCRASH(slot->getColor() >= 0,("We've tried to access array %d and that ain't good",slot->getColor()));
			availableColors[slot->getColor()] = false;
		}
	}

	Bool wasObserver = (GadgetComboBoxGetLength(comboArray[comboBox]) == 1);
	GadgetComboBoxReset(comboArray[comboBox]);

	MultiplayerColorDefinition *def = TheMultiplayerSettings->getColor(PLAYERTEMPLATE_RANDOM);
	Int newIndex = GadgetComboBoxAddEntry(comboArray[comboBox],
		(isObserver)?TheGameText->fetch("GUI:None"):TheGameText->fetch("GUI:???"), def->getColor());
	GadgetComboBoxSetItemData(comboArray[comboBox], newIndex, (void *)-1);

	if (isObserver)
	{
		GadgetComboBoxSetSelectedPos(comboArray[comboBox], 0);
		return;
	}

	for (Int c=0; c<numColors; ++c)
	{
		def = TheMultiplayerSettings->getColor(c);
		if (!def || availableColors[c] == false)
			continue;

		colorName = TheGameText->fetch(def->getTooltipName().str());
		newIndex = GadgetComboBoxAddEntry(comboArray[comboBox], colorName, def->getColor());
		GadgetComboBoxSetItemData(comboArray[comboBox], newIndex, (void *)c);
	}
	if (wasObserver)
		GadgetComboBoxSetSelectedPos(comboArray[comboBox], 0);
}

// -----------------------------------------------------------------------------

void PopulatePlayerTemplateComboBox(Int comboBox, GameWindow *comboArray[], GameInfo *myGame, Bool allowObservers)
{
	Int numPlayerTemplates = ThePlayerTemplateStore->getPlayerTemplateCount();
	UnicodeString playerTemplateName;

	GadgetComboBoxReset(comboArray[comboBox]);

	MultiplayerColorDefinition *def = TheMultiplayerSettings->getColor(PLAYERTEMPLATE_RANDOM);
	Int newIndex = GadgetComboBoxAddEntry(comboArray[comboBox], TheGameText->fetch("GUI:Random"), def->getColor());
	GadgetComboBoxSetItemData(comboArray[comboBox], newIndex, (void *)PLAYERTEMPLATE_RANDOM);

	std::set<AsciiString> seenSides;

	for (Int c=0; c<numPlayerTemplates; ++c)
	{
		const PlayerTemplate *fac = ThePlayerTemplateStore->getNthPlayerTemplate(c);
		if (!fac)
			continue;

		if (fac->getStartingBuilding().isEmpty())
			continue;

		if ( myGame->oldFactionsOnly() && !fac->isOldFaction() )
		  continue;

		// Prevent players from selecting the disabled Generals for use.
		// This is also enforced at game loading (GameLogic.cpp and UserPreferences.cpp).
		// @todo: unlock these when something rad happens
		Bool disallowLockedGenerals = TRUE;
		const GeneralPersona *general = TheChallengeGenerals->getGeneralByTemplateName(fac->getName());
		Bool startsLocked = general ? !general->isStartingEnabled() : FALSE;
		if (disallowLockedGenerals && startsLocked)
			continue;


		AsciiString side;
		side.format("SIDE:%s", fac->getSide().str());
		if (seenSides.find(side) != seenSides.end())
			continue;

		seenSides.insert(side);

		newIndex = GadgetComboBoxAddEntry(comboArray[comboBox], TheGameText->fetch(side), def->getColor());
		GadgetComboBoxSetItemData(comboArray[comboBox], newIndex, (void *)c);
	}
	seenSides.clear();

	// disabling observers for Multiplayer test
	if (allowObservers)
	{
		def = TheMultiplayerSettings->getColor(PLAYERTEMPLATE_OBSERVER);
		newIndex = GadgetComboBoxAddEntry(comboArray[comboBox], TheGameText->fetch("GUI:Observer"), def->getColor());
		GadgetComboBoxSetItemData(comboArray[comboBox], newIndex, (void *)PLAYERTEMPLATE_OBSERVER);
	}
	GadgetComboBoxSetSelectedPos(comboArray[comboBox], 0);

}

// -----------------------------------------------------------------------------

void PopulateTeamComboBox(Int comboBox, GameWindow *comboArray[], GameInfo *myGame, Bool isObserver)
{
	Int numTeams = MAX_SLOTS/2;
	UnicodeString teamName;

	GadgetComboBoxReset(comboArray[comboBox]);

	MultiplayerColorDefinition *def = TheMultiplayerSettings->getColor(PLAYERTEMPLATE_RANDOM);
	Int newIndex = GadgetComboBoxAddEntry(comboArray[comboBox], TheGameText->fetch("Team:0"), def->getColor());
	GadgetComboBoxSetItemData(comboArray[comboBox], newIndex, (void *)-1);

	if (isObserver)
	{
		GadgetComboBoxSetSelectedPos(comboArray[comboBox], 0);
		return;
	}

	for (Int c=0; c<numTeams; ++c)
	{
		AsciiString teamStr;
		teamStr.format("Team:%d", c + 1);
		teamName = TheGameText->fetch(teamStr.str());
		newIndex = GadgetComboBoxAddEntry(comboArray[comboBox], teamName, def->getColor());
		GadgetComboBoxSetItemData(comboArray[comboBox], newIndex, (void *)c);
	}
	GadgetComboBoxSetSelectedPos(comboArray[comboBox], 0);
}

// -----------------------------------------------------------------------------
static UnicodeString formatMoneyForStartingCashComboBox( const Money & moneyAmount )
{
  UnicodeString rtn;
  rtn.format( TheGameText->fetch( "GUI:StartingMoneyFormat" ), moneyAmount.countMoney() );
  return rtn;
}

void PopulateStartingCashComboBox(GameWindow *comboBox, GameInfo *myGame)
{
  GadgetComboBoxReset(comboBox);

  const MultiplayerStartingMoneyList & startingCashMap = TheMultiplayerSettings->getStartingMoneyList(); 
  Int currentSelectionIndex = -1;

  MultiplayerStartingMoneyList::const_iterator it = startingCashMap.begin();
  for ( ; it != startingCashMap.end(); it++ )
  {
    Int newIndex = GadgetComboBoxAddEntry(comboBox, formatMoneyForStartingCashComboBox( *it ), 
                                          comboBox->winGetEnabled() ? comboBox->winGetEnabledTextColor() : comboBox->winGetDisabledTextColor());
    GadgetComboBoxSetItemData(comboBox, newIndex, (void *)it->countMoney());

    if ( myGame->getStartingCash().amountEqual( *it ) )
    {
      currentSelectionIndex = newIndex;
    }
  }

  if ( currentSelectionIndex == -1 )
  {
    DEBUG_CRASH( ("Current selection for starting cash not found in list") );
    currentSelectionIndex = GadgetComboBoxAddEntry(comboBox, formatMoneyForStartingCashComboBox( myGame->getStartingCash() ), 
                                          comboBox->winGetEnabled() ? comboBox->winGetEnabledTextColor() : comboBox->winGetDisabledTextColor());
    GadgetComboBoxSetItemData(comboBox, currentSelectionIndex, (void *)it->countMoney() );
  }

  GadgetComboBoxSetSelectedPos(comboBox, currentSelectionIndex);
}

// -----------------------------------------------------------------------------

//  -----------------------------------------------------------------------------------------
// The slot list displaying function
//-------------------------------------------------------------------------------------------------
void UpdateSlotList( GameInfo *myGame, GameWindow *comboPlayer[],
										GameWindow *comboColor[], GameWindow *comboPlayerTemplate[],
										GameWindow *comboTeam[], GameWindow *buttonAccept[], 
										GameWindow *buttonStart, GameWindow *buttonMapStartPosition[] )
{
	if(!AreSlotListUpdatesEnabled())
		return;
	//LANGameInfo *myGame = TheLAN->GetMyGame();

	const MapMetaData *mapData = TheMapCache->findMap( myGame->getMap() );
	Bool willTransfer = TRUE;
	if (mapData)
	{
		willTransfer = !mapData->m_isOfficial;
	}
	else
	{
		willTransfer = WouldMapTransfer(myGame->getMap());
	}

	if (myGame)
	{
		for( int i =0; i < MAX_SLOTS; i++ )
		{
			GameSlot * slot = myGame->getSlot(i);

			// if i'm host, enable the controls for AI
			if(myGame->amIHost() && slot->isAI())
			{
				EnableAcceptControls(TRUE, myGame, comboPlayer, comboColor, comboPlayerTemplate,
					comboTeam, buttonAccept, buttonStart, buttonMapStartPosition, i);
			}
			else if (myGame->getLocalSlotNum() == i)
			{
				if(slot->isAccepted() && !myGame->amIHost())
				{
					EnableAcceptControls(FALSE, myGame, comboPlayer, comboColor, comboPlayerTemplate,
						comboTeam, buttonAccept, buttonStart, buttonMapStartPosition);
				}
				else
				{
					if (slot->hasMap()) {
						EnableAcceptControls(TRUE, myGame, comboPlayer, comboColor, comboPlayerTemplate,
							comboTeam, buttonAccept, buttonStart, buttonMapStartPosition);
					}
					else
					{
						EnableAcceptControls(willTransfer, myGame, comboPlayer, comboColor, comboPlayerTemplate,
							comboTeam, buttonAccept, buttonStart, buttonMapStartPosition);
					}
				}
				
			}
			else if(myGame->amIHost())
			{
				EnableAcceptControls(FALSE, myGame, comboPlayer, comboColor, comboPlayerTemplate,
					comboTeam, buttonAccept, buttonStart, buttonMapStartPosition, i);
			}
			if(slot->isHuman())
			{
				UnicodeString newName = slot->getName();
				UnicodeString oldName = GadgetComboBoxGetText(comboPlayer[i]);
				if (comboPlayer[i] && newName.compare(oldName))
				{
					GadgetComboBoxSetText(comboPlayer[i], newName);
				}
				if(i!= 0 && buttonAccept && buttonAccept[i])
				{
					buttonAccept[i]->winHide(FALSE);
				//Color In the little accepted boxes
					if(slot->isAccepted())
					{
						if(BitIsSet(buttonAccept[i]->winGetStatus(), WIN_STATUS_IMAGE	))
							buttonAccept[i]->winEnable(TRUE);
						else
							GadgetButtonSetEnabledColor(buttonAccept[i], acceptTrueColor );
					}
					else
					{
						if(BitIsSet(buttonAccept[i]->winGetStatus(), WIN_STATUS_IMAGE	))
							buttonAccept[i]->winEnable(FALSE);
						else
							GadgetButtonSetEnabledColor(buttonAccept[i], acceptFalseColor );
					}
				}
			}
			else
			{				
				GadgetComboBoxSetSelectedPos(comboPlayer[i], slot->getState(), TRUE);
        if( buttonAccept &&  buttonAccept[i] )
				  buttonAccept[i]->winHide(TRUE);
			}
/*
			if (myGame->getLocalSlotNum() == i && i!=0)
			{
				if (comboPlayer[i])
					comboPlayer[i]->winEnable( TRUE );
			}
			else*/ if (!myGame->amIHost())
			{
				if (comboPlayer[i])
					comboPlayer[i]->winEnable( FALSE );
			}
			//if( i == myGame->getLocalSlotNum())
      if((comboColor[i] != NULL) && BitIsSet(comboColor[i]->winGetStatus(), WIN_STATUS_ENABLED))
				PopulateColorComboBox(i, comboColor, myGame, myGame->getConstSlot(i)->getPlayerTemplate() == PLAYERTEMPLATE_OBSERVER);
			Int max, idx;
			if (comboColor[i] != NULL) {
				max = GadgetComboBoxGetLength(comboColor[i]);
				for (idx=0; idx<max; ++idx)
				{
					Int color = (Int)GadgetComboBoxGetItemData(comboColor[i], idx);
					if (color == slot->getColor())
					{
						GadgetComboBoxSetSelectedPos(comboColor[i], idx, TRUE);
						break;
					}
				}
			}

			if (comboTeam[i] != NULL) {
				max = GadgetComboBoxGetLength(comboTeam[i]);
				for (idx=0; idx<max; ++idx)
				{
					Int team = (Int)GadgetComboBoxGetItemData(comboTeam[i], idx);
					if (team == slot->getTeamNumber())
					{
						GadgetComboBoxSetSelectedPos(comboTeam[i], idx, TRUE);
						break;
					}
				}
			}

			if (comboPlayerTemplate[i] != NULL) {
				max = GadgetComboBoxGetLength(comboPlayerTemplate[i]);
				for (idx=0; idx<max; ++idx)
				{
					Int playerTemplate = (Int)GadgetComboBoxGetItemData(comboPlayerTemplate[i], idx);
					if (playerTemplate == slot->getPlayerTemplate())
					{
						GadgetComboBoxSetSelectedPos(comboPlayerTemplate[i], idx, TRUE);
						break;
					}
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------
