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

// FILE: SDL3Main.cpp /////////////////////////////////////////////////////////
//
// Entry point for game application
//
// Author: Colin Day, April 2001
//
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <crtdbg.h>
#include <eh.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "Common/CommandLine.h"
#include "Common/CriticalSection.h"
#include "Common/GlobalData.h"
#include "Common/GameEngine.h"
#include "Common/GameSounds.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"
#include "Common/MessageStream.h"
#include "Common/PlayerList.h"
#include "Common/Registry.h"
#include "Common/Team.h"
#include "GameClient/ClientInstance.h"
#include "GameClient/ControlBar.h"
#include "GameClient/InGameUI.h"
#include "GameClient/GameClient.h"
#include "GameClient/HeaderTemplate.h"
#include "GameLogic/GameLogic.h"  ///< @todo for demo, remove
#include "GameClient/Mouse.h"
#include "GameClient/IMEManager.h"
#include "SDL3Device/Common/SDL3GameEngine.h"
#include "../../../Generals/Code/Main/Platform/SDLPlatformWindow.h"
#include "Common/version.h"
#include "BuildVersion.h"
#include "GeneratedVersion.h"
#include "resource.h"

#ifdef RTS_ENABLE_CRASHDUMP
#include "Common/MiniDumper.h"
#endif


// GLOBALS ////////////////////////////////////////////////////////////////////

const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";
const char *gAppPrefix = ""; /// So WB can have a different debug log file name.

static Bool gInitializing = false;
static Bool gDoPaint = true;
static Bool isSDL3Active = false;

static SDLPlatformWindow gPlatformWindow;

class GeneralsMDSDL3GameEngine final : public SDL3GameEngine
{
protected:
	void onDisplaySizeChanged(UnsignedInt oldWidth, UnsignedInt oldHeight,
		UnsignedInt newWidth, UnsignedInt newHeight) override
	{
		if (TheHeaderTemplateManager != nullptr)
			TheHeaderTemplateManager->onResolutionChanged();
		// Keep the active match and all existing window objects alive. The layout
		// manager scales the current tree in place, which is the same coordinate
		// result that reparsing .wnd files would produce.
		if (TheWindowManager != nullptr)
			TheWindowManager->winScaleToResolution(oldWidth, oldHeight, newWidth, newHeight);
		if (TheControlBar != nullptr)
			TheControlBar->onDisplaySizeChanged(oldWidth, oldHeight, newWidth, newHeight);

		// These refresh only resolution-dependent resources; they do not recreate
		// the shell stack or the in-game control bar.
		if (TheMouse != nullptr)
		{
			TheMouse->setMouseLimits();
			TheMouse->onResolutionChanged();
		}
		if (TheInGameUI != nullptr && TheWindowManager != nullptr)
			TheInGameUI->refreshCustomUiResources();
	}
};



// initializeAppWindows =======================================================
/** Create the SDL3 application window. */
//=============================================================================
static Bool initializeAppWindows( Bool runWindowed )
{
	const Int startWidth = DEFAULT_DISPLAY_WIDTH;
	const Int startHeight = DEFAULT_DISPLAY_HEIGHT;
	if (!gPlatformWindow.initialize(startWidth, startHeight, !runWindowed))
		return false;
	gPlatformWindow.show();
	isSDL3Active = true;
	gInitializing = false;
	if (!runWindowed)
		gDoPaint = false;
	return true;


}

// Necessary to allow memory managers and such to have useful critical sections
static CriticalSection critSec1, critSec2, critSec3, critSec4, critSec5;

