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

// FILE: ControlBarCommand.cpp ////////////////////////////////////////////////////////////////////
// Author: Colin Day, March 2002
// Desc:   Methods specific to the control bar unit commands
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/NameKeyGenerator.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/SpecialPower.h"
#include "Common/Upgrade.h"
#include "Common/BuildAssistant.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/BattlePlanUpdate.h"
#include "GameLogic/Module/DozerAIUpdate.h"
#include "GameLogic/Module/OverchargeBehavior.h"
#include "GameLogic/Module/ProductionUpdate.h"
#include "GameLogic/Module/SpecialPowerModule.h"
#include "GameLogic/Module/TransportContain.h"
#include "GameLogic/Module/MobNexusContain.h"
#include "GameLogic/Module/SpecialAbilityUpdate.h"
#include "GameLogic/Module/BattlePlanUpdate.h"
#include "GameLogic/Module/VeterancyGainCreate.h"
#include "GameLogic/Module/HackInternetAIUpdate.h"
#include "GameLogic/Weapon.h"

#include "GameClient/InGameUI.h"
#include "GameClient/Drawable.h"
#include "GameClient/ControlBar.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetPushButton.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
static GameWindow *commandWindows[ MAX_COMMANDS_PER_SET ];
Bool commandWindowsInitialized = FALSE;
static Color BuildClockColor = GameMakeColor(0,0,0,100);
// STATIC DATA STORAGE ////////////////////////////////////////////////////////////////////////////
ControlBar::ContainEntry ControlBar::m_containData[ MAX_COMMANDS_PER_SET ];

//-------------------------------------------------------------------------------------------------
/** Note, this iterate callback assumes that the inventory exit buttons appear in a
	* continuous order in the layout of the command set */
//-------------------------------------------------------------------------------------------------
struct PopulateInvButtonData
{
	Int currIndex;					 ///< index that represents the control we're talking about
	Int maxIndex;					   ///< this is the last valid control we can use
	GameWindow **controls;   ///< the controls
	Object *transport;			 ///< the transport
};

//-------------------------------------------------------------------------------------------------
/** Used for the callback iterator on transport contents to do the actual GUI fill */
//-------------------------------------------------------------------------------------------------
void ControlBar::populateInvDataCallback( Object *obj, void *userData )
{
	PopulateInvButtonData *data = (PopulateInvButtonData *)userData;

	//
	// if we're beyond the max the GUI can support, design needs to change the parameters
	// of the transport object to carry less things
	//
	if( data->currIndex > data->maxIndex )
	{

		DEBUG_ASSERTCRASH( 0, ("There is not enough GUI slots to hold the # of items inside a '%s'\n", 
													data->transport->getTemplate()->getName().str()) );
		return;

	}  // end if

	// get the window control that we're going to put our smiling faces in
	GameWindow *control = data->controls[ data->currIndex ];
	DEBUG_ASSERTCRASH( control, ("populateInvDataCallback: Control not found\n") );

	// assign our control and object id to the transport data
	m_containData[ data->currIndex ].control = control;
	m_containData[ data->currIndex ].objectID = obj->getID();
	data->currIndex++;

	// fill out the control enabled, hilite, and pushed images
	const Image *image;
	image = obj->getTemplate()->getButtonImage();
	GadgetButtonSetEnabledImage( control, image );

	//No longer used
	//image = TheMappedImageCollection->findImageByName( obj->getTemplate()->getInventoryImageName( INV_IMAGE_HILITE ) );
	//GadgetButtonSetHiliteImage( control, image );
	//image = TheMappedImageCollection->findImageByName( obj->getTemplate()->getInventoryImageName( INV_IMAGE_PUSHED ) );
	//GadgetButtonSetHiliteSelectedImage( control, image );

	//Show the contained object's veterancy symbol!
	image = calculateVeterancyOverlayForObject( obj );
	GadgetButtonDrawOverlayImage( control, image );

	// enable the control
	control->winEnable( TRUE );

}  // end populateInvDataCallback

//-------------------------------------------------------------------------------------------------
/** Transports have an extra special manipulation of the user interface.  They get to look
	* at the available command set, and any of the commands that are TransportExit commands *AND*
	* there is actually an object to represent that slot contained in the transport, the
	* inventory picture of the contained object will be displayed in the window control for
	* that TransportExit command.  Also, transports will HIDE any TransportExit controls found
	* in the command set that represent slots that *DO NOT EXIST* for the transport (that is,
	* the transport can only hold 4 things, but the GUI has buttons for 8 things).  For slots
	* that are empty but present in the transport the UI will show a disabled button to show
	* the user that there is an open "slot" */
