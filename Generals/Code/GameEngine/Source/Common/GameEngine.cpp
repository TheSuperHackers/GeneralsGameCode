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

// GameEngine.cpp /////////////////////////////////////////////////////////////////////////////////
// Implementation of the Game Engine singleton
// Author: Michael S. Booth, April 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ActionManager.h"
#include "Common/AudioAffect.h"
#include "Common/BuildAssistant.h"
#include "Common/CRCDebug.h"
#include "Common/Radar.h"
#include "Common/PlayerTemplate.h"
#include "Common/Team.h"
#include "Common/PlayerList.h"
#include "Common/GameAudio.h"
#include "Common/GameEngine.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/MessageStream.h"
#include "Common/ThingFactory.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/FrameRateLimit.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/LocalFileSystem.h"
#include "Common/CDManager.h"
#include "Common/GlobalData.h"
#include "Common/PerfTimer.h"
#include "Common/RandomValue.h"
#include "Common/NameKeyGenerator.h"
#include "Common/ModuleFactory.h"
#include "Common/Debug.h"
#include "Common/GameState.h"
#include "Common/GameStateMap.h"
#include "Common/Science.h"
#include "Common/FunctionLexicon.h"
#include "Common/CommandLine.h"
#include "Common/DamageFX.h"
#include "Common/MultiplayerSettings.h"
#include "Common/Recorder.h"
#include "Common/SpecialPower.h"
#include "Common/TerrainTypes.h"
#include "Common/Upgrade.h"
#include "Common/UserPreferences.h"
#include "Common/Xfer.h"
#include "Common/XferCRC.h"
#include "Common/GameLOD.h"
#include "Common/Registry.h"

#include "GameLogic/Armor.h"
#include "GameLogic/AI.h"
#include "GameLogic/CaveSystem.h"
#include "GameLogic/CrateSystem.h"
#include "GameLogic/VictoryConditions.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/RankInfo.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/SidesList.h"

#include "GameClient/ClientInstance.h"
#include "GameClient/FXList.h"
#include "GameClient/GameClient.h"
#include "GameClient/Keyboard.h"
#include "GameClient/Shell.h"
#include "GameClient/GameText.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Water.h"
#include "GameClient/TerrainRoads.h"
#include "GameClient/MetaEvent.h"
#include "GameClient/MapUtil.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GlobalLanguage.h"
#include "GameClient/Drawable.h"
#include "GameClient/GUICallbacks.h"

#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/WOLBrowser/WebBrowser.h"
#include "GameNetwork/LANAPI.h"
#include "GameNetwork/GameSpy/GameResultsThread.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "Common/Player.h"


#include "Common/version.h"


//-------------------------------------------------------------------------------------------------

#ifdef DEBUG_CRC
class DeepCRCSanityCheck : public SubsystemInterface
{
public:
	DeepCRCSanityCheck() {}
	virtual ~DeepCRCSanityCheck() {}

	virtual void init(void) {}
	virtual void reset(void);
	virtual void update(void) {}

protected:
};

DeepCRCSanityCheck *TheDeepCRCSanityCheck = NULL;

void DeepCRCSanityCheck::reset(void)
{
	static Int timesThrough = 0;
	static UnsignedInt lastCRC = 0;

	AsciiString fname;
	fname.format("%sCRCAfter%dMaps.dat", TheGlobalData->getPath_UserData().str(), timesThrough);
	UnsignedInt thisCRC = TheGameLogic->getCRC( CRC_RECALC, fname );

	DEBUG_LOG(("DeepCRCSanityCheck: CRC is %X", thisCRC));
	DEBUG_ASSERTCRASH(timesThrough == 0 || thisCRC == lastCRC,
		("CRC after reset did not match beginning CRC!\nNetwork games won't work after this.\nOld: 0x%8.8X, New: 0x%8.8X",
		lastCRC, thisCRC));
	lastCRC = thisCRC;

	timesThrough++;
}
#endif // DEBUG_CRC

//-------------------------------------------------------------------------------------------------
/// The GameEngine singleton instance
GameEngine *TheGameEngine = NULL;

//-------------------------------------------------------------------------------------------------
SubsystemInterfaceList* TheSubsystemList = NULL;

//-------------------------------------------------------------------------------------------------
template<class SUBSYSTEM>
void initSubsystem(
	SUBSYSTEM*& sysref,
	AsciiString name,
	SUBSYSTEM* sys,
	Xfer *pXfer,
	const char* path1 = NULL,
	const char* path2 = NULL)
{
	sysref = sys;
	TheSubsystemList->initSubsystem(sys, path1, path2, pXfer, name);
}

//-------------------------------------------------------------------------------------------------
extern HINSTANCE ApplicationHInstance;  ///< our application instance
extern CComModule _Module;

//-------------------------------------------------------------------------------------------------
static void updateTGAtoDDS();

