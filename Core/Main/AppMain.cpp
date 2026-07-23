/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#include "AppMain.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdlib>
#include <cstdio>
#include <crtdbg.h>

#include "Lib/BaseType.h"
#include "Common/CommandLine.h"
#include "Common/CriticalSection.h"
#include "Common/Debug.h"
#include "Common/GameEngine.h"
#include "Common/GameMemory.h"
#include "Common/GlobalData.h"
#include "Common/StackDump.h"
#include "Common/version.h"
#include "BuildVersion.h"
#include "GeneratedVersion.h"
#include "GameClient/ClientInstance.h"

#include "Common/Registry.h"

#ifdef RTS_ENABLE_CRASHDUMP
#include "Common/MiniDumper.h"
#endif

// Global game data paths & prefixes required by GameText
const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";
const char *gAppPrefix = "";

class Win32Mouse;
Win32Mouse *TheWin32Mouse = nullptr;

// Critical sections for memory manager and string systems
static CriticalSection critSec1, critSec2, critSec3, critSec4, critSec5;

namespace AppMain
{

Bool initBeforeWindow()
{
	TheAsciiStringCriticalSection = &critSec1;
	TheUnicodeStringCriticalSection = &critSec2;
	TheDmaCriticalSection = &critSec3;
	TheMemoryPoolCriticalSection = &critSec4;
	TheDebugLogCriticalSection = &critSec5;

	// Initialize memory manager early
	initMemoryManager();

	// Set working directory to executable folder
	Char buffer[_MAX_PATH];
	GetModuleFileName(nullptr, buffer, sizeof(buffer));
	if (Char* pEnd = strrchr(buffer, '\\'))
	{
		*pEnd = 0;
	}
	::SetCurrentDirectory(buffer);

#ifdef RTS_DEBUG
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpFlag |= (_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
	tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;
	_CrtSetDbgFlag(tmpFlag);
#endif

	CommandLine::parseCommandLineForStartup();

#ifdef RTS_ENABLE_CRASHDUMP
	MiniDumper::initMiniDumper(TheGlobalData->getPath_UserData());
#endif

	return true;
}

Bool initAfterWindow()
{
	// Setup version info
	TheVersion = NEW Version;
	TheVersion->setVersion(
		VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNUM, VERSION_LOCALBUILDNUM,
		AsciiString(VERSION_BUILDUSER), AsciiString(VERSION_BUILDLOC),
		AsciiString(__TIME__), AsciiString(__DATE__)
	);

	// Single instance mutex check
	if (!rts::ClientInstance::initialize())
	{
		HWND ccwindow = FindWindow(rts::ClientInstance::getFirstInstanceName(), nullptr);
		if (ccwindow)
		{
			SetForegroundWindow(ccwindow);
			ShowWindow(ccwindow, SW_RESTORE);
		}

		DEBUG_LOG(("Generals is already running...Bail!"));
		delete TheVersion;
		TheVersion = nullptr;
		shutdownMemoryManager();
		return false;
	}
	DEBUG_LOG(("Create Generals Mutex okay."));

	return true;
}

static Bool s_isAppActive = false;

void setAppActive(Bool active)
{
	s_isAppActive = active;
	if (TheGameEngine)
	{
		TheGameEngine->setIsActive(active);
	}
}

Bool isAppActive()
{
	return s_isAppActive;
}

void getInitialWindowBounds(Int& outWidth, Int& outHeight)
{
	outWidth = (TheGlobalData && TheGlobalData->m_xResolution > 0) ? TheGlobalData->m_xResolution : 800;
	outHeight = (TheGlobalData && TheGlobalData->m_yResolution > 0) ? TheGlobalData->m_yResolution : 600;
}

Int run()
{
	return GameMain();
}

void getSplashFilePath(Char* outBuffer, UnsignedInt bufferSize)
{
	if (!outBuffer || bufferSize == 0)
		return;

	const char* fileName = "Install_Final.bmp";
	snprintf(outBuffer, bufferSize, "Data/%s/%s", GetRegistryLanguage().str(), fileName);
	FILE* fileImage = fopen(outBuffer, "rb");
	if (fileImage)
	{
		fclose(fileImage);
	}
	else
	{
		snprintf(outBuffer, bufferSize, "%s", fileName);
	}
}

void shutdown()
{
	delete TheVersion;
	TheVersion = nullptr;

#ifdef MEMORYPOOL_DEBUG
	TheMemoryPoolFactory->debugMemoryReport(REPORT_POOLINFO | REPORT_POOL_OVERFLOW | REPORT_SIMPLE_LEAKS, 0, 0);
#endif
#if defined(RTS_DEBUG)
	TheMemoryPoolFactory->memoryPoolUsageReport("AAAMemStats");
#endif

	shutdownMemoryManager();

#ifdef RTS_ENABLE_CRASHDUMP
	MiniDumper::shutdownMiniDumper();
#endif

	TheAsciiStringCriticalSection = nullptr;
	TheUnicodeStringCriticalSection = nullptr;
	TheDmaCriticalSection = nullptr;
	TheMemoryPoolCriticalSection = nullptr;
	TheDebugLogCriticalSection = nullptr;
}

} // namespace AppMain