//-------------------------------------------------------------------------------------------------
void ControlBar::doTransportInventoryUI( Object *transport, const CommandSet *commandSet )
{
/// @todo srj -- remove hard-coding here, please
	//static const CommandButton *exitCommand = findCommandButton( "Command_TransportExit" );
		
	// sanity
	if( transport == NULL || commandSet == NULL )
		return;

	// get the transport contain module
	ContainModuleInterface *contain = transport->getContain();
	
	// sanity
	if( contain == NULL )
		return;

	// how many slots do we have inside the transport
	Int transportMax = contain->getContainMax();

	//
	// first, hide any windows in 'm_commandWindows' that correspond to TransportExit commands
	// within the 'commandSet' that are overflow slots (the ui could be showing more inventory
	// exit button slots than there are slots in the transport)
	//
	// The extra slots bit means that a tank that takes up three slots will make two transport
	// buttons disappear off the end to show he takes up more room.
	transportMax = transportMax - contain->getExtraSlotsInUse();

	Int firstInventoryIndex = -1;
	Int lastInventoryIndex = -1;
	Int inventoryCommandCount = 0;

	const CommandButton *commandButton;
	for( Int i = 0; i < MAX_COMMANDS_PER_SET; i++ )
	{

		// get command button
		commandButton = commandSet->getCommandButton(i);

		// is this an inventory exit command
		if( commandButton && commandButton->getCommandType() == GUI_COMMAND_EXIT_CONTAINER )
		{

			// record the index of the control for the first inventory exit command we found
			if( firstInventoryIndex == -1 )
				firstInventoryIndex = i;

			//
			// since we're assuming all inventory exit commands appear in a continuous order,
			// we need to also need to keep track of what is the last valid inventory commadn index
			//
			lastInventoryIndex = i;

			// increment our count of available inventory exit commands found for the set
			inventoryCommandCount++;

			// show the window, but disable by default unless something is actually loaded in there
			m_commandWindows[ i ]->winHide( FALSE );
			m_commandWindows[ i ]->winEnable( FALSE );

			//Clear any potential veterancy rank, or else we'll see it when it's empty!
			GadgetButtonDrawOverlayImage( m_commandWindows[ i ], NULL );
			
			//Unmanned vehicles don't have any commands available -- in fact they are hidden!
 			if( transport->isDisabledByType( DISABLED_UNMANNED ) )
 			{
 				m_commandWindows[ i ]->winHide( TRUE );
 			}

			// if we've counted more UI spots than the transport can hold, hide this command window
			if( inventoryCommandCount > transportMax )
				m_commandWindows[ i ]->winHide( TRUE );

			//
			// set the inventory exit command into the window (even if it's one of the hidden ones
			// it's OK cause we'll never see it to click it
			//
			setControlCommand( m_commandWindows[ i ], commandButton );

		}  // end if

	}  // end for i
	
	// After Every change to the m_commandWIndows, we need to show fill in the missing blanks with the images
	// removed from multiplayer branch
	//showCommandMarkers();

	//
	// now, iterate the contained items of the transport and for each one we find we will
	// populate a user interface button with its inventory picture and store the inventory
	// data inside the control bar so we can respond to the button when its clicked
	//
	if( lastInventoryIndex >= 0 )  // just for sanity
	{
		PopulateInvButtonData data;

		data.controls = m_commandWindows;
		data.currIndex = firstInventoryIndex;
		data.maxIndex = lastInventoryIndex;
		data.transport = transport;
		contain->iterateContained( populateInvDataCallback, &data, FALSE );

	}  // end if

	//
	// save the last recorded inventory count so we know when we have to redo the gui when
	// something exits or enters
	//
	m_lastRecordedInventoryCount = contain->getContainCount();

}  // end doTransportInventoryUI

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ControlBar::populateCommand( Object *obj )
{
	const CommandSet *commandSet;
	Int i;
	Player *player = obj->getControllingPlayer();

	// reset contain data
	resetContainData();

	// reset the build queue data
	resetBuildQueueData();

	// get command set
	commandSet = TheControlBar->findCommandSet( obj->getCommandSetString() );

	// if no command set match is found hide all the buttons
	if( commandSet == NULL )
	{

		// hide all the buttons
		for( i = 0; i < MAX_COMMANDS_PER_SET; i++ )
			m_commandWindows[ i ]->winHide( TRUE );

		// nothing left to do
		return;

	}  // end if

	// transports do extra special things with the user interface buttons
	if( obj->getContain()  &&  obj->getContain()->isDisplayedOnControlBar() )
		doTransportInventoryUI( obj, commandSet );

	// populate the button with commands defined
	const CommandButton *commandButton;
	for( i = 0; i < MAX_COMMANDS_PER_SET; i++ )
	{

		// get command button
		commandButton = commandSet->getCommandButton(i);

		// if button is not present, just hide the window
		if( commandButton == NULL )
		{

			// hide window on interface
			m_commandWindows[ i ]->winHide( TRUE );

		}  // end if
		else
		{

			//Script only command -- don't show it in the UI.
			if( BitIsSet( commandButton->getOptions(), SCRIPT_ONLY ) )
			{
				m_commandWindows[ i ]->winHide( TRUE );
				continue;
			}
			//
			// inventory exit commands were a special case already taken care above ... we needed
			// to iterage through the containment for transport objects and fill out any available
			// inventory exit buttons on the UI
			//
			if( commandButton->getCommandType() != GUI_COMMAND_EXIT_CONTAINER )
			{

				// make sure the window is not hidden
				m_commandWindows[ i ]->winHide( FALSE );

				// enable by default
				m_commandWindows[ i ]->winEnable( TRUE );

				// populate the visible button with data from the command button
				setControlCommand( m_commandWindows[ i ], commandButton );

				//
				// commands that require sciences we don't have are hidden so they never show up
				// cause we can never pick "another" general technology throughout the game
				//
				if( BitIsSet( commandButton->getOptions(), NEED_SPECIAL_POWER_SCIENCE ) )
				{
					const SpecialPowerTemplate *power = commandButton->getSpecialPowerTemplate();

					if( power && power->getRequiredScience() != SCIENCE_INVALID )
					{
						if( player->hasScience( power->getRequiredScience() ) == FALSE )
						{
							//Hide the power
							m_commandWindows[ i ]->winHide( TRUE );
						}
						else
						{
							//The player does have the special power! Now determine if the images require
							//enhancement based on upgraded versions. This is determined by the command
							//button specifying a vector of sciences in the command button.
							Int bestIndex = -1;
							ScienceType science;
							for( Int scienceIndex = 0; scienceIndex < commandButton->getScienceVec().size(); ++scienceIndex )
							{
								science = commandButton->getScienceVec()[ scienceIndex ];
								
								//Keep going until we reach the end or don't have the required science!
								if( player->hasScience( science ) )
								{
									bestIndex = scienceIndex;
								}
								else
								{
									break;
								}
							}

							if( bestIndex != -1 )
							{
								//Now get the best sciencetype.
								science = commandButton->getScienceVec()[ bestIndex ];

								//Now we have to search through the command buttons to find a matching purchase science button.
								for( const CommandButton *command = m_commandButtons; command; command = command->getNext() )
								{
									if( command->getCommandType() == GUI_COMMAND_PURCHASE_SCIENCE )
									{
										//All purchase sciences specify a single science.
										if( command->getScienceVec().empty() )
										{
											DEBUG_CRASH( ("Commandbutton %s is a purchase science button without any science! Please add it.", command->getName().str() ) );
										}
										else if( command->getScienceVec()[0] == science )
										{
											commandButton->copyImagesFrom( command, true );
										}
									}
								}
							}
						}
					}

				}  // end if			
						
			}  // end else

		}  // end else

	}  // end for i

	// After Every change to the m_commandWIndows, we need to show fill in the missing blanks with the images
	// removed from multiplayer branch
	//showCommandMarkers();


	//
	// for objects that have a production exit interface, we may have a rally point set.
	// if we do, we want to show that rally point in the world
	//
	ExitInterface *exit = obj->getObjectExitInterface();
	if( exit )
	{

		//
		// if a rally point is set, show the rally point, if we don't have it set hide any rally
		// point we might have visible 
		//
		showRallyPoint( exit->getRallyPoint() );

	}  // end if

	//
	// to avoid a one frame delay where windows may become enabled/disabled, run the update
	// at once to get it all in the correct state immediately
	//
	updateContextCommand();

}  // end populateCommand