//-------------------------------------------------------------------------------------------------
static void updateWindowTitle()
{
	// TheSuperHackers @tweak Now prints product and version information in the Window title.

	DEBUG_ASSERTCRASH(TheVersion != NULL, ("TheVersion is NULL"));
	DEBUG_ASSERTCRASH(TheGameText != NULL, ("TheGameText is NULL"));

	UnicodeString title;

	if (rts::ClientInstance::getInstanceId() > 1u)
	{
		UnicodeString str;
		str.format(L"Instance:%.2u", rts::ClientInstance::getInstanceId());
		title.concat(str);
	}

	UnicodeString productString = TheVersion->getUnicodeProductString();

	if (!productString.isEmpty())
	{
		if (!title.isEmpty())
			title.concat(L" ");
		title.concat(productString);
	}

#if RTS_GENERALS
	const WideChar* defaultGameTitle = L"Command and Conquer Generals";
#elif RTS_ZEROHOUR
	const WideChar* defaultGameTitle = L"Command and Conquer Generals Zero Hour";
#endif
	UnicodeString gameTitle = TheGameText->FETCH_OR_SUBSTITUTE("GUI:Command&ConquerGenerals", defaultGameTitle);

	if (!gameTitle.isEmpty())
	{
		UnicodeString gameTitleFinal;
		UnicodeString gameVersion = TheVersion->getUnicodeVersion();

		if (productString.isEmpty())
		{
			gameTitleFinal = gameTitle;
		}
		else
		{
			UnicodeString gameTitleFormat = TheGameText->FETCH_OR_SUBSTITUTE("Version:GameTitle", L"for %ls");
			gameTitleFinal.format(gameTitleFormat.str(), gameTitle.str());
		}

		if (!title.isEmpty())
			title.concat(L" ");
		title.concat(gameTitleFinal.str());
		title.concat(L" ");
		title.concat(gameVersion.str());
	}

	if (!title.isEmpty())
	{
		AsciiString titleA;
		titleA.translate(title);	//get ASCII version for Win 9x

		extern HWND ApplicationHWnd;  ///< our application window handle
		if (ApplicationHWnd) {
			//Set it twice because Win 9x does not support SetWindowTextW.
			::SetWindowText(ApplicationHWnd, titleA.str());
			::SetWindowTextW(ApplicationHWnd, title.str());
		}
	}
}

//-------------------------------------------------------------------------------------------------
GameEngine::GameEngine( void )
{
	// Set the time slice size to 1 ms.
	timeBeginPeriod(1);

	// initialize to non garbage values
	m_maxFPS = BaseFps;
	m_logicTimeScaleFPS = LOGICFRAMES_PER_SECOND;
	m_updateTime = 1.0f / BaseFps; // initialized to something to avoid division by zero on first use
	m_logicTimeAccumulator = 0.0f;
	m_quitting = FALSE;
	m_isActive = FALSE;
	m_enableLogicTimeScale = FALSE;
	m_isTimeFrozen = FALSE;
	m_isGameHalted = FALSE;

	_Module.Init(NULL, ApplicationHInstance, NULL);
}

//-------------------------------------------------------------------------------------------------
GameEngine::~GameEngine()
{
	//extern std::vector<std::string>	preloadTextureNamesGlobalHack;
	//preloadTextureNamesGlobalHack.clear();

	delete TheMapCache;
	TheMapCache = NULL;

//	delete TheShell;
//	TheShell = NULL;

	TheGameResultsQueue->endThreads();

	// TheSuperHackers @fix helmutbuhler 03/06/2025
	// Reset all subsystems before deletion to prevent crashing due to cross dependencies.
	reset();

	TheSubsystemList->shutdownAll();
	delete TheSubsystemList;
	TheSubsystemList = NULL;

	delete TheNetwork;
	TheNetwork = NULL;

	delete TheCommandList;
	TheCommandList = NULL;

	delete TheNameKeyGenerator;
	TheNameKeyGenerator = NULL;

	delete TheFileSystem;
	TheFileSystem = NULL;

	Drawable::killStaticImages();

	_Module.Term();

#ifdef PERF_TIMERS
	PerfGather::termPerfDump();
#endif

	// Restore the previous time slice for Windows.
	timeEndPeriod(1);
}

//-------------------------------------------------------------------------------------------------
void GameEngine::setFramesPerSecondLimit( Int fps )
{
	DEBUG_LOG(("GameEngine::setFramesPerSecondLimit() - setting max fps to %d (TheGlobalData->m_useFpsLimit == %d)", fps, TheGlobalData->m_useFpsLimit));
	m_maxFPS = fps;
}

//-------------------------------------------------------------------------------------------------
Int GameEngine::getFramesPerSecondLimit( void )
{
	return m_maxFPS;
}

//-------------------------------------------------------------------------------------------------
Real GameEngine::getUpdateTime()
{
	return m_updateTime;
}

//-------------------------------------------------------------------------------------------------
Real GameEngine::getUpdateFps()
{
	return 1.0f / m_updateTime;
}