// SDL3 main ==================================================================
/** Application entry point */
//=============================================================================
// SDL3 main ==================================================================
/** Application entry point */
//=============================================================================
int main( int argc, char **argv )
{
	(void)argc;
	(void)argv;
	Int exitcode = 1;

#ifdef RTS_PROFILE_LEGACY
  Profile::StartRange("init");
#endif

	try {

		SDLPlatformWindow::installCrashHandler();
		//
		// there is something about checkin in and out the .dsp and .dsw files
		// that blows the working directory information away on each of the
		// developers machines so we're going to hack it for a while and set our
		// working directory to the directory with the .exe since that's not the
		// default in a DevStudio project
		//

		TheAsciiStringCriticalSection = &critSec1;
		TheUnicodeStringCriticalSection = &critSec2;
		TheDmaCriticalSection = &critSec3;
		TheMemoryPoolCriticalSection = &critSec4;
		TheDebugLogCriticalSection = &critSec5;

		// initialize the memory manager early
		initMemoryManager();

		SDLPlatformWindow::setWorkingDirectoryToExecutable();


		#ifdef RTS_DEBUG
			// Turn on Memory heap tracking
			int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
			tmpFlag |= (_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
			tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;
			_CrtSetDbgFlag( tmpFlag );
		#endif



		// install debug callbacks
	//	WWDebug_Install_Message_Handler(WWDebug_Message_Callback);
	//	WWDebug_Install_Assert_Handler(WWAssert_Callback);


		CommandLine::parseCommandLineForStartup();
#ifdef RTS_ENABLE_CRASHDUMP
		// Initialize minidump facilities - requires TheGlobalData so performed after parseCommandLineForStartup
		MiniDumper::initMiniDumper(TheGlobalData->getPath_UserData());
#endif

		// register windows class and create application window
		if(!TheGlobalData->m_headless && initializeAppWindows(TheGlobalData->m_windowed) == false)
		{
			return exitcode;
		}

		// save our application instance for future use

		// BGC - initialize COM
	//	OleInitialize(nullptr);



		// Set up version info
		TheVersion = NEW Version;
		TheVersion->setVersion(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNUM, VERSION_LOCALBUILDNUM,
			AsciiString(VERSION_BUILDUSER), AsciiString(VERSION_BUILDLOC),
			AsciiString(__TIME__), AsciiString(__DATE__));

		// TheSuperHackers @refactor The instance mutex now lives in its own class.

		if (!rts::ClientInstance::initialize())
		{
			SDLPlatformWindow::restoreExistingInstance(rts::ClientInstance::getFirstInstanceName());

			DEBUG_LOG(("Generals is already running...Bail!"));
			delete TheVersion;
			TheVersion = nullptr;
			shutdownMemoryManager();
			return exitcode;
		}
		DEBUG_LOG(("Create Generals Mutex okay."));

		DEBUG_LOG(("CRC message is %d", GameMessage::MSG_LOGIC_CRC));

		// run the game main loop
		exitcode = GameMain();

		delete TheVersion;
		TheVersion = nullptr;

	#ifdef MEMORYPOOL_DEBUG
		TheMemoryPoolFactory->debugMemoryReport(REPORT_POOLINFO | REPORT_POOL_OVERFLOW | REPORT_SIMPLE_LEAKS, 0, 0);
	#endif
	#if defined(RTS_DEBUG)
		TheMemoryPoolFactory->memoryPoolUsageReport("AAAMemStats");
	#endif

		shutdownMemoryManager();

		// BGC - shut down COM
	//	OleUninitialize();
	}
	catch (...)
	{

	}

#ifdef RTS_ENABLE_CRASHDUMP
	MiniDumper::shutdownMiniDumper();
#endif
	TheUnicodeStringCriticalSection = nullptr;
	TheDmaCriticalSection = nullptr;
	TheMemoryPoolCriticalSection = nullptr;

	return exitcode;

}

// CreateGameEngine ===========================================================
/** Create the SDL3 game engine we're going to use */
//=============================================================================
GameEngine *CreateGameEngine()
{
	SDL3GameEngine *engine = NEW GeneralsMDSDL3GameEngine;
	//game engine may not have existed when app got focus so make sure it
	//knows about current focus state.
	engine->setIsActive(isSDL3Active);

	return engine;

}