//-------------------------------------------------------------------------------------------------
/** reset transport data */
//-------------------------------------------------------------------------------------------------
void ControlBar::resetContainData( void )
{
	Int i;

	for( i = 0; i < MAX_COMMANDS_PER_SET; i++ )
	{

		m_containData[ i ].control = NULL;
		m_containData[ i ].objectID = INVALID_ID;

	}  // end for i

}  // end resetTransportData

//-------------------------------------------------------------------------------------------------
/** reset the build queue data we use to die queue entires to control */
//-------------------------------------------------------------------------------------------------
void ControlBar::resetBuildQueueData( void )
{
	Int i;

	for( i = 0; i < MAX_BUILD_QUEUE_BUTTONS; i++ )
	{

		m_queueData[ i ].control = NULL;
		m_queueData[ i ].type = PRODUCTION_INVALID;
		m_queueData[ i ].productionID = PRODUCTIONID_INVALID;
		m_queueData[ i ].upgradeToResearch = NULL;

	}  // end for i

}  // end resetBuildQueue

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ControlBar::populateBuildQueue( Object *producer )
{
/// @todo srj -- remove hard-coding here, please
	static const CommandButton *cancelUnitCommand = findCommandButton( "Command_CancelUnitCreate" );
/// @todo srj -- remove hard-coding here, please
	static const CommandButton *cancelUpgradeCommand = findCommandButton( "Command_CancelUpgradeCreate" );
	static NameKeyType buildQueueIDs[ MAX_BUILD_QUEUE_BUTTONS ];
	static Bool idsInitialized = FALSE;
	Int i;

	// reset the build queue data
	resetBuildQueueData();

	// get name key ids for the build queue buttons
	if( idsInitialized == FALSE )
	{
		AsciiString buttonName;

		for( i = 0; i < MAX_BUILD_QUEUE_BUTTONS; i++ )
		{
			
			buttonName.format( "ControlBar.wnd:ButtonQueue%02d", i + 1 );
			buildQueueIDs[ i ] = TheNameKeyGenerator->nameToKey( buttonName );

		}  // end for i

		idsInitialized = TRUE;

	}  // end if

	// get window pointers to all the buttons for the build queue
	for( i = 0; i < MAX_BUILD_QUEUE_BUTTONS; i++ )
	{

		// get window commented out cause I believe we already set this.  We'll see in a few minutes
		m_queueData[ i ].control = TheWindowManager->winGetWindowFromId( m_contextParent[ CP_BUILD_QUEUE ],
																																		 buildQueueIDs[ i ] );

		// disable window by default
		m_queueData[ i ].control->winEnable( FALSE );

		//Clear the status because this button doesn't use it -- and if it's set, it'll
		//become invisible meaning the image that was there will be showed.
		m_queueData[ i ].control->winClearStatus( WIN_STATUS_USE_OVERLAY_STATES );

		// set the text of the window to nothing by default
		GadgetButtonSetText( m_queueData[ i ].control, UnicodeString( L"" ) );

		//Clear any potential veterancy rank, or else we'll see it when it's empty!
		GadgetButtonDrawOverlayImage( m_queueData[ i ].control, NULL );

	}  // end for i

	// step through each object being built and set the image data for the buttons
	ProductionUpdateInterface *pu = producer->getProductionUpdateInterface();
	if( pu == NULL )
		return;  // sanity
	const ProductionEntry *production;
	Int windowIndex = 0;
	const Image *image;
	for( production = pu->firstProduction();	
			 production;
			 production = pu->nextProduction( production ) )
	{

		// don't go above how many queue windows we have
		if( windowIndex >= MAX_BUILD_QUEUE_BUTTONS )
			break;  // exit for

		// set the command into the queue button
		if( production->getProductionType() == PRODUCTION_UNIT )
		{

			// set the control command
			setControlCommand( m_queueData[ windowIndex ].control, cancelUnitCommand );
			m_queueData[ windowIndex ].type = PRODUCTION_UNIT;
			m_queueData[ windowIndex ].productionID = production->getProductionID();

			// set the images
			m_queueData[ windowIndex ].control->winEnable( TRUE );
			m_queueData[ windowIndex ].control->winSetStatus( WIN_STATUS_USE_OVERLAY_STATES );
			image = production->getProductionObject()->getButtonImage();
			GadgetButtonSetEnabledImage( m_queueData[ windowIndex ].control, image );
			
			//No longer used.
			//image = TheMappedImageCollection->findImageByName( production->getProductionObject()->getInventoryImageName( INV_IMAGE_HILITE ) );
			//GadgetButtonSetHiliteSelectedImage( m_queueData[ windowIndex ].control, image );
			//image = TheMappedImageCollection->findImageByName( production->getProductionObject()->getInventoryImageName( INV_IMAGE_PUSHED ) );
			//GadgetButtonSetHiliteImage( m_queueData[ windowIndex ].control, image );

			//Show the veterancy rank of the object being constructed in the queue
			const Image *image = calculateVeterancyOverlayForThing( production->getProductionObject() );
			GadgetButtonDrawOverlayImage( m_queueData[ windowIndex ].control, image );
			//
			// note we're not setting a disabled image into the queue button ... when there is
			// nothing in the queue we set the button to disabled, we want to leave the disabled
			// queue button graphic we already have in place
			//
	//		image = TheMappedImageCollection->findImageByName( production->getProductionObject()->getInventoryImageName( INV_IMAGE_DISABLED ) );
	//		GadgetButtonSetDisabledImage( m_queueData[ windowIndex ].control, image );

		}  // end if
		else
		{
			const UpgradeTemplate *ut = production->getProductionUpgrade();

			// set the control command
			setControlCommand( m_queueData[ windowIndex ].control, cancelUpgradeCommand );
			m_queueData[ windowIndex ].type = PRODUCTION_UPGRADE;
			m_queueData[ windowIndex ].upgradeToResearch = production->getProductionUpgrade();

			// set the images
			m_queueData[ windowIndex ].control->winEnable( TRUE );
			m_queueData[ windowIndex ].control->winSetStatus( WIN_STATUS_USE_OVERLAY_STATES );
			image = ut->getButtonImage();
			GadgetButtonSetEnabledImage( m_queueData[ windowIndex ].control, image );
			
			//No longer used
			//image = TheMappedImageCollection->findImageByName( ut->getQueueImageName( UpgradeTemplate::UPGRADE_HILITE ) );
			//GadgetButtonSetHiliteSelectedImage( m_queueData[ windowIndex ].control, image );
			//image = TheMappedImageCollection->findImageByName( ut->getQueueImageName( UpgradeTemplate::UPGRADE_PUSHED ) );
			//GadgetButtonSetHiliteImage( m_queueData[ windowIndex ].control, image );
			//
			// note we're not setting a disabled image into the queue button ... when there is
			// nothing in the queue we set the button to disabled, we want to leave the disabled
			// queue button graphic we already have in place
			//
	//		image = TheMappedImageCollection->findImageByName( ut->getQueueImageName( UpgradeTemplate::UPGRADE_DISABLED ) );
	//		GadgetButtonSetDisabledImage( m_queueData[ windowIndex ].control, image );

		}  // end else

		// we have filled up this window now
		windowIndex++;

	}  // end for

	//
	// save the count of things being produced in the build queue, when it changes we will
	// repopulate the queue to visually show the change
	//
	m_displayedQueueCount = pu->getProductionCount();

}  // end populateBuildQueue

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ControlBar::updateContextCommand( void )
{
 	Object *obj = NULL;
	Int i;

	// get object
	if( m_currentSelectedDrawable )
		obj = m_currentSelectedDrawable->getObject();

	//
	// the contents of objects are ususally showed on the UI, when those contents change
	// we always to update the UI
	//
	ContainModuleInterface *contain = obj ? obj->getContain() : NULL;
	if( contain && contain->getContainMax() > 0 && 
			m_lastRecordedInventoryCount != contain->getContainCount() )
	{

		// record this ast the last known number
		m_lastRecordedInventoryCount = contain->getContainCount();

		// re-evaluate the UI because something has changed
		evaluateContextUI();
					
	}  // end if, transport

	// get production update for those objects that have one
	ProductionUpdateInterface *pu = obj ? obj->getProductionUpdateInterface() : NULL;

	//
	// when we have a production update, we show the build queue when there is actually
	// something in the queue, otherwise we show the selection portrait for the object ... so if
	// the queue is visible we need to check to see if we should hide it and show the portrait,
	// and if the queue is hidden, we need to check and see if it should become shown
	//
	if( m_contextParent[ CP_BUILD_QUEUE ]->winIsHidden() == TRUE )
	{

		if( pu && pu->firstProduction() != NULL )
		{

			// don't show the portrait image
			setPortraitByObject( NULL );

			// show the build queue
			m_contextParent[ CP_BUILD_QUEUE ]->winHide( FALSE );
			populateBuildQueue( obj );

		}  // end if

	}  // end if
	else
	{

		if( pu && pu->firstProduction() == NULL )
		{

			// hide the build queue
			m_contextParent[ CP_BUILD_QUEUE ]->winHide( TRUE );

			// show the portrait image
			setPortraitByObject( obj );

		}  // end if

	}  // end else

	// update a visible production queue
	if( m_contextParent[ CP_BUILD_QUEUE ]->winIsHidden() == FALSE )
	{

		// when the build queue is enabled, the selected portrait cannot be shown
		setPortraitByObject( NULL );

		//
		// when showing a production queue, when the production count changes of the producer
		// object (the thing we have selected for the control bar) we will repopulate the
		// windows to visually show the new production linup
		//
		if( pu )
		{
		
			// update the whole queue as necessary
			if( pu->getProductionCount() != m_displayedQueueCount )
				populateBuildQueue( obj );

			//
			// update the build percentage on the first thing (the thing that's being built)
			// in the queue
			//
			const ProductionEntry *produce = pu->firstProduction();
			if( produce )
			{
				static NameKeyType winID = TheNameKeyGenerator->nameToKey( "ControlBar.wnd:ButtonQueue01" );
				GameWindow *win = TheWindowManager->winGetWindowFromId( m_contextParent[ CP_BUILD_QUEUE ], winID );
				
				DEBUG_ASSERTCRASH( win, ("updateContextCommand: Unable to find first build queue button\n") );
				//				UnicodeString text;
				//
				//				text.format( L"%.0f%%", produce->getPercentComplete() );
				//				GadgetButtonSetText( win, text );
				
				GadgetButtonDrawInverseClock(win,produce->getPercentComplete(), m_buildUpClockColor);

			}  // end if

		}  // end if

	}  // end if

	// evaluate each command on whether or not it should be enabled
	for( i = 0; i < MAX_COMMANDS_PER_SET; i++ )
	{
		GameWindow *win;
		const CommandButton *command;

		// get the window
		win = m_commandWindows[ i ];

		// only consider commands for windows that are actually shown
		//`tbd: fix the bug here, that is that if we don't change the unit, we won't attempt to show 
		// these.
		if( win->winIsHidden() == TRUE )
			continue;

		// get the command from the control
		command = (const CommandButton *)GadgetButtonGetData(win);
		//command = (const CommandButton *)win->winGetUserData();
		if( command == NULL )
			continue;

		// ignore transport/structure inventory commands, they are handled elsewhere
		if( command->getCommandType() == GUI_COMMAND_EXIT_CONTAINER )
		{
			win->winSetStatus( WIN_STATUS_ALWAYS_COLOR ); //Don't let these buttons render in grayscale ever!
			continue;
		}
		else
		{
			win->winClearStatus( WIN_STATUS_NOT_READY );
			win->winClearStatus( WIN_STATUS_ALWAYS_COLOR );
		}

		// is the command available
		CommandAvailability availability = getCommandAvailability( command, obj, win );

		// enable/disable the window control
		switch( availability )
		{
			case COMMAND_HIDDEN:
				win->winHide( TRUE );
				break;
			case COMMAND_RESTRICTED:
				win->winEnable( FALSE );
				break;
			case COMMAND_NOT_READY:
				win->winEnable( FALSE );
				win->winSetStatus( WIN_STATUS_NOT_READY );
				break;
			case COMMAND_CANT_AFFORD:
				win->winEnable( FALSE );
				win->winSetStatus( WIN_STATUS_ALWAYS_COLOR );
				break;
			default:
				win->winEnable( TRUE );
				break;
		}
  
		//Determine by the production type of this button, whether or not the created object
		//will have a veterancy rank
		const Image *image = calculateVeterancyOverlayForThing( command->getThingTemplate() );
		GadgetButtonDrawOverlayImage( win, image );
    
		//
		// for check-like commands we will keep the push button "pushed" or "unpushed" depending
		// on the current running status of the command
		//
		if( BitIsSet( command->getOptions(), CHECK_LIKE ))
		{

			// sanity, check like commands should have windows that are check like as well
			DEBUG_ASSERTCRASH( BitIsSet( win->winGetStatus(), WIN_STATUS_CHECK_LIKE ),	
												 ("updateContextCommand: Error, gadget window for command '%s' is not check-like!\n",
												 command->getName().str()) );

			if( availability == COMMAND_ACTIVE )
				GadgetCheckLikeButtonSetVisualCheck( win, TRUE );
			else
				GadgetCheckLikeButtonSetVisualCheck( win, FALSE );

		}  // end if

	}  // end for i
	
	// After Every change to the m_commandWIndows, we need to show fill in the missing blanks with the images
	// removed from multiplayer branch
	//showCommandMarkers();

//	// if we have a build tooltip layout, update it with the new data.
//	repopulateBuildTooltipLayout(); 

}  // end updatecontextCommand