//-------------------------------------------------------------------------------------------------
Bool GameEngine::isTimeFrozen()
{
	// TheSuperHackers @fix The time can no longer be frozen in Network games. It would disconnect the player.
	if (TheNetwork != NULL)
		return false;

	if (TheTacticalView != NULL)
	{
		if (TheTacticalView->isTimeFrozen() && !TheTacticalView->isCameraMovementFinished())
			return true;
	}

	if (TheScriptEngine != NULL)
	{
		if (TheScriptEngine->isTimeFrozenDebug() || TheScriptEngine->isTimeFrozenScript())
			return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
Bool GameEngine::isGameHalted()
{
	if (TheNetwork != NULL)
	{
		if (TheNetwork->isStalling())
			return true;
	}
	else
	{
		if (TheGameLogic != NULL && TheGameLogic->isGamePaused())
			return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
void GameEngine::setLogicTimeScaleFps( Int fps )
{
	m_logicTimeScaleFPS = fps;
}

//-------------------------------------------------------------------------------------------------
Int GameEngine::getLogicTimeScaleFps()
{
	return m_logicTimeScaleFPS;
}

//-------------------------------------------------------------------------------------------------
void GameEngine::enableLogicTimeScale( Bool enable )
{
	m_enableLogicTimeScale = enable;
}

//-------------------------------------------------------------------------------------------------
Bool GameEngine::isLogicTimeScaleEnabled()
{
	return m_enableLogicTimeScale;
}

//-------------------------------------------------------------------------------------------------
Int GameEngine::getActualLogicTimeScaleFps(LogicTimeQueryFlags flags)
{
	if (m_isTimeFrozen && (flags & IgnoreFrozenTime) == 0)
	{
		return 0;
	}

	if (m_isGameHalted && (flags & IgnoreHaltedGame) == 0)
	{
		return 0;
	}

	if (TheNetwork != NULL)
	{
		return TheNetwork->getFrameRate();
	}

	if (isLogicTimeScaleEnabled())
	{
		return min(getLogicTimeScaleFps(), getFramesPerSecondLimit());
	}

	return getFramesPerSecondLimit();
}

//-------------------------------------------------------------------------------------------------
Real GameEngine::getActualLogicTimeScaleRatio(LogicTimeQueryFlags flags)
{
	return (Real)getActualLogicTimeScaleFps(flags) / LOGICFRAMES_PER_SECONDS_REAL;
}

//-------------------------------------------------------------------------------------------------
Real GameEngine::getActualLogicTimeScaleOverFpsRatio(LogicTimeQueryFlags flags)
{
	// TheSuperHackers @info Clamps ratio to min 1, because the logic
	// frame rate is currently capped by the render frame rate.
	return min(1.0f, (Real)getActualLogicTimeScaleFps(flags) / getUpdateFps());
}

Real GameEngine::getLogicTimeStepSeconds(LogicTimeQueryFlags flags)
{
	return SECONDS_PER_LOGICFRAME_REAL * getActualLogicTimeScaleOverFpsRatio(flags);
}

Real GameEngine::getLogicTimeStepMilliseconds(LogicTimeQueryFlags flags)
{
	return MSEC_PER_LOGICFRAME_REAL * getActualLogicTimeScaleOverFpsRatio(flags);
}

/** -----------------------------------------------------------------------------------------------
 * Initialize the game engine by initializing the GameLogic and GameClient.
 */
void GameEngine::init()
{
	try {
		//create an INI object to use for loading stuff
		INI ini;

		if (TheVersion)
		{
			DEBUG_LOG(("================================================================================"));
			DEBUG_LOG(("Generals version %s", TheVersion->getAsciiVersion().str()));
			DEBUG_LOG(("Build date: %s", TheVersion->getAsciiBuildTime().str()));
			DEBUG_LOG(("Build location: %s", TheVersion->getAsciiBuildLocation().str()));
			DEBUG_LOG(("Build user: %s", TheVersion->getAsciiBuildUser().str()));
			DEBUG_LOG(("Build git revision: %s", TheVersion->getAsciiGitCommitCount().str()));
			DEBUG_LOG(("Build git version: %s", TheVersion->getAsciiGitTagOrHash().str()));
			DEBUG_LOG(("Build git commit time: %s", TheVersion->getAsciiGitCommitTime().str()));
			DEBUG_LOG(("Build git commit author: %s", Version::getGitCommitAuthorName()));
			DEBUG_LOG(("================================================================================"));
		}

		TheSubsystemList = MSGNEW("GameEngineSubsystem") SubsystemInterfaceList;

		TheSubsystemList->addSubsystem(this);

		// initialize the random number system
		InitRandom();

		// Create the low-level file system interface
		TheFileSystem = createFileSystem();

		// not part of the subsystem list, because it should normally never be reset!
		TheNameKeyGenerator = MSGNEW("GameEngineSubsystem") NameKeyGenerator;
		TheNameKeyGenerator->init();

		// not part of the subsystem list, because it should normally never be reset!
		TheCommandList = MSGNEW("GameEngineSubsystem") CommandList;
		TheCommandList->init();

		XferCRC xferCRC;
		xferCRC.open("lightCRC");

		initSubsystem(TheLocalFileSystem, "TheLocalFileSystem", createLocalFileSystem(), NULL);
		initSubsystem(TheArchiveFileSystem, "TheArchiveFileSystem", createArchiveFileSystem(), NULL); // this MUST come after TheLocalFileSystem creation

		DEBUG_ASSERTCRASH(TheWritableGlobalData,("TheWritableGlobalData expected to be created"));
		initSubsystem(TheWritableGlobalData, "TheWritableGlobalData", TheWritableGlobalData, &xferCRC, "Data\\INI\\Default\\GameData", "Data\\INI\\GameData");
		TheWritableGlobalData->parseCustomDefinition();



	#if defined(RTS_DEBUG)
		// If we're in Debug, load the Debug settings as well.
		ini.loadFileDirectory( AsciiString( "Data\\INI\\GameDataDebug" ), INI_LOAD_OVERWRITE, NULL );
	#endif

		// special-case: parse command-line parameters after loading global data
		CommandLine::parseCommandLineForEngineInit();

		TheArchiveFileSystem->loadMods();

		// doesn't require resets so just create a single instance here.
		TheGameLODManager = MSGNEW("GameEngineSubsystem") GameLODManager;
		TheGameLODManager->init();

		// after parsing the command line, we may want to perform dds stuff. Do that here.
		if (TheGlobalData->m_shouldUpdateTGAToDDS) {
			// update any out of date targas here.
			updateTGAtoDDS();
		}

	#if defined(PERF_TIMERS) || defined(DUMP_PERF_STATS)
		DEBUG_LOG(("Calculating CPU frequency for performance timers."));
		InitPrecisionTimer();
	#endif
	#ifdef PERF_TIMERS
		PerfGather::initPerfDump("AAAPerfStats", PerfGather::PERF_NETTIME);
	#endif

		// read the water settings from INI (must do prior to initing GameClient, apparently)
		ini.loadFileDirectory( AsciiString( "Data\\INI\\Default\\Water" ), INI_LOAD_OVERWRITE, &xferCRC );
		ini.loadFileDirectory( AsciiString( "Data\\INI\\Water" ), INI_LOAD_OVERWRITE, &xferCRC );

#ifdef DEBUG_CRC
		initSubsystem(TheDeepCRCSanityCheck, "TheDeepCRCSanityCheck", MSGNEW("GameEngineSubystem") DeepCRCSanityCheck, NULL);
#endif // DEBUG_CRC
		initSubsystem(TheGameText, "TheGameText", CreateGameTextInterface(), NULL);
		updateWindowTitle();
		initSubsystem(TheScienceStore,"TheScienceStore", MSGNEW("GameEngineSubsystem") ScienceStore(), &xferCRC, "Data\\INI\\Default\\Science", "Data\\INI\\Science");
		initSubsystem(TheMultiplayerSettings,"TheMultiplayerSettings", MSGNEW("GameEngineSubsystem") MultiplayerSettings(), &xferCRC, "Data\\INI\\Default\\Multiplayer", "Data\\INI\\Multiplayer");
		initSubsystem(TheTerrainTypes,"TheTerrainTypes", MSGNEW("GameEngineSubsystem") TerrainTypeCollection(), &xferCRC, "Data\\INI\\Default\\Terrain", "Data\\INI\\Terrain");
		initSubsystem(TheTerrainRoads,"TheTerrainRoads", MSGNEW("GameEngineSubsystem") TerrainRoadCollection(), &xferCRC, "Data\\INI\\Default\\Roads", "Data\\INI\\Roads");
		initSubsystem(TheGlobalLanguageData,"TheGlobalLanguageData",MSGNEW("GameEngineSubsystem") GlobalLanguage, NULL); // must be before the game text
		initSubsystem(TheCDManager,"TheCDManager", CreateCDManager(), NULL);
		initSubsystem(TheAudio,"TheAudio", TheGlobalData->m_headless ? NEW AudioManagerDummy : createAudioManager(), NULL);
		if (!TheAudio->isMusicAlreadyLoaded())
			setQuitting(TRUE);
		initSubsystem(TheFunctionLexicon,"TheFunctionLexicon", createFunctionLexicon(), NULL);
		initSubsystem(TheModuleFactory,"TheModuleFactory", createModuleFactory(), NULL);
		initSubsystem(TheMessageStream,"TheMessageStream", createMessageStream(), NULL);
		initSubsystem(TheSidesList,"TheSidesList", MSGNEW("GameEngineSubsystem") SidesList(), NULL);
		initSubsystem(TheCaveSystem,"TheCaveSystem", MSGNEW("GameEngineSubsystem") CaveSystem(), NULL);
		initSubsystem(TheRankInfoStore,"TheRankInfoStore", MSGNEW("GameEngineSubsystem") RankInfoStore(), &xferCRC, NULL, "Data\\INI\\Rank");
		initSubsystem(ThePlayerTemplateStore,"ThePlayerTemplateStore", MSGNEW("GameEngineSubsystem") PlayerTemplateStore(), &xferCRC, "Data\\INI\\Default\\PlayerTemplate", "Data\\INI\\PlayerTemplate");
		initSubsystem(TheParticleSystemManager,"TheParticleSystemManager", createParticleSystemManager(), NULL);
		initSubsystem(TheFXListStore,"TheFXListStore", MSGNEW("GameEngineSubsystem") FXListStore(), &xferCRC, "Data\\INI\\Default\\FXList", "Data\\INI\\FXList");
		initSubsystem(TheWeaponStore,"TheWeaponStore", MSGNEW("GameEngineSubsystem") WeaponStore(), &xferCRC, NULL, "Data\\INI\\Weapon");
		initSubsystem(TheObjectCreationListStore,"TheObjectCreationListStore", MSGNEW("GameEngineSubsystem") ObjectCreationListStore(), &xferCRC, "Data\\INI\\Default\\ObjectCreationList", "Data\\INI\\ObjectCreationList");
		initSubsystem(TheLocomotorStore,"TheLocomotorStore", MSGNEW("GameEngineSubsystem") LocomotorStore(), &xferCRC, NULL, "Data\\INI\\Locomotor");
		initSubsystem(TheSpecialPowerStore,"TheSpecialPowerStore", MSGNEW("GameEngineSubsystem") SpecialPowerStore(), &xferCRC, "Data\\INI\\Default\\SpecialPower", "Data\\INI\\SpecialPower");
		initSubsystem(TheDamageFXStore,"TheDamageFXStore", MSGNEW("GameEngineSubsystem") DamageFXStore(), &xferCRC, NULL, "Data\\INI\\DamageFX");
		initSubsystem(TheArmorStore,"TheArmorStore", MSGNEW("GameEngineSubsystem") ArmorStore(), &xferCRC, NULL, "Data\\INI\\Armor");
		initSubsystem(TheBuildAssistant,"TheBuildAssistant", MSGNEW("GameEngineSubsystem") BuildAssistant, NULL);
		initSubsystem(TheThingFactory,"TheThingFactory", createThingFactory(), &xferCRC, "Data\\INI\\Default\\Object", "Data\\INI\\Object");
		initSubsystem(TheUpgradeCenter,"TheUpgradeCenter", MSGNEW("GameEngineSubsystem") UpgradeCenter, &xferCRC, "Data\\INI\\Default\\Upgrade", "Data\\INI\\Upgrade");
		initSubsystem(TheGameClient,"TheGameClient", createGameClient(), NULL);
		initSubsystem(TheAI,"TheAI", MSGNEW("GameEngineSubsystem") AI(), &xferCRC,  "Data\\INI\\Default\\AIData", "Data\\INI\\AIData");
		initSubsystem(TheGameLogic,"TheGameLogic", createGameLogic(), NULL);
		initSubsystem(TheTeamFactory,"TheTeamFactory", MSGNEW("GameEngineSubsystem") TeamFactory(), NULL);
		initSubsystem(TheCrateSystem,"TheCrateSystem", MSGNEW("GameEngineSubsystem") CrateSystem(), &xferCRC, "Data\\INI\\Default\\Crate", "Data\\INI\\Crate");
		initSubsystem(ThePlayerList,"ThePlayerList", MSGNEW("GameEngineSubsystem") PlayerList(), NULL);
		initSubsystem(TheRecorder,"TheRecorder", createRecorder(), NULL);
		initSubsystem(TheRadar,"TheRadar", TheGlobalData->m_headless ? NEW RadarDummy : createRadar(), NULL);
		initSubsystem(TheVictoryConditions,"TheVictoryConditions", createVictoryConditions(), NULL);

		AsciiString fname;
		fname.format("Data\\%s\\CommandMap", GetRegistryLanguage().str());
		initSubsystem(TheMetaMap,"TheMetaMap", MSGNEW("GameEngineSubsystem") MetaMap(), NULL, fname.str(), "Data\\INI\\CommandMap");

		TheMetaMap->generateMetaMap();

#if defined(RTS_DEBUG)
		ini.loadFileDirectory("Data\\INI\\CommandMapDebug", INI_LOAD_MULTIFILE, NULL);
#endif

		initSubsystem(TheActionManager,"TheActionManager", MSGNEW("GameEngineSubsystem") ActionManager(), NULL);
		//initSubsystem((CComObject<WebBrowser> *)TheWebBrowser,"(CComObject<WebBrowser> *)TheWebBrowser", (CComObject<WebBrowser> *)createWebBrowser(), NULL);
		initSubsystem(TheGameStateMap,"TheGameStateMap", MSGNEW("GameEngineSubsystem") GameStateMap, NULL );
		initSubsystem(TheGameState,"TheGameState", MSGNEW("GameEngineSubsystem") GameState, NULL );

		// Create the interface for sending game results
		initSubsystem(TheGameResultsQueue,"TheGameResultsQueue", GameResultsInterface::createNewGameResultsInterface(), NULL);

		xferCRC.close();
		TheWritableGlobalData->m_iniCRC = xferCRC.getCRC();
		DEBUG_LOG(("INI CRC is 0x%8.8X", TheGlobalData->m_iniCRC));

		TheSubsystemList->postProcessLoadAll();

		setFramesPerSecondLimit(TheGlobalData->m_framesPerSecondLimit);

		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_musicOn, AudioAffect_Music);
		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_soundsOn, AudioAffect_Sound);
		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_sounds3DOn, AudioAffect_Sound3D);
		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_speechOn, AudioAffect_Speech);

		// We're not in a network game yet, so set the network singleton to NULL.
		TheNetwork = NULL;

		//Create a default ini file for options if it doesn't already exist.
		//OptionPreferences prefs( TRUE );

		// If we turn m_quitting to FALSE here, then we throw away any requests to quit that
		// took place during loading. :-\ - jkmcd
		// If this really needs to take place, please make sure that pressing cancel on the audio
		// load music dialog will still cause the game to quit.
		// m_quitting = FALSE;

		// initialize the MapCache
		TheMapCache = MSGNEW("GameEngineSubsystem") MapCache;
		TheMapCache->updateCache();

		if (TheGlobalData->m_buildMapCache)
		{
			// just quit, since the map cache has already updated
			//populateMapListbox(NULL, true, true);
			m_quitting = TRUE;
		}

		// load the initial shell screen
		//TheShell->push( AsciiString("Menus/MainMenu.wnd") );

		// This allows us to run a map from the command line
		if (TheGlobalData->m_initialFile.isEmpty() == FALSE)
		{
			AsciiString fname = TheGlobalData->m_initialFile;
			fname.toLower();

			if (fname.endsWithNoCase(".map"))
			{
				TheWritableGlobalData->m_shellMapOn = FALSE;
				TheWritableGlobalData->m_playIntro = FALSE;
				TheWritableGlobalData->m_pendingFile = TheGlobalData->m_initialFile;

				// shutdown the top, but do not pop it off the stack
	//			TheShell->hideShell();

				// send a message to the logic for a new game
				GameMessage *msg = TheMessageStream->appendMessage( GameMessage::MSG_NEW_GAME );
				msg->appendIntegerArgument(GAME_SINGLE_PLAYER);
				msg->appendIntegerArgument(DIFFICULTY_NORMAL);
				msg->appendIntegerArgument(0);
				InitRandom(0);
			}
		}

		//
		if (TheMapCache && TheGlobalData->m_shellMapOn)
		{
			AsciiString lowerName = TheGlobalData->m_shellMapName;
			lowerName.toLower();

			MapCache::const_iterator it = TheMapCache->find(lowerName);
			if (it == TheMapCache->end())
			{
				TheWritableGlobalData->m_shellMapOn = FALSE;
			}
		}

		if(!TheGlobalData->m_playIntro)
			TheWritableGlobalData->m_afterIntro = TRUE;

		initDisabledMasks();

	}
	catch (ErrorCode ec)
	{
		if (ec == ERROR_INVALID_D3D)
		{
			RELEASE_CRASHLOCALIZED("ERROR:D3DFailurePrompt", "ERROR:D3DFailureMessage");
		}
	}
	catch (INIException e)
	{
		if (e.mFailureMessage)
			RELEASE_CRASH((e.mFailureMessage));
		else
			RELEASE_CRASH(("Uncaught Exception during initialization."));

	}
	catch (...)
	{
		RELEASE_CRASH(("Uncaught Exception during initialization."));
	}

	if(!TheGlobalData->m_playIntro)
		TheWritableGlobalData->m_afterIntro = TRUE;

	initDisabledMasks();

	resetSubsystems();

	HideControlBar();
}

