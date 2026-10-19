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

void CrashReporting::initialize(const AsciiString& userDirectory, int major, int minor, int build)
{
#ifdef RTS_ENABLE_CRASHDUMP
#if RTS_ZEROHOUR
    MiniDumper::initMiniDumper(userDirectory.str(), 'Z');
#else
    MiniDumper::initMiniDumper(userDirectory.str(), 'G');
#endif
    DEBUG_LOG(("Crash reporting: %s\n",
        TheMiniDumper && TheMiniDumper->IsInitialized() ? "minidumper" : "unavailable"));
#endif
}

void CrashReporting::userDirectoryReady(const AsciiString& userDirectory)
{
}

void CrashReporting::captureFatal()
{
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
#ifdef RTS_ENABLE_CRASHDUMP
    MiniDumper::shutdownMiniDumper();
#endif
}
