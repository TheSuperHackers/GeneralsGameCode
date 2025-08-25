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

// FILE: GameClient.cpp ////////////////////////////////////////////////////
// Implementation of GameClient singleton
// Author: Michael S. Booth, March 2001
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "GameClient/GameClient.h"

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/ActionManager.h"
#include "Common/GameEngine.h"
#include "Common/GameState.h"
#include "Common/GlobalData.h"
#include "Common/PerfTimer.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "Common/GameLOD.h"
#include "GameClient/Anim2D.h"
#include "GameClient/CampaignManager.h"
#include "GameClient/ChallengeGenerals.h"
#include "GameClient/CommandXlat.h"
#include "GameClient/ControlBar.h"
#include "GameClient/Diplomacy.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/Drawable.h"
#include "GameClient/DrawGroupInfo.h"
#include "GameClient/Eva.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GlobalLanguage.h"
#include "GameClient/GraphDraw.h"
#include "GameClient/GUICommandTranslator.h"
#include "GameClient/HeaderTemplate.h"
#include "GameClient/HintSpy.h"
#include "GameClient/HotKey.h"
#include "GameClient/IMEManager.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Keyboard.h"
#include "GameClient/LanguageFilter.h"
#include "GameClient/LookAtXlat.h"
#include "GameClient/MetaEvent.h"
#include "GameClient/Mouse.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/PlaceEventTranslator.h"
#include "GameClient/RayEffect.h"
#include "GameClient/SelectionXlat.h"
#include "GameClient/Shell.h"
#include "GameClient/Snow.h"
#include "GameClient/TerrainVisual.h"
#include "GameClient/View.h"
#include "GameClient/VideoPlayer.h"
#include "GameClient/WindowXlat.h"
#include "GameLogic/FPUControl.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/GhostObject.h"
#include "GameLogic/Object.h"
#include "GameLogic/ScriptEngine.h"		// For TheScriptEngine - jkmcd

#define DRAWABLE_HASH_SIZE	8192

/// The GameClient singleton instance
GameClient *TheGameClient = NULL;

//-------------------------------------------------------------------------------------------------
GameClient::GameClient()
{

	// zero our translator list
	for( Int i = 0; i < MAX_CLIENT_TRANSLATORS; i++ )
		m_translators[ i ] = TRANSLATOR_ID_INVALID;
	m_numTranslators = 0;
	m_commandTranslator = NULL;

	// Added By Sadullah Nader
	// Initializations missing and needed
	m_drawableTOC.clear();
	//
	m_textBearingDrawableList.clear();

	m_frame = 0;

	m_drawableList = NULL;

	m_nextDrawableID = (DrawableID)1;
	TheDrawGroupInfo = new DrawGroupInfo;
}

//std::vector<std::string>	preloadTextureNamesGlobalHack;
//std::vector<std::string>	preloadTextureNamesGlobalHack2;

//-------------------------------------------------------------------------------------------------
GameClient::~GameClient()
{
#ifdef PERF_TIMERS
	delete TheGraphDraw;
	TheGraphDraw = NULL;
#endif

	if (TheDrawGroupInfo)
	{
		delete TheDrawGroupInfo;
		TheDrawGroupInfo = NULL;
	}

	// clear any drawable TOC we might have
	m_drawableTOC.clear();

	//DEBUG_LOG(("Preloaded texture files ------------------------------------------"));
	//for (Int oog=0; oog<preloadTextureNamesGlobalHack2.size(); ++oog)
	//{
	//	DEBUG_LOG(("%s", preloadTextureNamesGlobalHack2[oog]));
	//}
	//DEBUG_LOG(("------------------------------------------------------------------"));
	//for (oog=0; oog<preloadTextureNamesGlobalHack.size(); ++oog)
	//{
	//	DEBUG_LOG(("%s", preloadTextureNamesGlobalHack[oog]));
	//}
	//DEBUG_LOG(("End Texture files ------------------------------------------------"));
	if(TheCampaignManager)
		delete TheCampaignManager;
	TheCampaignManager = NULL;

	// destroy all Drawables
	Drawable *draw, *nextDraw;
	for( draw = m_drawableList; draw; draw = nextDraw )
	{
		nextDraw = draw->getNextDrawable();
		destroyDrawable( draw );
	}
	m_drawableList = NULL;

	// delete the ray effects
	delete TheRayEffects;
	TheRayEffects = NULL;

	// delete the hot key manager
	delete TheHotKeyManager;
	TheHotKeyManager = NULL;

	// destroy the in-game user interface
	delete TheInGameUI;
	TheInGameUI = NULL;

	delete TheChallengeGenerals;
	TheChallengeGenerals = NULL;

	// delete the shell
	delete TheShell;
	TheShell = NULL;

	delete TheIMEManager;
	TheIMEManager = NULL;

	// delete window manager
	delete TheWindowManager;
	TheWindowManager = NULL;

	// delete the font library
	TheFontLibrary->reset();
	delete TheFontLibrary;
	TheFontLibrary = NULL;

	TheMouse->reset();
	delete TheMouse;
	TheMouse = NULL;

	///@todo :  TheTerrainVisual used to be the first thing destroyed.
	//I had to put in here so that drawables free their track marks before
	//the terrain visual deletes the track laying system. MW

	// destroy the terrain visual representation
	delete TheTerrainVisual;
	TheTerrainVisual = NULL;

	// destroy the display
	delete TheDisplay;
	TheDisplay = NULL;

	delete TheHeaderTemplateManager;
	TheHeaderTemplateManager = NULL;

	delete TheLanguageFilter;
	TheLanguageFilter = NULL;

	delete TheVideoPlayer;
	TheVideoPlayer = NULL;

	// destroy all translators
	for( UnsignedInt i = 0; i < m_numTranslators; i++ )
		TheMessageStream->removeTranslator( m_translators[ i ] );
	m_numTranslators = 0;
	m_commandTranslator = NULL;

	delete TheAnim2DCollection;
	TheAnim2DCollection = NULL;

	delete TheMappedImageCollection;
	TheMappedImageCollection = NULL;

	delete TheKeyboard;
	TheKeyboard = NULL;

	delete TheDisplayStringManager;
	TheDisplayStringManager = NULL;

	delete TheEva;
	TheEva = NULL;

	delete TheSnowManager;
	TheSnowManager = NULL;

}  // end ~GameClient

