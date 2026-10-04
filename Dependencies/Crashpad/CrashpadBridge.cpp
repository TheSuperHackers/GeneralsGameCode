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

// Deliberately compiled outside the game PCH, forced includes and allocator.
#include <windows.h>
#include <csignal>
#include <memory>
#include <string>
#include <vector>

#include "CrashpadBridge.h"
#include "client/crashpad_client.h"
#include "client/crash_report_database.h"
#include "client/prune_crash_reports.h"
#include "client/settings.h"
#include "util/misc/capture_context.h"

namespace
{
    std::unique_ptr<crashpad::CrashpadClient> client;
    LPTOP_LEVEL_EXCEPTION_FILTER previousFilter;
    void (*previousAbortHandler)(int);
    HANDLE stopEvent;
    HANDLE captureEvent;
    HANDLE completedEvent;
    HANDLE watchdogThread;
    volatile LONG capturedFatal;
    bool attempted;
    bool active;

    // DumpWithoutCrash waits indefinitely in the pinned Windows client. Keep
    // the request on the faulting thread, and bound fatal capture with a thread
    // created while the process is healthy. Do not terminate a reporting thread
    // and then let the game reuse memory still referenced by the handler.
    DWORD WINAPI Watchdog(void*)
    {
        const HANDLE events[] = {stopEvent, captureEvent};
        if (WaitForMultipleObjects(2, events, FALSE, INFINITE) == WAIT_OBJECT_0 + 1)
        {
            if (WaitForSingleObject(completedEvent, 15000) != WAIT_OBJECT_0)
            {
                TerminateProcess(GetCurrentProcess(), 1);
            }
        }

        return 0;
    }

    LONG WINAPI AlreadyCaptured(EXCEPTION_POINTERS*)
    {
        // A failure in best-effort fatal diagnostics must not create a second
        // report that hides the original explicit fatal capture.
        TerminateProcess(GetCurrentProcess(), 1);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    void StopWatchdog()
    {
        if (watchdogThread)
        {
            SetEvent(stopEvent);
            WaitForSingleObject(watchdogThread, INFINITE);
            CloseHandle(watchdogThread);
            watchdogThread = nullptr;
        }

        if (stopEvent)
        {
            CloseHandle(stopEvent);
            stopEvent = nullptr;
        }

        if (captureEvent)
        {
            CloseHandle(captureEvent);
            captureEvent = nullptr;
        }

        if (completedEvent)
        {
            CloseHandle(completedEvent);
            completedEvent = nullptr;
        }
    }

    bool Initialize(const char* userDirectory, const char* game,
                    const char* version, const char* revision, int dirty)
    {
        const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                             userDirectory, -1, nullptr, 0);
        if (count <= 1)
        {
            return false;
        }

        std::vector<wchar_t> userPath(count);
        if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, userDirectory,
                                -1, userPath.data(), count))
        {
            return false;
        }

        const base::FilePath user(userPath.data());
        if (!user.IsAbsolute())
        {
            return false;
        }

        std::vector<wchar_t> executable(32768);
        const DWORD length = GetModuleFileNameW(nullptr, executable.data(),
                                                static_cast<DWORD>(executable.size()));
        if (!length || length >= executable.size())
        {
            return false;
        }

        const base::FilePath handler = base::FilePath(executable.data()).DirName()
            .Append(L"crashpad_handler.exe");
        if (GetFileAttributesW(handler.value().c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            return false;
        }

        const base::FilePath dumps = user.Append(L"CrashDumps");
        if (!CreateDirectoryW(dumps.value().c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            return false;
        }

        const base::FilePath databasePath = dumps.Append(L"Crashpad");
        auto database = crashpad::CrashReportDatabase::Initialize(databasePath);
        if (!database || !database->GetSettings()->SetUploadsEnabled(false))
        {
            return false;
        }

        // Prune through the database so active/locked reports stay protected.
        // Disable the handler's different default policy below. The size limit
        // applies at startup; reports from concurrent runs can exceed it until
        // the next startup.
        crashpad::BinaryPruneCondition retention(crashpad::BinaryPruneCondition::OR,
            new crashpad::AgePruneCondition(30),
            new crashpad::DatabaseSizePruneCondition(500 * 1024));
        crashpad::PruneCrashReportDatabase(database.get(), &retention);

        stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        captureEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        completedEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!stopEvent || !captureEvent || !completedEvent)
        {
            return false;
        }

        watchdogThread = CreateThread(nullptr, 0, Watchdog, nullptr, 0, nullptr);
        if (!watchdogThread)
        {
            return false;
        }

        const std::map<std::string, std::string> annotations = {
            {"game", game}, {"version", version}, {"revision", revision},
            {"dirty", dirty ? "true" : "false"}, {"architecture", "x86"},
            {"compiler", RTS_CRASHPAD_COMPILER}, {"configuration", RTS_CRASHPAD_CONFIGURATION},
            {"backend", "crashpad"}, {"crashpad_revision", RTS_CRASHPAD_REVISION}
        };
        client = std::make_unique<crashpad::CrashpadClient>();
        // Wait for the IPC ping before claiming readiness, but do not block game
        // startup forever if a valid executable never services the pipe. On a
        // timeout the DLL must stay loaded for the upstream background thread.
        // Windows has no automatic restart here. No URL, metrics or attachments.
        return client->StartHandler(handler, databasePath, base::FilePath(), "",
            annotations, {"--no-periodic-tasks"}, false, true)
            && client->WaitForHandlerStart(5000);
    }
}

extern "C" int __cdecl RtsCrashpadInitialize(const char* userDirectory,
    const char* game, const char* version, const char* revision, int dirty)
{
    if (attempted)
    {
        return active;
    }

    attempted = true;
    previousFilter = SetUnhandledExceptionFilter(nullptr);
    SetUnhandledExceptionFilter(previousFilter);
    previousAbortHandler = std::signal(SIGABRT, SIG_DFL);
    std::signal(SIGABRT, previousAbortHandler);
    try
    {
        active = Initialize(userDirectory, game, version, revision, dirty);
    }
    catch (...)
    {
        active = false;
    }

    if (!active)
    {
        RtsCrashpadShutdown();
    }

    return active;
}

extern "C" void __cdecl RtsCrashpadCaptureFatal()
{
    if (!active || InterlockedCompareExchange(&capturedFatal, 1, 0) != 0)
    {
        return;
    }

    CONTEXT context;
    crashpad::CaptureContext(&context);
    if (!SetEvent(captureEvent))
    {
        TerminateProcess(GetCurrentProcess(), 1);
    }

    crashpad::CrashpadClient::DumpWithoutCrash(context);
    SetEvent(completedEvent);
    SetUnhandledExceptionFilter(AlreadyCaptured);
    // Removes the heap-corruption vectored handler too. The client leaves its
    // IPC state alive for process lifetime; never unload this DLL.
    client.reset();
    std::signal(SIGABRT, SIG_DFL);
}

extern "C" void __cdecl RtsCrashpadShutdown()
{
    active = false;
    SetUnhandledExceptionFilter(previousFilter);
    std::signal(SIGABRT, previousAbortHandler);
    client.reset();
    StopWatchdog();
    // The handler observes process exit and exits itself. Crashpad's Windows
    // API intentionally retains its IPC handles until then.
}