/** -----------------------------------------------------------------------------------------------
	* Reset all necessary parts of the game engine to be ready to accept new game data
	*/
void GameEngine::reset( void )
{

	WindowLayout *background = TheWindowManager->winCreateLayout("Menus/BlankWindow.wnd");
	DEBUG_ASSERTCRASH(background,("We Couldn't Load Menus/BlankWindow.wnd"));
	background->hide(FALSE);
	background->bringForward();
	background->getFirstWindow()->winClearStatus(WIN_STATUS_IMAGE);
	Bool deleteNetwork = false;
	if (TheGameLogic->isInMultiplayerGame())
		deleteNetwork = true;

	resetSubsystems();

	if (deleteNetwork)
	{
		DEBUG_ASSERTCRASH(TheNetwork, ("Deleting NULL TheNetwork!"));
		delete TheNetwork;
		TheNetwork = NULL;
	}
	if(background)
	{
		background->destroyWindows();
		deleteInstance(background);
		background = NULL;
	}
}

/// -----------------------------------------------------------------------------------------------
void GameEngine::resetSubsystems( void )
{
	// TheSuperHackers @fix xezon 09/06/2025 Reset GameLogic first to purge all world objects early.
	// This avoids potentially catastrophic issues when objects and subsystems have cross dependencies.
	TheGameLogic->reset();

	TheSubsystemList->resetAll();
}

