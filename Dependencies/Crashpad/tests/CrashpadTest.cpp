/* SPDX-License-Identifier: GPL-3.0-or-later */
// Dedicated fault injection only. This executable never starts a game session.
#include <windows.h>
#include <tlhelp32.h>
#include <cstdio>
#include <cstring>
#include <new>
#include "../CrashpadBridge.h"

namespace
{
    bool forbidAllocation;
}

void* operator new(size_t size)
{
    if (forbidAllocation)
    {
        TerminateProcess(GetCurrentProcess(), 90);
    }

    void* memory = HeapAlloc(GetProcessHeap(), 0, size ? size : 1);
    if (!memory)
    {
        throw std::bad_alloc();
    }

    return memory;
}

void operator delete(void* memory) noexcept
{
    if (forbidAllocation)
    {
        TerminateProcess(GetCurrentProcess(), 91);
    }

    HeapFree(GetProcessHeap(), 0, memory);
}

void operator delete(void* memory, size_t) noexcept
{
    operator delete(memory);
}

__declspec(noinline) void CrashpadTestFault(void* address)
{
    *static_cast<volatile int*>(address) = 42;
}

DWORD WINAPI FaultingWorker(void* address)
{
    std::printf("fault-thread=%lu\n", GetCurrentThreadId());
    std::fflush(stdout);
    CrashpadTestFault(address);
    return 0;
}

__declspec(noinline) unsigned int OverflowStack(unsigned int depth)
{
    if (depth == 0xffffffffu)
    {
        return depth;
    }

    volatile char page[4096];
    page[depth % sizeof(page)] = static_cast<char>(depth);
    return OverflowStack(depth + 1) + page[depth % sizeof(page)];
}

bool KillOwnHandler()
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W process = {};
    process.dwSize = sizeof(process);
    bool killed = false;
    if (Process32FirstW(snapshot, &process))
    {
        do
        {
            if (process.th32ParentProcessID == GetCurrentProcessId()
                && wcscmp(process.szExeFile, L"crashpad_handler.exe") == 0)
            {
                HANDLE child = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, process.th32ProcessID);
                if (child)
                {
                    killed = TerminateProcess(child, 1) != FALSE;
                    WaitForSingleObject(child, 5000);
                    CloseHandle(child);
                }
            }
        } while (Process32NextW(snapshot, &process));
    }

    CloseHandle(snapshot);
    return killed;
}

LONG WINAPI PreviousFilter(EXCEPTION_POINTERS*)
{
    return EXCEPTION_EXECUTE_HANDLER;
}

int wmain(int argc, wchar_t** argv)
{
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter(PreviousFilter);
    if (argc != 3)
    {
        return 2;
    }

    char userDirectory[32768 * 4];
    if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, argv[1], -1,
                            userDirectory, sizeof(userDirectory), nullptr, nullptr))
    {
        return 2;
    }

    forbidAllocation = true;
    const ULONGLONG start = GetTickCount64();
    if (!RtsCrashpadInitialize(userDirectory, "test", "1.0.0", "test-revision", 1))
    {
        const bool restored = SetUnhandledExceptionFilter(PreviousFilter) == PreviousFilter;
        std::printf("backend=unavailable filter-restored=%d\n", restored);
        return restored ? 3 : 4;
    }

    std::printf("backend=crashpad startup-ms=%llu fault-thread=%lu\n",
                GetTickCount64() - start, GetCurrentThreadId());
    std::fflush(stdout);
    void* address = VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_NOACCESS);
    if (wcscmp(argv[2], L"main") == 0)
    {
        CrashpadTestFault(address);
    }
    else if (wcscmp(argv[2], L"worker") == 0)
    {
        HANDLE worker = CreateThread(nullptr, 0, FaultingWorker, address, 0, nullptr);
        WaitForSingleObject(worker, INFINITE);
    }
    else if (wcscmp(argv[2], L"stack") == 0)
    {
        OverflowStack(0);
    }
    else if (wcscmp(argv[2], L"heap") == 0)
    {
        RaiseException(0xC0000374, EXCEPTION_NONCONTINUABLE, 0, nullptr);
    }
    else if (wcscmp(argv[2], L"explicit") == 0 || wcscmp(argv[2], L"duplicate") == 0)
    {
        RtsCrashpadCaptureFatal();
        RtsCrashpadCaptureFatal();
        if (wcscmp(argv[2], L"duplicate") == 0)
        {
            CrashpadTestFault(address);
        }
    }
    else if (wcscmp(argv[2], L"dead-handler") == 0)
    {
        if (!KillOwnHandler())
        {
            return 5;
        }

        RtsCrashpadCaptureFatal();
        return 6;
    }
    else if (wcscmp(argv[2], L"wait") == 0)
    {
        Sleep(1000);
    }
    else if (wcscmp(argv[2], L"shutdown") != 0)
    {
        return 2;
    }

    RtsCrashpadShutdown();
    return 0;
}