//-------------------------------------------------------------------------------------------------
const Image* ControlBar::calculateVeterancyOverlayForThing( const ThingTemplate *thingTemplate )
{
	VeterancyLevel level = LEVEL_REGULAR;

	if( !thingTemplate )
	{
		return NULL;
	}

	Player *player = ThePlayerList->getLocalPlayer();
	if( !player )
	{
		return NULL;
	}

	//See if the thingTemplate has a VeterancyGainCreate
	//This is HORROR CODE and needs to be optimized!
	const VeterancyGainCreateModuleData *data = NULL;
	AsciiString modName;
	const ModuleInfo& mi = thingTemplate->getBehaviorModuleInfo();
	for( Int modIdx = 0; modIdx < mi.getCount(); ++modIdx )
	{
		modName = mi.getNthName(modIdx);
		if( !modName.compare( "VeterancyGainCreate" ) )
		{
			data = (const VeterancyGainCreateModuleData*)mi.getNthData( modIdx );
			break;
		}
	}

	//It does, so see if the player has that upgrade
	if( data && player->hasScience( data->m_scienceRequired ) )
	{
		//We do! So now check to see what the veterancy level would be.
		level = data->m_startingLevel;
	}

	//Return the appropriate image (including NULL if no veterancy levels)
	switch( level )
	{
		case LEVEL_VETERAN:
			return m_rankVeteranIcon;
		case LEVEL_ELITE:
			return m_rankEliteIcon;
		case LEVEL_HEROIC:
			return m_rankHeroicIcon;
	}
	return NULL;;
}