/// -----------------------------------------------------------------------------------------------
Bool GameEngine::canUpdateGameLogic()
{
	// Must be first.
	TheGameLogic->preUpdate();

	m_isTimeFrozen = isTimeFrozen();
	m_isGameHalted = isGameHalted();

	if (TheNetwork != NULL)
	{
		return canUpdateNetworkGameLogic();
	}
	else
	{
		return canUpdateRegularGameLogic();
	}
}

Bool GameEngine::canUpdateNetworkGameLogic()
{
	DEBUG_ASSERTCRASH(TheNetwork != NULL, ("TheNetwork is NULL"));

	if (TheNetwork->isFrameDataReady())
	{
		// Important: The Network is definitely no longer stalling.
		m_isGameHalted = false;

		return true;
	}

	return false;
}

Bool GameEngine::canUpdateRegularGameLogic()
{
	const Bool enabled = isLogicTimeScaleEnabled();
	const Int logicTimeScaleFps = getLogicTimeScaleFps();
	const Int maxRenderFps = getFramesPerSecondLimit();

#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
	const Bool useFastMode = TheGlobalData->m_TiVOFastMode;
#else	//always allow this cheat key if we're in a replay game.
	const Bool useFastMode = TheGlobalData->m_TiVOFastMode && TheGameLogic->isInReplayGame();
#endif

	if (useFastMode || !enabled || logicTimeScaleFps >= maxRenderFps)
	{
		// Logic time scale is uncapped or larger equal Render FPS. Update straight away.
		return true;
	}
	else
	{
		// TheSuperHackers @tweak xezon 06/08/2025
		// The logic time step is now decoupled from the render update.
		const Real targetFrameTime = 1.0f / logicTimeScaleFps;
		m_logicTimeAccumulator += min(m_updateTime, targetFrameTime);

		if (m_logicTimeAccumulator >= targetFrameTime)
		{
			m_logicTimeAccumulator -= targetFrameTime;
			return true;
		}
	}

	return false;
}

