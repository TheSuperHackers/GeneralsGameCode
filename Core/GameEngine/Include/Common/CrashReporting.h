/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

class AsciiString;

// Keep this interface compatible with VC6 and tools that only use MiniDumper.
namespace CrashReporting
{
    void initialize(const AsciiString& userDirectory, int major, int minor, int build);
    void userDirectoryReady(const AsciiString& userDirectory);
    const char* backendName();
    void captureFatal();
    void shutdown();
}