//-------------------------------------------------------------------------------------------------
/** Initialize resources for the game client */
//-------------------------------------------------------------------------------------------------
void GameClient::init( void )
{

	setFrameRate(MSEC_PER_LOGICFRAME_REAL);		// from GameCommon.h... tell W3D what our expected framerate is

	INI ini;
	// Load the DrawGroupInfo here, before the Display Manager is loaded.
	ini.loadFileDirectory("Data\\INI\\DrawGroupInfo", INI_LOAD_OVERWRITE, NULL);

	// Override the ini values with localized versions:
	if (TheGlobalLanguageData && TheGlobalLanguageData->m_drawGroupInfoFont.name.isNotEmpty())
	{
		TheDrawGroupInfo->m_fontName = TheGlobalLanguageData->m_drawGroupInfoFont.name;
		TheDrawGroupInfo->m_fontSize = TheGlobalLanguageData->m_drawGroupInfoFont.size;
		TheDrawGroupInfo->m_fontIsBold = TheGlobalLanguageData->m_drawGroupInfoFont.bold;
	}

	// create the display string factory
	TheDisplayStringManager = createDisplayStringManager();
	if( TheDisplayStringManager )	{
		TheDisplayStringManager->init();
		TheDisplayStringManager->setName("TheDisplayStringManager");
	}

	if (!TheGlobalData->m_headless)
	{
		// create the keyboard
		TheKeyboard = createKeyboard();
		TheKeyboard->init();
		TheKeyboard->setName("TheKeyboard");
	}

	// allocate and load image collection for the GUI and just load the 256x256 ones for now
	TheMappedImageCollection = MSGNEW("GameClientSubsystem") ImageCollection;
	TheMappedImageCollection->load( 512 );

	// now that we have all the images loaded ... load any animation definitions from those images
	TheAnim2DCollection = MSGNEW("GameClientSubsystem") Anim2DCollection;
	TheAnim2DCollection->init();
 	TheAnim2DCollection->setName("TheAnim2DCollection");

	// register message translators
	if( TheMessageStream )
	{

		//
		// NOTE: Make sure m_translators[] is large enough to accomodate all the translators you
		// are loading here.  See MAX_CLIENT_TRANSLATORS
		//

		// since we only allocate one of each, don't bother pooling 'em
		m_translators[ m_numTranslators++ ] =	TheMessageStream->attachTranslator( MSGNEW("GameClientSubsystem") WindowTranslator,     10 );
		m_translators[ m_numTranslators++ ] =	TheMessageStream->attachTranslator( MSGNEW("GameClientSubsystem") MetaEventTranslator,	20 );
		m_translators[ m_numTranslators++ ] =	TheMessageStream->attachTranslator( MSGNEW("GameClientSubsystem") HotKeyTranslator,	25 );
		m_translators[ m_numTranslators++ ] =	TheMessageStream->attachTranslator( MSGNEW("GameClientSubsystem") PlaceEventTranslator,	30 );
		m_translators[ m_numTranslators++ ] = TheMessageStream->attachTranslator( MSGNEW("GameClientSubsystem") GUICommandTranslator, 40 );
		m_translators[ m_numTranslators++ ] =	TheMessageStream->attachTranslator( MSGNEW("GameClientSubsystem") SelectionTranslator,	50 );
		m_translators[ m_numTranslators++ ] =	TheMessageStream->attachTranslator( MSGNEW("GameClientSubsystem") LookAtTranslator,			60 );
		m_translators[ m_numTranslators ] =	TheMessageStream->attachTranslator( MSGNEW("GameClientSubsystem") CommandTranslator,		70 );
		// we keep a pointer to the command translator because it's useful
		m_commandTranslator = (CommandTranslator *)TheMessageStream->findTranslator( m_translators[ m_numTranslators++ ] );
		m_translators[ m_numTranslators++ ] =	TheMessageStream->attachTranslator( MSGNEW("GameClientSubsystem") HintSpyTranslator,		100 );

		//
		// the client message translator should probably remain as the last reaction of the
		// client before the messages are given to the network for processing.  This
		// lets all systems in the client give events that can be processed by the
		// client message translator
		//
		m_translators[ m_numTranslators++ ] =	TheMessageStream->attachTranslator( MSGNEW("GameClientSubsystem") GameClientMessageDispatcher, 999999999 );

	}

	// create the font library
	TheFontLibrary = createFontLibrary();
	if( TheFontLibrary )
		TheFontLibrary->init();

	// create the mouse
	TheMouse = TheGlobalData->m_headless ? NEW MouseDummy : createMouse();
	TheMouse->parseIni();
	TheMouse->initCursorResources();
 	TheMouse->setName("TheMouse");

	// instantiate the display
	TheDisplay = createGameDisplay();
	if( TheDisplay ) {
		TheDisplay->init();
 		TheDisplay->setName("TheDisplay");
	}

	TheHeaderTemplateManager = MSGNEW("GameClientSubsystem") HeaderTemplateManager;
	if(TheHeaderTemplateManager){
		TheHeaderTemplateManager->init();
	}

	// create the window manager
	TheWindowManager = TheGlobalData->m_headless ? NEW GameWindowManagerDummy : createWindowManager();
	if( TheWindowManager )
	{

		TheWindowManager->init();
 		TheWindowManager->setName("TheWindowManager");
//		TheWindowManager->initTestGUI();

	}  // end if

	// create the IME manager
	TheIMEManager = CreateIMEManagerInterface();
	if ( TheIMEManager )
	{
		TheIMEManager->init();
 		TheIMEManager->setName("TheIMEManager");
	}

	// create the shell
	TheShell = MSGNEW("GameClientSubsystem") Shell;
	if( TheShell ) {
		TheShell->init();
 		TheShell->setName("TheShell");
	}

	// instantiate the in-game user interface
	TheInGameUI = createInGameUI();
	if( TheInGameUI ) {
		TheInGameUI->init();
 		TheInGameUI->setName("TheInGameUI");
	}

 	TheChallengeGenerals = createChallengeGenerals();
 	if( TheChallengeGenerals ) {
 		TheChallengeGenerals->init();
 	}

	TheHotKeyManager = MSGNEW("GameClientSubsystem") HotKeyManager;
	if( TheHotKeyManager ) {
		TheHotKeyManager->init();
 		TheHotKeyManager->setName("TheHotKeyManager");
	}

	// instantiate the terrain visual display
	TheTerrainVisual = createTerrainVisual();
	if( TheTerrainVisual ) {
		TheTerrainVisual->init();
 		TheTerrainVisual->setName("TheTerrainVisual");
	}

	// allocate the ray effects manager
	TheRayEffects = MSGNEW("GameClientSubsystem") RayEffectSystem;
	if( TheRayEffects )	{
		TheRayEffects->init();
 		TheRayEffects->setName("TheRayEffects");
	}

	TheMouse->init();	//finish initializing the mouse.

	// set the limits of the mouse now that we've created the display and such
	if( TheMouse )
	{
		TheMouse->setPosition( 0, 0 );
		TheMouse->setMouseLimits();
 		TheMouse->setName("TheMouse");
	}  // end if

	// create the video player
	TheVideoPlayer = createVideoPlayer();
	if ( TheVideoPlayer )
	{
		TheVideoPlayer->init();
 		TheVideoPlayer->setName("TheVideoPlayer");
	}

	// create the language filter.
	TheLanguageFilter = createLanguageFilter();
	if (TheLanguageFilter)
	{
		TheLanguageFilter->init();
 		TheLanguageFilter->setName("TheLanguageFilter");
	}

	TheCampaignManager = MSGNEW("GameClientSubsystem") CampaignManager;
	TheCampaignManager->init();

	TheEva = MSGNEW("GameClientSubsystem") Eva;
	TheEva->init();
 	TheEva->setName("TheEva");

	TheDisplayStringManager->postProcessLoad();

	TheSnowManager = createSnowManager();
	if (TheSnowManager)
	{
		TheSnowManager->init();
		TheSnowManager->setName("TheSnowManager");
	}

#ifdef PERF_TIMERS
	TheGraphDraw = new GraphDraw;
#endif

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset the game client for a new game */
void GameClient::reset( void )
{
	Drawable *draw, *nextDraw;
//	m_drawableHash.clear();
//	m_drawableHash.resize(DRAWABLE_HASH_SIZE);

	m_drawableVector.clear();
	m_drawableVector.resize(DRAWABLE_HASH_SIZE, NULL);

	// need to reset the in game UI to clear drawables before they are destroyed
	TheInGameUI->reset();

	// destroy all Drawables
	for( draw = m_drawableList; draw; draw = nextDraw )
	{
		nextDraw = draw->getNextDrawable();
		destroyDrawable( draw );
	}
	m_drawableList = NULL;

	TheDisplay->reset();
	TheTerrainVisual->reset();
	TheRayEffects->reset();
	TheVideoPlayer->reset();
	TheEva->reset();
	if (TheSnowManager)
		TheSnowManager->reset();

	// clear any drawable TOC we might have
	m_drawableTOC.clear();

	// TheSuperHackers @fix Mauller 13/04/2025 Reset the drawable id so it does not keep growing over the lifetime of the game.
	m_nextDrawableID = (DrawableID)1;

}  // end reset

/** -----------------------------------------------------------------------------------------------
 * Return a new unique object id.
 */
DrawableID GameClient::allocDrawableID( void )
{
	/// @todo Find unused value in current set
	DrawableID ret = m_nextDrawableID;
	m_nextDrawableID = (DrawableID)((UnsignedInt)m_nextDrawableID + 1);
	return ret;
}

/** -----------------------------------------------------------------------------------------------
 * Given a drawable, register it with the GameClient and give it a unique ID.
 */
void GameClient::registerDrawable( Drawable *draw )
{

	// assign this drawable a unique ID, this will add it to the fast lookup table too
	draw->setID( allocDrawableID() );

	// add the drawable to the master list
	draw->prependToList( &m_drawableList );

}  // end registerDrawable

/** -----------------------------------------------------------------------------------------------
 * Redraw all views, update the GUI, play sound effects, etc.
 */
DECLARE_PERF_TIMER(GameClient_update)
DECLARE_PERF_TIMER(GameClient_draw)
void GameClient::update( void )
{
	USE_PERF_TIMER(GameClient_update)
	// create the FRAME_TICK message
	GameMessage *frameMsg = TheMessageStream->appendMessage( GameMessage::MSG_FRAME_TICK );
	frameMsg->appendTimestampArgument( getFrame() );
	static Bool playSizzle = FALSE;
	// We need to show the movie first.
	if(TheGlobalData->m_playIntro && !TheDisplay->isMoviePlaying())
	{
		if(TheGameLODManager && TheGameLODManager->didMemPass())
			TheDisplay->playLogoMovie("EALogoMovie", 5000, 3000);
		else
			TheDisplay->playLogoMovie("EALogoMovie640", 5000, 3000);
		TheWritableGlobalData->m_playIntro = FALSE;
		TheWritableGlobalData->m_afterIntro = TRUE;
		playSizzle = TRUE;
	}

	//Initial Game Codition.  We must show the movie first and then we can display the shell
	if(TheGlobalData->m_afterIntro && !TheDisplay->isMoviePlaying())
	{
		if( playSizzle && TheGlobalData->m_playSizzle )
		{
			TheWritableGlobalData->m_allowExitOutOfMovies = TRUE;
			if(TheGameLODManager && TheGameLODManager->didMemPass())
				TheDisplay->playMovie("Sizzle");
			else
				TheDisplay->playMovie("Sizzle640");
			playSizzle = FALSE;
		}
		else
		{
			TheWritableGlobalData->m_breakTheMovie = TRUE;
			TheWritableGlobalData->m_allowExitOutOfMovies = TRUE;

			if(TheGameLODManager && !TheGameLODManager->didMemPass())
			{
				TheWritableGlobalData->m_breakTheMovie = FALSE;

				WindowLayout *legal = TheWindowManager->winCreateLayout("Menus/LegalPage.wnd");
				if(legal)
				{
					legal->hide(FALSE);
					legal->bringForward();
					Int beginTime = timeGetTime();
					while(beginTime + 4000 > timeGetTime() )
					{
						TheWindowManager->update();
						// redraw all views, update the GUI
						TheDisplay->draw();
						Sleep(100);
					}
					setFPMode();


					legal->destroyWindows();
					deleteInstance(legal);

				}
				TheWritableGlobalData->m_breakTheMovie = TRUE;


			}

			TheShell->showShellMap(TRUE);
			TheShell->showShell();
			TheWritableGlobalData->m_afterIntro = FALSE;
		}
	}

	//Update snow particles.
	if (TheSnowManager)
		TheSnowManager->UPDATE();

	// update animation 2d collection
	TheAnim2DCollection->UPDATE();

	// update the keyboard
	if( TheKeyboard )
	{
		TheKeyboard->UPDATE();
		TheKeyboard->createStreamMessages();

	}  // end if

	// Update the Eva stuff
	TheEva->UPDATE();

	// update the mouse
	if( TheMouse )
	{
		TheMouse->UPDATE();
		TheMouse->createStreamMessages();

	}  // end if


  if (TheInGameUI->isCameraTrackingDrawable())
  {
    Drawable *draw = TheInGameUI->getFirstSelectedDrawable();
    if ( draw )
    {
      const Coord3D *pos = draw->getPosition();
      TheTacticalView->lookAt( pos );
    }
    else
      TheInGameUI->setCameraTrackingDrawable( FALSE );
  }

	if(TheGlobalData->m_playIntro || TheGlobalData->m_afterIntro)
	{
		// redraw all views, update the GUI
		TheDisplay->DRAW();
		TheDisplay->UPDATE();
		return;
	}

	// update the window system itself
	{
		TheWindowManager->UPDATE();
	}

	// update the video player
	{
		TheVideoPlayer->UPDATE();
	}

	Bool freezeTime = TheTacticalView->isTimeFrozen() && !TheTacticalView->isCameraMovementFinished();
	freezeTime = freezeTime || TheScriptEngine->isTimeFrozenDebug();
	freezeTime = freezeTime || TheScriptEngine->isTimeFrozenScript();
	freezeTime = freezeTime || TheGameLogic->isGamePaused();
	Int localPlayerIndex = ThePlayerList ? ThePlayerList->getLocalPlayer()->getPlayerIndex() : 0;

	// hack to let client spin fast in network games but still do effects at the same pace. -MDC
	static UnsignedInt lastFrame = ~0;
	freezeTime = freezeTime || (lastFrame == m_frame);
	lastFrame = m_frame;

	if (!freezeTime)
	{
#if ENABLE_CONFIGURABLE_SHROUD
		if (TheGlobalData->m_shroudOn)
#else
		if (true)
#endif
		{
			//localPlayerIndex=TheGhostObjectManager->getLocalPlayerIndex();	//always use the first local player set since normally can't change.  Doesn't work with debug "CTRL_SHIFT_SPACE"
#ifdef DEBUG_FOG_MEMORY
			//Find indices of all active players
			Int numPlayers=ThePlayerList->getPlayerCount();
			Int numNonLocalPlayers=0;
			Int nonLocalPlayerIndices[MAX_PLAYER_COUNT];
			for (Int i=0; i<numPlayers; i++)
			{	Player *player=ThePlayerList->getNthPlayer(i);
				//if (player->getPlayerType == PLAYER_HUMAN)
				if (player->getPlayerIndex() != localPlayerIndex)
					nonLocalPlayerIndices[numNonLocalPlayers++]=player->getPlayerIndex();
			}
			//update ghostObjects which don't have drawables or objects.
			TheGhostObjectManager->updateOrphanedObjects(nonLocalPlayerIndices,numNonLocalPlayers);
#else
			TheGhostObjectManager->updateOrphanedObjects(NULL,0);
#endif
		}


		// call the update for all client drawables
		Drawable* draw = firstDrawable();
		while (draw)
		{	// update() could free the Drawable, so go ahead and grab 'next'
			Drawable* next = draw->getNextDrawable();
#if ENABLE_CONFIGURABLE_SHROUD
			if (TheGlobalData->m_shroudOn)
#else
			if (true)
#endif
			{	//immobile objects need to take snapshots whenever they become fogged
				//so need to refresh their status.  We can't rely on external calls
				//to getShroudStatus() because they are only made for visible on-screen
				//objects.
				Object *object=draw->getObject();
				if (object)
				{
	#ifdef DEBUG_FOG_MEMORY
					Int *playerIndex=nonLocalPlayerIndices;
					for (i=0; i<numNonLocalPlayers; i++, playerIndex++)
						object->getShroudedStatus(*playerIndex);
	#endif
					ObjectShroudStatus ss=object->getShroudedStatus(localPlayerIndex);
					if (ss >= OBJECTSHROUD_FOGGED && draw->getShroudClearFrame()!=0) {
						UnsignedInt limit = 2*LOGICFRAMES_PER_SECOND;
						if (object->isEffectivelyDead()) {
							// extend the time, so we can see the dead plane blow up & crash.
							limit += 3*LOGICFRAMES_PER_SECOND;
						}
						if (TheGameLogic->getFrame() < limit + draw->getShroudClearFrame()) {
							// It's been less than 2 seconds since we could see them clear, so keep showing them.
							ss = OBJECTSHROUD_CLEAR;
						}
					}
					draw->setFullyObscuredByShroud(ss >= OBJECTSHROUD_FOGGED);
				}
			}
			draw->updateDrawable();
			draw = next;
		}
	}

#if defined(RTS_DEBUG)
	// need to draw the first frame, then don't draw again until TheGlobalData->m_noDraw
	if (TheGlobalData->m_noDraw > TheGameLogic->getFrame() && TheGameLogic->getFrame() > 0)
	{
		return;
	}
#endif

	// update all particle systems
	if( !freezeTime )
	{
		// update particle systems
		TheParticleSystemManager->setLocalPlayerIndex(localPlayerIndex);
//		TheParticleSystemManager->update();

	}  // end if

	// update the terrain visuals
	{
		TheTerrainVisual->UPDATE();
	}

	// update display
	{
		TheDisplay->UPDATE();
	}

	{
		USE_PERF_TIMER(GameClient_draw)

	// redraw all views, update the GUI
	//if(TheGameLogic->getFrame() >= 2)

		TheDisplay->DRAW();
	}

	{
		// let display string factory handle its update
		TheDisplayStringManager->update();
	}

	{
		// update the shell
		TheShell->UPDATE();
	}

	{
		// update the in game UI
		TheInGameUI->UPDATE();
	}
}  // end update

void GameClient::step()
{
	TheDisplay->step();
}

void GameClient::updateHeadless()
{
	// TheSuperHackers @info helmutbuhler 03/05/2025
	// When we play a replay back in headless mode, we want to skip the update of GameClient
	// because it's not necessary for CRC checking.
	// But we do reset the particles. The problem is that particles can be generated during
	// GameLogic and are only cleaned up during rendering. If we don't clean this up here,
	// the particles accumulate and slow things down a lot and can even cause a crash on
	// longer replays.
	TheParticleSystemManager->reset();
}

/** -----------------------------------------------------------------------------------------------
 * Call the given callback function for each object contained within the given region.
 */
void GameClient::iterateDrawablesInRegion( Region3D *region, GameClientFuncPtr userFunc, void *userData )
{
	Drawable *draw, *nextDrawable;

	for( draw = m_drawableList; draw; draw=nextDrawable )
	{
		nextDrawable = draw->getNextDrawable();

		Coord3D pos = *draw->getPosition();
		if( region == NULL ||
			  (pos.x >= region->lo.x && pos.x <= region->hi.x &&
			   pos.y >= region->lo.y && pos.y <= region->hi.y &&
				 pos.z >= region->lo.z && pos.z <= region->hi.z) )
		{
			(*userFunc)( draw, userData );
		}
	}
}

/**Helper function to update fake GLA structures to become visible to certain players.
We should only call this during critical moments, such as changing teams, changing to
observer, etc.*/
void GameClient::updateFakeDrawables(void)
{
	for( Drawable *draw = getDrawableList(); draw; draw = draw->getNextDrawable() )
	{
		const Object *object=draw->getObject();

		if( object && object->isKindOf( KINDOF_FS_FAKE ) )
		{
			Relationship rel=ThePlayerList->getLocalPlayer()->getRelationship(object->getTeam());
			if (rel == ALLIES || rel == NEUTRAL)
				draw->setTerrainDecal(TERRAIN_DECAL_SHADOW_TEXTURE);
			else
				draw->setTerrainDecal(TERRAIN_DECAL_NONE);
		}
	}
}

/** -----------------------------------------------------------------------------------------------
 * Destroy the drawable immediately.
 */
void GameClient::destroyDrawable( Drawable *draw )
{

	// remove any notion of the Drawable in the in-game user interface
	TheInGameUI->disregardDrawable( draw );

	// remove from the master list
	draw->removeFromList(&m_drawableList);

	//
	// because drawables and objects are tightly coupled, not only MUST we maintain
	// our links in all instances, but it is NECESSARY for the client to actually
	// modify data in the logic, that is the pointer in an object to *this* drawable
	//
	Object *obj = draw->getObject();
	if( obj )
	{

		DEBUG_ASSERTCRASH( obj->getDrawable() == draw, ("Object/Drawable pointer mismatch!") );
		obj->friend_bindToDrawable( NULL );

	}  // end if

	// remove the drawable from our hash of drawables
	removeDrawableFromLookupTable( draw );

	// free storage
	deleteInstance(draw);

}

// ------------------------------------------------------------------------------------------------
/** Add drawable to lookup table for fast id searching */
// ------------------------------------------------------------------------------------------------
void GameClient::addDrawableToLookupTable(Drawable *draw )
{

	// sanity
	if( draw == NULL )
		return;

	// add to lookup
//	m_drawableHash[ draw->getID() ] = draw;
	DrawableID newID = draw->getID();
	while( newID >= m_drawableVector.size() ) // Fail case is hella rare, so faster to double up on size() call
		m_drawableVector.resize(m_drawableVector.size() * 2, NULL);

	m_drawableVector[ newID ] = draw;

}  // end addDrawableToLookupTable

// ------------------------------------------------------------------------------------------------
/** Remove drawable from lookup table of fast id searching */
// ------------------------------------------------------------------------------------------------
void GameClient::removeDrawableFromLookupTable( Drawable *draw )
{

	// sanity
	// TheSuperHackers @fix Mauller/Xezon 24/04/2025 Prevent out of range access to vector lookup table
	if( draw == NULL || static_cast<size_t>(draw->getID()) >= m_drawableVector.size() )
		return;

	// remove from table
//	m_drawableHash.erase( draw->getID() );
	m_drawableVector[ draw->getID() ] = NULL;

}  // end removeDrawableFromLookupTable

//-------------------------------------------------------------------------------------------------
/** Load a map into the game interface */
Bool GameClient::loadMap( AsciiString mapName )
{

	// sanity
	if( mapName.isEmpty() )
		return false;

	assert( 0 );  // who calls this?

	return TRUE;

}  // end loadMap

//-------------------------------------------------------------------------------------------------
/** Unload a map from the game interface */
void GameClient::unloadMap( AsciiString mapName )
{

	assert( 0 );  // who calls this?

}  // end unloadMap

//-------------------------------------------------------------------------------------------------
void GameClient::setTimeOfDay( TimeOfDay tod )
{
	Drawable *draw = firstDrawable();

	while( draw )
	{
		draw->setTimeOfDay( tod );

		draw = draw->getNextDrawable();
	}
}

//-------------------------------------------------------------------------------------------------
void GameClient::assignSelectedDrawablesToGroup( Int group )
{
/*
	Drawable *draw = firstDrawable();
	while( draw )
	{

		if( draw->isSelected() && draw->getObject()->isLocallyControlled())
		{
			draw->setDrawableGroup( group );
		}
		else if( draw->getDrawableGroup() == group )
		{
			draw->setDrawableGroup( 0 );
		}

		draw = draw->getNextDrawable();

	}
*/
}

//-------------------------------------------------------------------------------------------------
void GameClient::selectDrawablesInGroup( Int group )
{
/*
	Drawable *draw = firstDrawable();

	// create a message that will assign a group ID to all the selected drawables, this
	// way when we do things with this current selected group of units, we only have
	// to refer to the group ID and not each individual object ID
	GameMessage *teamMsg = TheMessageStream->appendMessage( GameMessage::MSG_CREATE_SELECTED_GROUP );

	//We are creating a new group from scratch.
	teamMsg->appendBooleanArgument( true );

	while( draw )
	{
		int counter = 0;

		const Object *object = draw->getObject();

		if( object && draw->getDrawableGroup() == group && object->isLocallyControlled() && !object->isContained() )
		{
			//Only select the object if it is locally controlled and not contained by anything.
			TheInGameUI->selectDrawable(draw);
			teamMsg->appendObjectIDArgument( draw->getObject()->getID() );
		}
		else
		{
			TheInGameUI->deselectDrawable(draw);
		}

		draw = draw->getNextDrawable();
		counter++;
	}

*/
}

// ------------------------------------------------------------------------------------------------
void GameClient::addTextBearingDrawable( Drawable *tbd )
{
	if ( tbd != NULL )
		m_textBearingDrawableList.push_back( tbd );
}
// ------------------------------------------------------------------------------------------------
void GameClient::flushTextBearingDrawables( void )
{

	/////////////////////////////
	// WALK THIS LIST AND CALL EACH DRAWABLES TEXTY STUFF
	/////////////////////////////

	for( TextBearingDrawableListIterator it = m_textBearingDrawableList.begin(); it != m_textBearingDrawableList.end(); ++it )
	  (*it)->drawUIText();

	m_textBearingDrawableList.clear();
}

// ------------------------------------------------------------------------------------------------
GameMessage::Type GameClient::evaluateContextCommand( Drawable *draw,
																											const Coord3D *pos,
																											CommandTranslator::CommandEvaluateType cmdType )
{

	if( m_commandTranslator )
		return m_commandTranslator->evaluateContextCommand( draw, pos, cmdType );
	else
		return GameMessage::MSG_INVALID;

}  // end evaluateContextCommand

//-------------------------------------------------------------------------------------------------
/** Get the ray effect data for a drawable */
void GameClient::getRayEffectData( Drawable *draw, RayEffectData *effectData )
{

	TheRayEffects->getRayEffectData( draw, effectData );

}  // end getRayEffectData

//-------------------------------------------------------------------------------------------------
/** remove the drawble from the ray effects sytem if present */
void GameClient::removeFromRayEffects( Drawable *draw )
{

	TheRayEffects->deleteRayEffect( draw );

}  // end removeFromRayEffects

/** frees all shadow resources used by this module - used by Options screen.*/
void GameClient::releaseShadows(void)
{
	Drawable *draw;
	for( draw = firstDrawable(); draw; draw = draw->getNextDrawable() )
		draw->releaseShadows();
}

/** create shadow resources if not already present. Used by Options screen.*/
void GameClient::allocateShadows(void)
{
	Drawable *draw;
	for( draw = firstDrawable(); draw; draw = draw->getNextDrawable() )
		draw->allocateShadows();
}

//-------------------------------------------------------------------------------------------------
/** Preload assets for the currently loaded map.  Those assets include all the damage states
	* for every building loaded, as well as any faction units/structures we can build and
	* all their damage states */
//-------------------------------------------------------------------------------------------------
void GameClient::preloadAssets( TimeOfDay timeOfDay )
{

	MEMORYSTATUS before, after;
	GlobalMemoryStatus(&before);

	// first, for every drawable in the map load the assets for all states we care about
	Drawable *draw;
	for( draw = firstDrawable(); draw; draw = draw->getNextDrawable() )
		draw->preloadAssets( timeOfDay );

	//
	// now create a temporary drawble for each of the faction things we can create, preload
	// their assets, and dump the drawable
	//
	AsciiString side;
	const ThingTemplate *tTemplate;
	for( tTemplate = TheThingFactory->firstTemplate();
			 tTemplate;
			 tTemplate = tTemplate->friend_getNextTemplate() )
	{

		// if this isn't one of the objects that can be preloaded ignore it
		if( tTemplate->isKindOf( KINDOF_PRELOAD ) == FALSE && !TheGlobalData->m_preloadEverything )
			continue;

		// create the drawable and do the preloading
		draw = TheThingFactory->newDrawable( tTemplate );
		if( draw )
		{

			// preload the assets
			draw->preloadAssets( timeOfDay );

			// destroy the drawable
			TheGameClient->destroyDrawable( draw );

		}  // end if

	}  // end for
	GlobalMemoryStatus(&after);

	DEBUG_LOG(("Preloading memory dwAvailPageFile %d --> %d : %d",
		before.dwAvailPageFile, after.dwAvailPageFile, before.dwAvailPageFile - after.dwAvailPageFile));
	DEBUG_LOG(("Preloading memory dwAvailPhys     %d --> %d : %d",
		before.dwAvailPhys, after.dwAvailPhys, before.dwAvailPhys - after.dwAvailPhys));
	DEBUG_LOG(("Preloading memory dwAvailVirtual  %d --> %d : %d",
		before.dwAvailVirtual, after.dwAvailVirtual, before.dwAvailVirtual - after.dwAvailVirtual));
	/*
	DEBUG_LOG(("Preloading memory dwLength        %d --> %d : %d",
		before.dwLength, after.dwLength, before.dwLength - after.dwLength));
	DEBUG_LOG(("Preloading memory dwMemoryLoad    %d --> %d : %d",
		before.dwMemoryLoad, after.dwMemoryLoad, before.dwMemoryLoad - after.dwMemoryLoad));
	DEBUG_LOG(("Preloading memory dwTotalPageFile %d --> %d : %d",
		before.dwTotalPageFile, after.dwTotalPageFile, before.dwTotalPageFile - after.dwTotalPageFile));
	DEBUG_LOG(("Preloading memory dwTotalPhys     %d --> %d : %d",
		before.dwTotalPhys , after.dwTotalPhys, before.dwTotalPhys - after.dwTotalPhys));
	DEBUG_LOG(("Preloading memory dwTotalVirtual  %d --> %d : %d",
		before.dwTotalVirtual , after.dwTotalVirtual, before.dwTotalVirtual - after.dwTotalVirtual));
	*/

	GlobalMemoryStatus(&before);
	extern std::vector<AsciiString>	debrisModelNamesGlobalHack;
	size_t i=0;
	for (; i<debrisModelNamesGlobalHack.size(); ++i)
	{
		TheDisplay->preloadModelAssets(debrisModelNamesGlobalHack[i]);
	}
	GlobalMemoryStatus(&after);
	debrisModelNamesGlobalHack.clear();

	DEBUG_LOG(("Preloading memory dwAvailPageFile %d --> %d : %d",
		before.dwAvailPageFile, after.dwAvailPageFile, before.dwAvailPageFile - after.dwAvailPageFile));
	DEBUG_LOG(("Preloading memory dwAvailPhys     %d --> %d : %d",
		before.dwAvailPhys, after.dwAvailPhys, before.dwAvailPhys - after.dwAvailPhys));
	DEBUG_LOG(("Preloading memory dwAvailVirtual  %d --> %d : %d",
		before.dwAvailVirtual, after.dwAvailVirtual, before.dwAvailVirtual - after.dwAvailVirtual));

	TheControlBar->preloadAssets( timeOfDay );

	GlobalMemoryStatus(&before);
	TheParticleSystemManager->preloadAssets( timeOfDay );
	GlobalMemoryStatus(&after);

	DEBUG_LOG(("Preloading memory dwAvailPageFile %d --> %d : %d",
		before.dwAvailPageFile, after.dwAvailPageFile, before.dwAvailPageFile - after.dwAvailPageFile));
	DEBUG_LOG(("Preloading memory dwAvailPhys     %d --> %d : %d",
		before.dwAvailPhys, after.dwAvailPhys, before.dwAvailPhys - after.dwAvailPhys));
	DEBUG_LOG(("Preloading memory dwAvailVirtual  %d --> %d : %d",
		before.dwAvailVirtual, after.dwAvailVirtual, before.dwAvailVirtual - after.dwAvailVirtual));

	const char *textureNames[] = {
		"ptspruce01.tga",
		"exrktflame.tga",
		"cvlimo3_d2.tga",
		"exfthrowerstream.tga",
		"uvrockbug_d1.tga",
		"arcbackgroundc.tga",
		"grade3.tga",
		"framebasec.tga",
		"gradec.tga",
		"frametopc.tga",
		"arcbackgrounda.tga",
		"arcglow2.tga",
		"framebasea.tga",
		"gradea.tga",
		"frametopa.tga",
		"sauserinterface256_002.tga",
		"sauserinterface256_001.tga",
		"unitbackgrounda.tga",
		"sauserinterface256_004.tga",
		"sagentank.tga",
		"sauserinterface256_005.tga",
		"sagenair.tga",
		"sauserinterface256_003.tga",
		"sagenspec.tga",
		"snuserinterface256_003.tga",
		"snuserinterface256_002.tga",
		"unitbackgroundc.tga",
		"snuserinterface256_004.tga",
		"sngenredarm.tga",
		"snuserinterface256_001.tga",
		"sngenspewea.tga",
		"sngensecpol.tga",
		"ciburn.tga",
		"ptmaple02.tga",
		"scuserinterface256_005.tga",
		"scuserinterface256_002.tga",
		"sauserinterface256_006.tga",
		"pmcrates.tga",
		""
	};

	GlobalMemoryStatus(&before);
	for (i=0; *textureNames[i]; ++i)
		TheDisplay->preloadTextureAssets(textureNames[i]);
	GlobalMemoryStatus(&after);

	DEBUG_LOG(("Preloading memory dwAvailPageFile %d --> %d : %d",
		before.dwAvailPageFile, after.dwAvailPageFile, before.dwAvailPageFile - after.dwAvailPageFile));
	DEBUG_LOG(("Preloading memory dwAvailPhys     %d --> %d : %d",
		before.dwAvailPhys, after.dwAvailPhys, before.dwAvailPhys - after.dwAvailPhys));
	DEBUG_LOG(("Preloading memory dwAvailVirtual  %d --> %d : %d",
		before.dwAvailVirtual, after.dwAvailVirtual, before.dwAvailVirtual - after.dwAvailVirtual));

//	preloadTextureNamesGlobalHack2 = preloadTextureNamesGlobalHack;
//	preloadTextureNamesGlobalHack.clear();

}  // end preloadAssets

// ------------------------------------------------------------------------------------------------
/** Given a string name, find the drawable TOC entry (if any) associated with it */
// ------------------------------------------------------------------------------------------------
GameClient::DrawableTOCEntry *GameClient::findTOCEntryByName( AsciiString name )
{

	for( DrawableTOCListIterator it = m_drawableTOC.begin(); it != m_drawableTOC.end(); ++it )
		if( (*it).name == name )
			return &(*it);

	return NULL;

}  // end findTOCEntryByname

// ------------------------------------------------------------------------------------------------
/** Given a drawable TOC identifier, find the drawable TOC if any */
// ------------------------------------------------------------------------------------------------
GameClient::DrawableTOCEntry *GameClient::findTOCEntryById( UnsignedShort id )
{

	for( DrawableTOCListIterator it = m_drawableTOC.begin(); it != m_drawableTOC.end(); ++it )
		if( (*it).id == id )
			return &(*it);

	return NULL;

}  // end findTOCEntryById

// ------------------------------------------------------------------------------------------------
/** Add an drawable TOC entry */
// ------------------------------------------------------------------------------------------------
void GameClient::addTOCEntry( AsciiString name, UnsignedShort id )
{

	DrawableTOCEntry tocEntry;
	tocEntry.name = name;
	tocEntry.id = id;
	m_drawableTOC.push_back( tocEntry );

}  // end addTOCEntry

// ------------------------------------------------------------------------------------------------
static Bool shouldSaveDrawable(const Drawable* draw)
{
	if (draw->testDrawableStatus(DRAWABLE_STATUS_NO_SAVE))
	{
		if (draw->getObject() == NULL)
		{
			return false;
		}
		else
		{
			DEBUG_CRASH(("You should not ever set DRAWABLE_STATUS_NO_SAVE for a Drawable with an object. (%s)",draw->getTemplate()->getName().str()));
		}
	}
	return true;
}

// ------------------------------------------------------------------------------------------------
/** Xfer drawable table of contents */
// ------------------------------------------------------------------------------------------------
void GameClient::xferDrawableTOC( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// clear our current table of contents
	m_drawableTOC.clear();

	// xfer the table
	UnsignedInt tocCount = 0;
	if( xfer->getXferMode() == XFER_SAVE )
	{
		AsciiString templateName;

		// generate a new TOC based on the drawables that are in the map
		for( Drawable *draw = getDrawableList(); draw; draw = draw->getNextDrawable() )
		{
			if (!shouldSaveDrawable(draw))
				continue;

			// get the name we're working with
			templateName = draw->getTemplate()->getName();

			// if is this drawable name already in the TOC, skip it
			if( findTOCEntryByName( templateName ) != NULL )
				continue;

			// add this entry to the TOC
			addTOCEntry( draw->getTemplate()->getName(), ++tocCount );

		}  // end for obj

		// xfer entries in the TOC
		xfer->xferUnsignedInt( &tocCount );

		// xfer each TOC entry
		DrawableTOCListIterator it;
		DrawableTOCEntry *tocEntry;
		for( it = m_drawableTOC.begin(); it != m_drawableTOC.end(); ++it )
		{

			// get this toc entry
			tocEntry = &(*it);

			// xfer the name
			xfer->xferAsciiString( &tocEntry->name );

			// xfer the paired id
			xfer->xferUnsignedShort( &tocEntry->id );

		}  // end for

	}  // end if
	else
	{
		AsciiString templateName;
		UnsignedShort id;

		// how many entries are we going to read
		xfer->xferUnsignedInt( &tocCount );

		// read all the entries
		for( UnsignedInt i = 0; i < tocCount; ++i )
		{

			// read the name
			xfer->xferAsciiString( &templateName );

			// read the id
			xfer->xferUnsignedShort( &id );

			// add this to the TOC
			addTOCEntry( templateName, id );

		}  // end for i

	}  // end else

}  // end xferDrawableTOC

// ------------------------------------------------------------------------------------------------
/** Xfer method for Game Client
  * Version History:
  * 1: Initial
  * 2: Adding mission briefing history
	* 3: Added block markers around drawable data, no version checking is done and therefore
	*		 this version breaks compatibility with previous versions. (CBD)
 */
// ------------------------------------------------------------------------------------------------
void GameClient::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 3;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// client frame number
	xfer->xferUnsignedInt( &m_frame );

	//
	// note that we do not do the id counter here, we did it in the game state block because
	// it's important to do that part very early in the load process
	//
	// !!!DON'T DO THIS!!! ----> xfer->xferDrawableID( &m_nextDrawableID ); <---- !!!DON'T DO THIS!!!

	//
	// xfer a table of contents that contain thing template and indentifier pairs.  this
	// table of contents is good for this save file only as unique numbers are only
	// generated and stored for the actual things that are on this map
	//
	xferDrawableTOC( xfer );

	// drawable count
	Drawable *draw;
	UnsignedShort drawableCount = 0;
	for( draw = getDrawableList(); draw; draw = draw->getNextDrawable() )
	{
		if (xfer->getXferMode() == XFER_SAVE && !shouldSaveDrawable(draw))
			continue;
		drawableCount++;
	}
	xfer->xferUnsignedShort( &drawableCount );

	// drawable data
	DrawableTOCEntry *tocEntry;
	ObjectID objectID;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// iterate all drawables
		for( draw = getDrawableList(); draw; draw = draw->getNextDrawable() )
		{
			if (!shouldSaveDrawable(draw))
				continue;

			// get TOC entry for this drawable
			tocEntry = findTOCEntryByName( draw->getTemplate()->getName() );
			if( tocEntry == NULL )
			{

				DEBUG_CRASH(( "GameClient::xfer - Drawable TOC entry not found for '%s'", draw->getTemplate()->getName().str() ));
				throw SC_INVALID_DATA;

			}  // end if

			// xfer toc id entry
			xfer->xferUnsignedShort( &tocEntry->id );

			// begin data block
			xfer->beginBlock();

			// write the object ID this drawable is attached to
			objectID = draw->getObject() ? draw->getObject()->getID() : INVALID_ID;
			xfer->xferObjectID( &objectID );

			// write snapshot data
			xfer->xferSnapshot( draw );

			// end data block
			xfer->endBlock();

		}  // end for, draw

	}  // end if, save
	else
	{
		UnsignedShort tocID;
		const ThingTemplate *thingTemplate;
		Int dataSize;

		// read all entries
		for( UnsignedShort i = 0; i < drawableCount; ++i )
		{

			// read toc id entry
			xfer->xferUnsignedShort( &tocID );

			// find TOC entry with this identifier
			tocEntry = findTOCEntryById( tocID );
			if( tocEntry == NULL )
			{

				DEBUG_CRASH(( "GameClient::xfer - No TOC entry match for id '%d'", tocID ));
				throw SC_INVALID_DATA;

			}  // end if

			// read data block size
			dataSize = xfer->beginBlock();

			// find matching thing template
			thingTemplate = TheThingFactory->findTemplate( tocEntry->name );
			if( thingTemplate == NULL )
			{

				DEBUG_CRASH(( "GameClient::xfer - Unrecognized thing template '%s', skipping.  ENGINEERS - Are you *sure* it's OK to be ignoring this object from the save file???  Think hard about it!",
											tocEntry->name.str() ));
				xfer->skip( dataSize );
				continue;

			}  // end if

			// read the object ID this drawable is attached to (if any)
			xfer->xferObjectID( &objectID );

			//
			// if we have an attached object ID, we won't create a new drawable, we'll use the
			// one that has been created and attached to the object already
			//
			if( objectID != INVALID_ID )
			{
				Object *object = TheGameLogic->findObjectByID( objectID );

				// sanity
				if( object == NULL )
				{

					DEBUG_CRASH(( "GameClient::xfer - Cannot find object '%d' that is supposed to be attached to this drawable '%s'",
												objectID, thingTemplate->getName().str() ));
					throw SC_INVALID_DATA;

				}  // end if

				// get the drawable from the object
				draw = object->getDrawable();
				if( draw == NULL )
				{

					DEBUG_CRASH(( "GameClient::xfer - There is no drawable attached to the object '%s' (%d) and there should be",
												object->getTemplate()->getName().str(), object->getID() ));
					throw SC_INVALID_DATA;

				}  // end if

				// srj sez: some objects (eg, diguised bombtrucks) may have an "abnormal" drawable. so check.
				//
				// note carefully: we do NOT want to use isEquivalentTo() here, because different object reskins
				// SHOULD count as different templates for our purposes here (which are purely visual). however, we
				// do need to compare getFinalOverride, because draw->getTemplate() is always gonna return the final
				// override, while TheThingFactory->findTemplate does not.
				//
				const ThingTemplate* drawTemplate = draw->getTemplate();
				if (drawTemplate->getFinalOverride() != thingTemplate->getFinalOverride())
				{
					TheGameClient->destroyDrawable( draw );
					draw = TheThingFactory->newDrawable( thingTemplate );
					TheGameLogic->bindObjectAndDrawable(object, draw);
				}

			}  // end if
			else
			{

				//
				// there was no object attached to this drawable when we saved, we need to create a
				// whole brand new drawable now
				//
				draw = TheThingFactory->newDrawable( thingTemplate );

				// sanity
				if( draw == NULL )
				{

					DEBUG_CRASH(( "GameClient::xfer - Unable to create drawable for '%s'",
												thingTemplate->getName().str() ));
					throw SC_INVALID_DATA;

				}  // end if

			}  // end else

			// xfer the drawable data
			xfer->xferSnapshot( draw );

			// end block (not necessary since this is a no-op but symettrically nice)
			xfer->endBlock();

		}  // end for, i

	}  // end else, load

	// xfer the in-game mission briefing history list
	if (version >= 2)
	{
		if( xfer->getXferMode() == XFER_SAVE )
		{
			BriefingList *bList = GetBriefingTextList();
			Int numEntries = bList->size();
			xfer->xferInt(&numEntries);
			DEBUG_LOG(("Saving %d briefing lines", numEntries));
			for (BriefingList::const_iterator bIt = bList->begin(); bIt != bList->end(); ++bIt)
			{
				AsciiString tempStr = *bIt;
				DEBUG_LOG(("'%s'", tempStr.str()));
				xfer->xferAsciiString(&tempStr);
			}
		}
		else // XFER_LOAD
		{
			Int numEntries = 0;
			xfer->xferInt(&numEntries);
			DEBUG_LOG(("Loading %d briefing lines", numEntries));
			UpdateDiplomacyBriefingText(AsciiString::TheEmptyString, TRUE); // clear out briefing list first
			while (numEntries-- > 0)
			{
				AsciiString tempStr;
				xfer->xferAsciiString(&tempStr);
				DEBUG_LOG(("'%s'", tempStr.str()));
				UpdateDiplomacyBriefingText(tempStr, FALSE);
			}
		}
	}

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void GameClient::loadPostProcess( void )
{

	//
	// due to the fact that during the load process we have called newDrawable for drawables
	// without objects, and then overwrote their ids with data from the save file, our allocater
	// id may be far higher than it needs to be.  We'll pull it back down as low as we can
	//
	Drawable *draw;
	for( draw = getDrawableList(); draw; draw = draw->getNextDrawable() )
		if( draw->getID() >= m_nextDrawableID )
			m_nextDrawableID = (DrawableID)((UnsignedInt)draw->getID() + 1);

}  // end loadPostProcess

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void GameClient::crc( Xfer *xfer )
{

}  // end crc