/// -----------------------------------------------------------------------------------------------
DECLARE_PERF_TIMER(GameEngine_update)

/** -----------------------------------------------------------------------------------------------
 * Update the game engine by updating the GameClient and GameLogic singletons.
 */
void GameEngine::update( void )
{
	USE_PERF_TIMER(GameEngine_update)
	{
		{
			// VERIFY CRC needs to be in this code block.  Please to not pull TheGameLogic->update() inside this block.
			VERIFY_CRC

			TheRadar->UPDATE();

			/// @todo Move audio init, update, etc, into GameClient update

			TheAudio->UPDATE();
			TheGameClient->UPDATE();
			TheMessageStream->propagateMessages();

			if (TheNetwork != NULL)
			{
				TheNetwork->UPDATE();
			}

			TheCDManager->UPDATE();
		}

		const Bool canUpdate = canUpdateGameLogic();
		const Bool canUpdateLogic = canUpdate && !m_isGameHalted && !m_isTimeFrozen;
		const Bool canUpdateScript = canUpdate && !m_isGameHalted;

		if (canUpdateLogic)
		{
			TheGameClient->step();
			TheGameLogic->UPDATE();
		}
		else if (canUpdateScript)
		{
			// TheSuperHackers @info Still update the Script Engine to allow
			// for scripted camera movements while the time is frozen.
			TheScriptEngine->UPDATE();
		}
	}
}

// Horrible reference, but we really, really need to know if we are windowed.
extern bool DX8Wrapper_IsWindowed;
extern HWND ApplicationHWnd;

/** -----------------------------------------------------------------------------------------------
 * The "main loop" of the game engine. It will not return until the game exits.
 */