//-------------------------------------------------------------------------------------------------
const Image* ControlBar::calculateVeterancyOverlayForObject( const Object *obj )
{
	if( !obj )
	{
		return NULL;
	}
	VeterancyLevel level = obj->getVeterancyLevel();

	//Return the appropriate image (including NULL if no veterancy levels)
	switch( level )
	{
		case LEVEL_VETERAN:
			return m_rankVeteranIcon;
		case LEVEL_ELITE:
			return m_rankEliteIcon;
		case LEVEL_HEROIC:
			return m_rankHeroicIcon;
	}
	return NULL;;
}

//-------------------------------------------------------------------------------------------------
static Int getRappellerCount(Object* obj)
{
	Int num = 0;
	const ContainedItemsList* items = obj->getContain() ? obj->getContain()->getContainedItemsList() : NULL;
	if (items)
	{
		for (ContainedItemsList::const_iterator it = items->begin(); it != items->end(); ++it )
		{
			if ((*it)->isKindOf(KINDOF_CAN_RAPPEL))
			{
				++num;
			}
		}
	}
	return num;
}

//-------------------------------------------------------------------------------------------------
/** What's the status between 'obj' and the 'command' at present.  Can we do it?  Are
	* we already doing it?  Can ya dig it? */
//-------------------------------------------------------------------------------------------------
CommandAvailability ControlBar::getCommandAvailability( const CommandButton *command, 
																												Object *obj, 
																												GameWindow *win,
																												Bool forceDisabledEvaluation ) const
{
	if (command->getCommandType() == GUI_COMMAND_SPECIAL_POWER_FROM_COMMAND_CENTER)
	{
		if (ThePlayerList && ThePlayerList->getLocalPlayer())
			obj = ThePlayerList->getLocalPlayer()->findNaturalCommandCenter();
		else
			obj = NULL;
	}

	if (obj == NULL)
		return COMMAND_HIDDEN;	// probably better than crashing....

	Player *player = obj->getControllingPlayer();

	if (obj->testScriptStatusBit(OBJECT_STATUS_SCRIPT_DISABLED) || obj->testScriptStatusBit(OBJECT_STATUS_SCRIPT_UNPOWERED))
	{
		// if the object status is disabled or unpowered, you cannot do anything to it.
		return COMMAND_HIDDEN;
	}
	
	//Unmanned vehicles don't have any commands available -- in fact they are hidden!
 	if( obj->isDisabledByType( DISABLED_UNMANNED ) )
 	{
 		return COMMAND_HIDDEN;
 	}

	//It's possible for command buttons to be a single use only type of a button -- like detonating a nuke from a convoy truck.
	if( obj->hasSingleUseCommandBeenUsed() )
	{
		return COMMAND_RESTRICTED;
	}
 
	//Other disabled objects are unable to use buttons -- so gray them out.
	Bool disabled = obj->isDisabled();
	
	// if we are only disabled by being underpowered, and this button doesn't care, well, fix it
	if (disabled
			&& BitIsSet(command->getOptions(), IGNORES_UNDERPOWERED) 
			&& obj->getDisabledFlags().test(DISABLED_UNDERPOWERED)
			&& obj->getDisabledFlags().count() == 1)
	{
		disabled = false;
	}

 	if (disabled && !forceDisabledEvaluation)
 	{

		GUICommandType commandType = command->getCommandType();
		if( commandType != GUI_COMMAND_SELL && 
				commandType != GUI_COMMAND_EVACUATE &&
				commandType != GUI_COMMAND_EXIT_CONTAINER && 
				commandType != GUI_COMMAND_BEACON_DELETE && 
				commandType != GUI_COMMAND_SET_RALLY_POINT && 
				commandType != GUI_COMMAND_SWITCH_WEAPON )
		{
			if( getCommandAvailability( command, obj, win, TRUE ) == COMMAND_HIDDEN )
			{
				return COMMAND_HIDDEN;
			}
 			return COMMAND_RESTRICTED;
		}
 	}

	// if the command requires an upgrade and we don't have it we can't do it
	if( BitIsSet( command->getOptions(), NEED_UPGRADE ) )
	{
		const UpgradeTemplate *upgradeT = command->getUpgradeTemplate();
		if (upgradeT)
		{
			// upgrades come in the form of player upgrades and object upgrades
			if( upgradeT->getUpgradeType() == UPGRADE_TYPE_PLAYER )
			{
				if( player->hasUpgradeComplete( upgradeT ) == FALSE )
					return COMMAND_RESTRICTED;
			}
			else if( upgradeT->getUpgradeType() == UPGRADE_TYPE_OBJECT && 
							 obj->hasUpgrade( upgradeT ) == FALSE )
			{
				return COMMAND_RESTRICTED;
			}
		}
	}

	ProductionUpdateInterface *pu = obj->getProductionUpdateInterface();
	if( pu && pu->firstProduction() && BitIsSet( command->getOptions(), NOT_QUEUEABLE ) )
	{
		//This button is designated so that it is incapable of building this upgrade/object
		//when anything is in the production queue.
		return COMMAND_RESTRICTED;
	}

	Bool queueMaxed = pu ? ( pu->getProductionCount() == MAX_BUILD_QUEUE_BUTTONS ) : FALSE;

	switch( command->getCommandType() )
	{
		case GUI_COMMAND_DOZER_CONSTRUCT:
		{
			// if the command is a dozer construct task and the object dozer is building anything
			// this command is not available
			if(command->getThingTemplate())
			{
				BuildableStatus bStatus = command->getThingTemplate()->getBuildable();
				if (bStatus == BSTATUS_NO || (bStatus == BSTATUS_ONLY_BY_AI && obj->getControllingPlayer()->getPlayerType() != PLAYER_COMPUTER))
					return COMMAND_HIDDEN;
			}

			// sanity, non dozer object
			if( obj->isKindOf( KINDOF_DOZER ) == FALSE )
				return COMMAND_RESTRICTED;

			// get the dozer ai update interface
			DozerAIInterface* dozerAI = NULL;
			if( obj->getAIUpdateInterface() == NULL )
				return COMMAND_RESTRICTED;

			dozerAI = obj->getAIUpdateInterface()->getDozerAIInterface();

			DEBUG_ASSERTCRASH( dozerAI != NULL, ("Something KINDOF_DOZER must have a Dozer-like AIUpdate") );
			if( dozerAI == NULL )
				return COMMAND_RESTRICTED;

			// if building anything at all right now we can't build another
			if( dozerAI->isTaskPending( DOZER_TASK_BUILD ) == TRUE )
				return COMMAND_RESTRICTED;
			
			// return whether or not the player can build this thing
			if( player->canBuild( command->getThingTemplate() ) == FALSE )
				return COMMAND_RESTRICTED;

			if( !player->canAffordBuild( command->getThingTemplate() ) )
			{
				return COMMAND_RESTRICTED;//COMMAND_CANT_AFFORD;
			}

			break;
		}  

		case GUI_COMMAND_SELL:
		{
			// if this is a sell command, is the object marked as "This cannot be sold?"
			// if so, remove the button, otherwise, its available
			if (obj->testScriptStatusBit(OBJECT_STATUS_SCRIPT_UNSELLABLE))
				return COMMAND_HIDDEN;
			break;
		}

		case GUI_COMMAND_UNIT_BUILD:
		{
			// command is a unit build
			if(command->getThingTemplate())
			{
				BuildableStatus bStatus = command->getThingTemplate()->getBuildable();
				if (bStatus == BSTATUS_NO || (bStatus == BSTATUS_ONLY_BY_AI && obj->getControllingPlayer()->getPlayerType() != PLAYER_COMPUTER))
					return COMMAND_HIDDEN;
			}

			if( queueMaxed )
			{
				return COMMAND_RESTRICTED;
			}

			// return whether or not the player can build this thing
			//NOTE: Player::canBuild() only checks prerequisites!
			if( player->canBuild( command->getThingTemplate() ) == FALSE )
				return COMMAND_RESTRICTED;

			CanMakeType makeType = TheBuildAssistant->canMakeUnit( obj, command->getThingTemplate() );
			if( makeType == CANMAKE_MAXED_OUT_FOR_PLAYER || makeType == CANMAKE_PARKING_PLACES_FULL )
			{
				//Disable the button if the player has a max amount of these units in build queue or existance.
				return COMMAND_RESTRICTED;
			}
			if( makeType == CANMAKE_NO_MONEY )
			{
				return COMMAND_RESTRICTED; //COMMAND_CANT_AFFORD;
			}

			break;
		}  

		case GUI_COMMAND_PLAYER_UPGRADE:
		{
			if( queueMaxed )
			{
				return COMMAND_RESTRICTED;
			}
			// if we can build it, we must also NOT already have it or be building it
			if( player->hasUpgradeComplete( command->getUpgradeTemplate() ) == TRUE ||
					player->hasUpgradeInProduction( command->getUpgradeTemplate() ) == TRUE )
				return COMMAND_CANT_AFFORD;//COMMAND_RESTRICTED;

			// if this is an upgrade create we must be able to build it.
			if( TheUpgradeCenter->canAffordUpgrade( player, command->getUpgradeTemplate() ) == FALSE )
				return COMMAND_RESTRICTED;//COMMAND_CANT_AFFORD;

			break;
		} 

		case GUI_COMMAND_OBJECT_UPGRADE:
		{
			if( queueMaxed )
			{
				return COMMAND_RESTRICTED;
			}
			// no production update, can't possibly do this command
			if( pu == NULL )
			{
				DEBUG_CRASH(("Objects that have Object-Level Upgrades must also have ProductionUpdate. Just cuz."));
				return COMMAND_RESTRICTED;
			}

			//
			// if this object already has this upgrade, or is researching it already in the queue
			// we will disable the button so you can't build another one
			//
			if( obj->hasUpgrade( command->getUpgradeTemplate() ) == TRUE ||
					pu->isUpgradeInQueue( command->getUpgradeTemplate() ) == TRUE ||
					obj->affectedByUpgrade( command->getUpgradeTemplate() ) == FALSE )
				return COMMAND_CANT_AFFORD;//COMMAND_RESTRICTED;

			if( TheUpgradeCenter->canAffordUpgrade( player, command->getUpgradeTemplate() ) == FALSE )
				return COMMAND_RESTRICTED;//COMMAND_CANT_AFFORD;
			break;
		} 

		case GUI_COMMAND_FIRE_WEAPON:
		{
			AIUpdateInterface *ai = obj->getAIUpdateInterface();

			// no ai, can't possibly fire weapon
			if( ai == NULL )
				return COMMAND_RESTRICTED;

			// ask the ai if the weapon is ready to fire
			const Weapon* w = obj->getWeaponInWeaponSlot( command->getWeaponSlot() );

			// changed this to Log rather than Crash, because this can legitimately happen now for
			// dozers and workers with mine-clearing stuff... (srj)
			//DEBUG_ASSERTLOG( w, ("Unit %s's CommandButton %s is trying to access weaponslot %d, but doesn't have a weapon there in its FactionUnit ini entry.\n", 
			//	obj->getTemplate()->getName().str(), command->getName().str(), (Int)command->getWeaponSlot() ) );
			
			UnsignedInt now = TheGameLogic->getFrame();

			/// @Kris -- We need to show the button as always available for anything with a 0 clip reload time.
			if( w && w->getClipReloadTime( obj ) == 0 )
			{
				return COMMAND_AVAILABLE;
			}

			if( w == NULL																	// No weapon
				|| w->getStatus() != READY_TO_FIRE					// Weapon not ready
				|| w->getPossibleNextShotFrame() == now			// Weapon ready, but could fire this exact frame (handle button flicker since it may be going to fire anyway)
/// @todo srj -- not sure why this next check is necessary, but the Comanche missile buttons will flicker without it. figure out someday.
/// @todo ml  -- and note: that the "now-1" below causes zero-clip-reload weapons to never be ready, so I added this
/// If you make changes to this code, make sure that the DragonTank's firewall weapon can be retargeted while active,
/// that is, while the tank is squirting out flames all over the floor, you can click the firewall button (or "F"), 
/// and re-target the firewall without having to stop or move in-betwen.. Thanks for reading 
				|| (w->getPossibleNextShotFrame()==now-1) 	
				)
			{
				if ( w != NULL )
				{
					// only draw the clock when reloading a clip, not when merely between shots, since that's usually a tiny amount of time
					if ( w->getStatus() == RELOADING_CLIP)
					{
						Int percent = w->getPercentReadyToFire() * 100;
						GadgetButtonDrawInverseClock(win, percent, m_buildUpClockColor);
					}
					return COMMAND_NOT_READY;
				}
				else
				{
					// if this is a mine-clearing button but we don't have the right weaponset,
					// just declare it available... we'll switch weaponsets when the time comes
					if (
						(command->getOptions() & USES_MINE_CLEARING_WEAPONSET) != 0
						&& !obj->testWeaponSetFlag(WEAPONSET_MINE_CLEARING_DETAIL)
					)
					{
						return COMMAND_AVAILABLE;
					}

					// no weapon in the slot means "gray me out"
					return COMMAND_RESTRICTED;
				}
			} 

			break;
		}

		case GUI_COMMAND_GUARD:
		case GUI_COMMAND_GUARD_WITHOUT_PURSUIT:
		case GUI_COMMAND_GUARD_FLYING_UNITS_ONLY:
			// always available
			break;	

		case GUI_COMMAND_COMBATDROP:
		{
			if( getRappellerCount(obj) <= 0 )
				return COMMAND_RESTRICTED;
			break;
		}

		case GUI_COMMAND_EXIT_CONTAINER:
		{
			
			//
			// this method is really used as a per frame update to see if we should enable
			// disable a control ... inventory of objects shows as buttons have that enable
			// disable logic handled elsewhere, where if the contained count of the entire
			// container changes the UI is completely repopulated
			//
			break;
		} 

		case GUI_COMMAND_EVACUATE:
		{

			// if we have no contained objects we can't evacuate anything
			if( !obj->getContain() || obj->getContain()->getContainCount() <= 0 )
				return COMMAND_RESTRICTED;
			break;
		}  

		case GUI_COMMAND_EXECUTE_RAILED_TRANSPORT:
		{
			DockUpdateInterface *dui = obj->getDockUpdateInterface();

			// if the dock is closed or not present this command is invalid
			if( dui == NULL || dui->isDockOpen() == FALSE )
				return COMMAND_RESTRICTED;
			break;
		}  

		case GUI_COMMAND_SPECIAL_POWER_FROM_COMMAND_CENTER:
		case GUI_COMMAND_SPECIAL_POWER:
		{
			// sanity
			DEBUG_ASSERTCRASH( command->getSpecialPowerTemplate() != NULL,
												 ("The special power in the command '%s' is NULL\n", command->getName().str()) );
			// get special power module from the object to execute it
			SpecialPowerModuleInterface *mod = obj->getSpecialPowerModule( command->getSpecialPowerTemplate() );

			if( mod == NULL )
			{
				// sanity ... we must have a module for the special power, if we don't somebody probably
				// forgot to put it in the object
				DEBUG_CRASH(( "Object does not contain special power module (%s) to execute.  Did you forget to add it to the object INI?\n",
											command->getSpecialPowerTemplate()->getName().str() ));
			} 
			else if( mod->isReady() == FALSE )
			{
				Int percent =  mod->getPercentReady() * 100;

				GadgetButtonDrawInverseClock(win, percent, m_buildUpClockColor);
				return COMMAND_NOT_READY;
			}
			else if( SpecialAbilityUpdate *spUpdate = obj->findSpecialAbilityUpdate( command->getSpecialPowerTemplate()->getSpecialPowerType() ) )
			{
				if( spUpdate->isPowerCurrentlyInUse( command ) )
				{
					return COMMAND_RESTRICTED;
				}
			}
			else if( mod->getSpecialPowerTemplate()->getSpecialPowerType() == SPECIAL_CHANGE_BATTLE_PLANS )
			{
				static NameKeyType key_BattlePlanUpdate = NAMEKEY( "BattlePlanUpdate" );
				BattlePlanUpdate *update = (BattlePlanUpdate*)obj->findUpdateModule( key_BattlePlanUpdate );
				if( update && update->getCommandOption() & command->getOptions() )
				{
					return COMMAND_ACTIVE;
				}
			}

			break;
		}  

		case GUI_COMMAND_TOGGLE_OVERCHARGE:
		{
			OverchargeBehaviorInterface *obi;
			// search object behavior mdoules
			for( BehaviorModule **bmi = obj->getBehaviorModules(); *bmi; ++bmi )
			{
				// we're looking for the overcharge interface
				obi = (*bmi)->getOverchargeBehaviorInterface();
				if( obi )
				{
					if( obi->isOverchargeActive() )
						return COMMAND_ACTIVE;
				} 
			}  
			break;
		} 

		// switch weapon command
		case GUI_COMMAND_SWITCH_WEAPON:
		{
			// ask the ai which weapon is in the current slot
			const Weapon* w = obj->getWeaponInWeaponSlot( command->getWeaponSlot() );

			DEBUG_ASSERTCRASH( w, ("Unit %s's CommandButton %s is trying to access weaponslot %d, but doesn't have a weapon there in its FactionUnit ini entry.", 
				obj->getTemplate()->getName().str(), command->getName().str(), (Int)command->getWeaponSlot() ) );
			
			if( w == NULL)
				return COMMAND_RESTRICTED;

			const DrawableList *selected = TheInGameUI->getAllSelectedDrawables();
			for( DrawableListCIt it = selected->begin(); it != selected->end(); ++it )
			{
				Drawable *draw = *it;
				if( draw && draw->getObject() && draw->getObject()->isLocallyControlled() && draw->getObject()->getCurrentWeapon())
				{
					WeaponSlotType wslot = draw->getObject()->getCurrentWeapon()->getWeaponSlot();
					if (wslot != command->getWeaponSlot())
						return COMMAND_AVAILABLE;
				}
			}
			
			return COMMAND_ACTIVE;
		}
		
		case GUI_COMMAND_HACK_INTERNET:
		{
			AIUpdateInterface *ai = obj->getAI();
			if( ai )
			{
				HackInternetAIInterface *hackAI = ai->getHackInternetAIInterface();
				if( hackAI && hackAI->isHackingPackingOrUnpacking() )
				{
					return COMMAND_RESTRICTED;
				}
			}
			return COMMAND_AVAILABLE;
		}
		
		case GUI_COMMAND_STOP:
		{
			if( !BitIsSet( command->getOptions(), OPTION_ONE ) )
			{
				return COMMAND_AVAILABLE;
			}

			//We're dealing with a strategy center stop button. Only show the button
			//if we're in bombardment mode (to stop the artillery cannon).
			static NameKeyType key_BattlePlanUpdate = NAMEKEY( "BattlePlanUpdate" );
			BattlePlanUpdate *bpUpdate = (BattlePlanUpdate*)obj->findUpdateModule( key_BattlePlanUpdate );
			if( bpUpdate && bpUpdate->getActiveBattlePlan() != PLANSTATUS_BOMBARDMENT )
			{
				return COMMAND_RESTRICTED;
			}
			return COMMAND_AVAILABLE;
		}
	}
	
	// all is well with the command
	return COMMAND_AVAILABLE;

}  // end getCommandAvailability

