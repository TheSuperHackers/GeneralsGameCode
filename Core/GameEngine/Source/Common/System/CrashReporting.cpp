/* SPDX-License-Identifier: GPL-3.0-or-later */
// TheSuperHackers @feature Codex 01/10/2026 Select optional local Crashpad reporting with legacy MiniDumper fallback.
#include "PreRTS.h" // This must go first in EVERY cpp file in the GameEngine
#include "Common/CrashReporting.h"
#include "Common/MiniDumper.h"

#ifdef RTS_USE_CRASHPAD
#include "CrashpadBridge.h"
#include "gitinfo.h"

namespace
{
    RtsCrashpadCaptureFatalFunction capture;
    RtsCrashpadShutdownFunction stop;
    bool awaitingUserDirectory;
    int savedMajor;
    int savedMinor;
    int savedBuild;

    bool StartCrashpad(const AsciiString& userDirectory, int major, int minor, int build)
    {
        wchar_t path[32768];
        const DWORD count = GetModuleFileNameW(nullptr, path, ARRAY_SIZE(path));
        if (!count || count >= ARRAY_SIZE(path))
        {
            return false;
        }

        wchar_t* leaf = wcsrchr(path, L'\\');
        const wchar_t name[] = L"rts_crashpad.dll";
        if (!leaf || (leaf + 1 - path) + ARRAY_SIZE(name) > ARRAY_SIZE(path))
        {
            return false;
        }

        memcpy(leaf + 1, name, sizeof(name));
        HMODULE module = LoadLibraryExW(path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!module)
        {
            return false;
        }

        RtsCrashpadInitializeFunction start = reinterpret_cast<RtsCrashpadInitializeFunction>(
            GetProcAddress(module, "RtsCrashpadInitialize"));
        capture = reinterpret_cast<RtsCrashpadCaptureFatalFunction>(GetProcAddress(module, "RtsCrashpadCaptureFatal"));
        stop = reinterpret_cast<RtsCrashpadShutdownFunction>(GetProcAddress(module, "RtsCrashpadShutdown"));
        char version[64];
        snprintf(version, ARRAY_SIZE(version), "%d.%d.%d", major, minor, build);
#if RTS_ZEROHOUR
        const char* game = "Zero Hour";
#else
        const char* game = "Generals";
#endif
        if (start && capture && stop && start(userDirectory.str(), game, version, GitSHA1, GitUncommittedChanges))
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

void CrashReporting::initialize(const AsciiString& userDirectory, int major, int minor, int build)
{
#ifdef RTS_USE_CRASHPAD
    // Generals resolves UserDataLeafName from GameData.ini during engine init.
    // Zero Hour already has its registry-derived path at WinMain startup.
    awaitingUserDirectory = userDirectory.isEmpty();
    savedMajor = major;
    savedMinor = minor;
    savedBuild = build;
    if (!awaitingUserDirectory && StartCrashpad(userDirectory, major, minor, build))
    {
        OutputDebugStringA("Crash reporting: Crashpad (local only)\n");
        DEBUG_LOG(("Crash reporting: Crashpad (local only)\n"));
        return;
    }

    if (!awaitingUserDirectory)
    {
        OutputDebugStringA("Crashpad startup failed; trying MiniDumper\n");
        DEBUG_LOG(("Crashpad startup failed; trying MiniDumper\n"));
    }
#endif
#ifdef RTS_ENABLE_CRASHDUMP
    MiniDumper::initMiniDumper(userDirectory);
    DEBUG_LOG(("Crash reporting: %s\n", backendName()));
#endif
}

void CrashReporting::userDirectoryReady(const AsciiString& userDirectory)
{
#ifdef RTS_USE_CRASHPAD
    // Tools never call initialize(), so this cannot start Crashpad in a tool.
    if (awaitingUserDirectory && !userDirectory.isEmpty())
    {
        MiniDumper::shutdownMiniDumper();
        initialize(userDirectory, savedMajor, savedMinor, savedBuild);
    }
#endif
}

const char* CrashReporting::backendName()
{
#ifdef RTS_USE_CRASHPAD
    if (capture)
    {
        return "crashpad";
    }
#endif
#ifdef RTS_ENABLE_CRASHDUMP
    if (TheMiniDumper && TheMiniDumper->IsInitialized())
    {
        return "minidumper";
    }
#endif

    return "unavailable";
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
    awaitingUserDirectory = false;
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