void GameEngine::execute( void )
{
	FrameRateLimit* frameRateLimit = new FrameRateLimit();

#if defined(RTS_DEBUG)
	DWORD startTime = timeGetTime() / 1000;
#endif

	// pretty basic for now
	while( !m_quitting )
	{

		//if (TheGlobalData->m_vTune)
		{
#ifdef PERF_TIMERS
			PerfGather::resetAll();
#endif
		}

		{

#if defined(RTS_DEBUG)
			{
				// enter only if in benchmark mode
				if (TheGlobalData->m_benchmarkTimer > 0)
				{
					DWORD currentTime = timeGetTime() / 1000;
					if (TheGlobalData->m_benchmarkTimer < currentTime - startTime)
					{
						if (TheGameLogic->isInGame())
						{
							if (TheRecorder->getMode() == RECORDERMODETYPE_RECORD)
							{
								TheRecorder->stopRecording();
							}
							TheGameLogic->clearGameData();
						}
						TheGameEngine->setQuitting(TRUE);
					}
				}
			}
#endif

			{
				try
				{
					// compute a frame
					update();
				}
				catch (INIException e)
				{
					// Release CRASH doesn't return, so don't worry about executing additional code.
					if (e.mFailureMessage)
						RELEASE_CRASH((e.mFailureMessage));
					else
						RELEASE_CRASH(("Uncaught Exception in GameEngine::update"));
				}
				catch (...)
				{
					// try to save info off
					try
					{
						if (TheRecorder && TheRecorder->getMode() == RECORDERMODETYPE_RECORD && TheRecorder->isMultiplayer())
							TheRecorder->cleanUpReplayFile();
					}
					catch (...)
					{
					}
					RELEASE_CRASH(("Uncaught Exception in GameEngine::update"));
				}
			}

			{
				{
					Bool allowFpsLimit = TheTacticalView->getTimeMultiplier()<=1 && !TheScriptEngine->isTimeFast();

		// I'm disabling this in debug because many people need alt-tab capability.  If you happen to be
		// doing performance tuning, please just change this on your local system. -MDC
		#if defined(RTS_DEBUG)
					if (allowFpsLimit)
						::Sleep(1); // give everyone else a tiny time slice.
		#endif


		#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
					allowFpsLimit &= !(!TheGameLogic->isGamePaused() && TheGlobalData->m_TiVOFastMode);
		#else	//always allow this cheat key if we're in a replay game.
					allowFpsLimit &= !(!TheGameLogic->isGamePaused() && TheGlobalData->m_TiVOFastMode && TheGameLogic->isInReplayGame());
		#endif
					{
						// TheSuperHackers @bugfix xezon 05/08/2025 Re-implements the frame rate limiter
						// with higher resolution counters to cap the frame rate more accurately to the desired limit.
						allowFpsLimit &= TheGlobalData->m_useFpsLimit;
						const UnsignedInt maxFps = allowFpsLimit ? getFramesPerSecondLimit() : RenderFpsPreset::UncappedFpsValue;
						m_updateTime = frameRateLimit->wait(maxFps);
					}

				}
			}

		}

#ifdef PERF_TIMERS
		if (!m_quitting && TheGameLogic->isInGame() && !TheGameLogic->isInShellGame() && !TheGameLogic->isGamePaused())
		{
			PerfGather::dumpAll(TheGameLogic->getFrame());
			PerfGather::displayGraph(TheGameLogic->getFrame());
			PerfGather::resetAll();
		}
#endif

	}

	delete frameRateLimit;
}

/** -----------------------------------------------------------------------------------------------
	* Factory for the message stream
	*/
MessageStream *GameEngine::createMessageStream( void )
{
	// if you change this update the tools that use the engine systems
	// like GUIEdit, it creates a message stream to run in "test" mode
	return MSGNEW("GameEngineSubsystem") MessageStream;
}

//-------------------------------------------------------------------------------------------------
FileSystem *GameEngine::createFileSystem( void )
{
	return MSGNEW("GameEngineSubsystem") FileSystem;
}

//-------------------------------------------------------------------------------------------------
Bool GameEngine::isMultiplayerSession( void )
{
	return TheRecorder->isMultiplayer();
}

