/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

// C ABI only. No allocation or C++ object crosses the game/DLL boundary.
// The game loads these exports explicitly so a missing DLL permits fallback.
typedef int (__cdecl* RtsCrashpadInitializeFunction)(const char* userDirectory,
    const char* game, const char* version, const char* revision, int dirty);
typedef void (__cdecl* RtsCrashpadCaptureFatalFunction)();
typedef void (__cdecl* RtsCrashpadShutdownFunction)();

extern "C" int __cdecl RtsCrashpadInitialize(const char* userDirectory,
    const char* game, const char* version, const char* revision, int dirty);
extern "C" void __cdecl RtsCrashpadCaptureFatal();
extern "C" void __cdecl RtsCrashpadShutdown();
