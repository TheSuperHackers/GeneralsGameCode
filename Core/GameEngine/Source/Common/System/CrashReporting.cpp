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

// TheSuperHackers @feature CryoTheRenegade 01/10/2026 Select optional local Crashpad reporting with legacy MiniDumper fallback.
#include "PreRTS.h" // This must go first in EVERY cpp file in the GameEngine
#include "Common/CrashReporting.h"
#ifdef RTS_ENABLE_CRASHDUMP
#include "MiniDumper/MiniDumper.h"
#endif

#ifdef RTS_USE_CRASHPAD
#include "CrashpadBridge.h"
#include "gitinfo.h"

namespace
{
    RtsCrashpadCaptureFatalFunction capture;
    RtsCrashpadShutdownFunction stop;
    Bool awaitingUserDirectory;

    Bool StartCrashpad(const AsciiString& userDirectory)
    {
        HMODULE module = LoadLibraryExW(L"rts_crashpad.dll", nullptr,
            LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!module)
        {
            return false;
        }

        RtsCrashpadInitializeFunction start = reinterpret_cast<RtsCrashpadInitializeFunction>(
            GetProcAddress(module, "RtsCrashpadInitialize"));
        capture = reinterpret_cast<RtsCrashpadCaptureFatalFunction>(GetProcAddress(module, "RtsCrashpadCaptureFatal"));
        stop = reinterpret_cast<RtsCrashpadShutdownFunction>(GetProcAddress(module, "RtsCrashpadShutdown"));
#if RTS_ZEROHOUR
        const char* game = "Zero Hour";
#else
        const char* game = "Generals";
#endif
        if (start && capture && stop && start(userDirectory.str(), game, GitSHA1, GitUncommittedChanges))
        {
            return true;
        }

        // Crashpad retains process-lifetime pointers even after failed startup.
        // Keep the module loaded; its initializer restores the previous filter.
        capture = nullptr;
        stop = nullptr;
        return false;
    }
}
#endif

void CrashReporting::initialize(const AsciiString& userDirectory)
{
#ifdef RTS_USE_CRASHPAD
    // Generals resolves UserDataLeafName from GameData.ini during engine init.
    // Zero Hour already has its registry-derived path at WinMain startup.
    awaitingUserDirectory = userDirectory.isEmpty();
    if (!awaitingUserDirectory)
    {
        if (StartCrashpad(userDirectory))
        {
            DEBUG_LOG(("Crash reporting: Crashpad (local only)"));
            return;
        }

        DEBUG_LOG(("Crashpad startup failed; trying MiniDumper"));
    }
#endif
#ifdef RTS_ENABLE_CRASHDUMP
#if RTS_ZEROHOUR
    MiniDumper::initMiniDumper(userDirectory.str(), 'Z');
#else
    MiniDumper::initMiniDumper(userDirectory.str(), 'G');
#endif
    DEBUG_LOG(("Crash reporting: %s",
        TheMiniDumper && TheMiniDumper->IsInitialized() ? "minidumper" : "unavailable"));
#endif
}

void CrashReporting::userDirectoryReady(const AsciiString& userDirectory)
{
#ifdef RTS_USE_CRASHPAD
    // Tools never call initialize(), so this cannot start Crashpad in a tool.
    if (awaitingUserDirectory && !userDirectory.isEmpty())
    {
#ifdef RTS_ENABLE_CRASHDUMP
        MiniDumper::shutdownMiniDumper();
#endif
        initialize(userDirectory);
    }
#endif
}

void CrashReporting::captureFatal()
{
#ifdef RTS_USE_CRASHPAD
    if (capture)
    {
        capture();
        return;
    }
#endif
#ifdef RTS_ENABLE_CRASHDUMP
    if (TheMiniDumper && TheMiniDumper->IsInitialized())
    {
        // Preserve both minimal and full memory dumps for the legacy backend.
        TheMiniDumper->TriggerMiniDump(DumpType_Minimal);
        TheMiniDumper->TriggerMiniDump(DumpType_Full);
    }

    MiniDumper::shutdownMiniDumper();
#endif
}

void CrashReporting::shutdown()
{
#ifdef RTS_USE_CRASHPAD
    if (stop)
    {
        stop();
        stop = nullptr;
        capture = nullptr;
    }
#endif
#ifdef RTS_ENABLE_CRASHDUMP
    MiniDumper::shutdownMiniDumper();
#endif
}