/**	MW - 6-10-03: I added this function in order to verify that users who quit the
application by not using the menus (Alt-F4, etc.) are not doing it in the
middle of an internet game.  Quitting in this way is considered cheating so
we will log the results to a file and transmit the updated stats on the next
login to gamespy.  I copied most of this code from the normal disconnection
logging code that is usually called from another thread.  I'm doing it here
because there's no way to guarantee the other thread will execute before we
exit the app.
*/
void GameEngine::checkAbnormalQuitting(void)
{
	if (TheRecorder->isMultiplayer() && TheGameLogic->isInInternetGame())
	{	//Should not be quitting at this time, record it as a cheat.

		Int localID = TheGameSpyInfo->getLocalProfileID();
		PSPlayerStats stats = TheGameSpyPSMessageQueue->findPlayerStatsByID(localID);

		Player *player=ThePlayerList->getLocalPlayer();

		Int ptIdx;
		const PlayerTemplate *myTemplate = player->getPlayerTemplate();
		DEBUG_LOG(("myTemplate = %X(%s)", myTemplate, myTemplate->getName().str()));
		for (ptIdx = 0; ptIdx < ThePlayerTemplateStore->getPlayerTemplateCount(); ++ptIdx)
		{
			const PlayerTemplate *nthTemplate = ThePlayerTemplateStore->getNthPlayerTemplate(ptIdx);
			DEBUG_LOG(("nthTemplate = %X(%s)", nthTemplate, nthTemplate->getName().str()));
			if (nthTemplate == myTemplate)
			{
					break;
			}
		}

		PSRequest req;

		req.requestType = PSRequest::PSREQUEST_UPDATEPLAYERSTATS;
		req.email = TheGameSpyInfo->getLocalEmail().str();
		req.nick = TheGameSpyInfo->getLocalBaseName().str();
		req.password = TheGameSpyInfo->getLocalPassword().str();
		req.player = stats;
		req.addDesync = FALSE;
		req.addDiscon = TRUE;
		req.lastHouse = ptIdx;

		UserPreferences pref;
		AsciiString userPrefFilename;
		userPrefFilename.format("GeneralsOnline\\MiscPref%d.ini", stats.id);
		DEBUG_LOG(("using the file %s", userPrefFilename.str()));
		pref.load(userPrefFilename);

		Int addedInDesyncs2 = pref.getInt("0", 0);
		DEBUG_LOG(("addedInDesyncs2 = %d", addedInDesyncs2));
		if (addedInDesyncs2 < 0)
			addedInDesyncs2 = 10;
		Int addedInDesyncs3 = pref.getInt("1", 0);
		DEBUG_LOG(("addedInDesyncs3 = %d", addedInDesyncs3));
		if (addedInDesyncs3 < 0)
			addedInDesyncs3 = 10;
		Int addedInDesyncs4 = pref.getInt("2", 0);
		DEBUG_LOG(("addedInDesyncs4 = %d", addedInDesyncs4));
		if (addedInDesyncs4 < 0)
			addedInDesyncs4 = 10;
		Int addedInDiscons2 = pref.getInt("3", 0);
		DEBUG_LOG(("addedInDiscons2 = %d", addedInDiscons2));
		if (addedInDiscons2 < 0)
			addedInDiscons2 = 10;
		Int addedInDiscons3 = pref.getInt("4", 0);
		DEBUG_LOG(("addedInDiscons3 = %d", addedInDiscons3));
		if (addedInDiscons3 < 0)
			addedInDiscons3 = 10;
		Int addedInDiscons4 = pref.getInt("5", 0);
		DEBUG_LOG(("addedInDiscons4 = %d", addedInDiscons4));
		if (addedInDiscons4 < 0)
			addedInDiscons4 = 10;

		DEBUG_LOG(("req.addDesync=%d, req.addDiscon=%d, addedInDesync=%d,%d,%d, addedInDiscon=%d,%d,%d",
			req.addDesync, req.addDiscon, addedInDesyncs2, addedInDesyncs3, addedInDesyncs4,
			addedInDiscons2, addedInDiscons3, addedInDiscons4));

		if (req.addDesync || req.addDiscon)
		{
			AsciiString val;
			if (req.lastHouse == 2)
			{
				val.format("%d", addedInDesyncs2 + req.addDesync);
				pref["0"] = val;
				val.format("%d", addedInDiscons2 + req.addDiscon);
				pref["3"] = val;
				DEBUG_LOG(("house 2 req.addDesync || req.addDiscon: %d %d",
					addedInDesyncs2 + req.addDesync, addedInDiscons2 + req.addDiscon));
			}
			else if (req.lastHouse == 3)
			{
				val.format("%d", addedInDesyncs3 + req.addDesync);
				pref["1"] = val;
				val.format("%d", addedInDiscons3 + req.addDiscon);
				pref["4"] = val;
				DEBUG_LOG(("house 3 req.addDesync || req.addDiscon: %d %d",
					addedInDesyncs3 + req.addDesync, addedInDiscons3 + req.addDiscon));
			}
			else
			{
				val.format("%d", addedInDesyncs4 + req.addDesync);
				pref["2"] = val;
				val.format("%d", addedInDiscons4 + req.addDiscon);
				pref["5"] = val;
				DEBUG_LOG(("house 4 req.addDesync || req.addDiscon: %d %d",
					addedInDesyncs4 + req.addDesync, addedInDiscons4 + req.addDiscon));
			}
			pref.write();
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#define CONVERT_EXEC1	"..\\Build\\nvdxt -list buildDDS.txt -dxt5 -full -outdir Art\\Textures > buildDDS.out"

void updateTGAtoDDS()
{
	// Here's the scoop. We're going to traverse through all of the files in the Art\Textures folder
	// and determine if there are any .tga files that are newer than associated .dds files. If there
	// are, then we will re-run the compression tool on them.

	File *fp = TheLocalFileSystem->openFile("buildDDS.txt", File::WRITE | File::CREATE | File::TRUNCATE | File::TEXT);
	if (!fp) {
		return;
	}

	FilenameList files;
	TheLocalFileSystem->getFileListInDirectory("Art\\Textures\\", "", "*.tga", files, TRUE);
	FilenameList::iterator it;
	for (it = files.begin(); it != files.end(); ++it) {
		AsciiString filenameTGA = *it;
		AsciiString filenameDDS = *it;
		FileInfo infoTGA;
		TheLocalFileSystem->getFileInfo(filenameTGA, &infoTGA);

		// skip the water textures, since they need to be NOT compressed
		filenameTGA.toLower();
		if (strstr(filenameTGA.str(), "caust"))
		{
			continue;
		}
		// and the recolored stuff.
		if (strstr(filenameTGA.str(), "zhca"))
		{
			continue;
		}

		// replace tga with dds
		filenameDDS.truncateBy(3);	// "tga"
		filenameDDS.concat("dds");

		Bool needsToBeUpdated = FALSE;
		FileInfo infoDDS;
		if (TheFileSystem->doesFileExist(filenameDDS.str())) {
			TheFileSystem->getFileInfo(filenameDDS, &infoDDS);
			if (infoTGA.timestampHigh > infoDDS.timestampHigh ||
					(infoTGA.timestampHigh == infoDDS.timestampHigh &&
					 infoTGA.timestampLow > infoDDS.timestampLow)) {
				needsToBeUpdated = TRUE;
			}
		} else {
			needsToBeUpdated = TRUE;
		}

		if (!needsToBeUpdated) {
			continue;
		}

		filenameTGA.concat("\n");
		fp->write(filenameTGA.str(), filenameTGA.getLength());
	}

	fp->close();

	system(CONVERT_EXEC1);
}

//-------------------------------------------------------------------------------------------------
// System things

// If we're using the Wide character version of MessageBox, then there's no additional
// processing necessary. Please note that this is a sleazy way to get this information,
// but pending a better one, this'll have to do.
extern const Bool TheSystemIsUnicode = (((void*) (::MessageBox)) == ((void*) (::MessageBoxW)));
