# Optional local Crashpad reports

`RTS_BUILD_OPTION_CRASHPAD` defaults to `OFF`. It enables local crash reporting
through a separate handler process for modern MSVC and ClangCL Win32 game builds.
WorldBuilder and other tools keep their existing reporting behavior. Disabled
builds do not acquire or link Crashpad. Unsupported configurations, including
VC6, non-Windows, x64, sanitizer/fuzzing builds and `RTS_CRASHDUMP_ENABLE=OFF`,
reject an explicit request to enable it.

The optional backend expects Windows 10 or newer for the Universal CRT. It does
not change the OS requirements of default builds. A failure to load or start
Crashpad falls back to MiniDumper; the game can start if neither is available.

## Building

Use the separate pinned dependency manifest so ordinary builds keep their
existing dependencies. From the repository root in Git Bash, with `VCPKG_ROOT`
pointing to vcpkg:

```bash
"$VCPKG_ROOT/vcpkg.exe" install \
  --x-manifest-root=Dependencies/Crashpad \
  --x-install-root=build/crashpad-package \
  --triplet=x86-windows-static-md --host-triplet=x86-windows-static-md

cmake -S . -B build/crashpad -G "Visual Studio 18 2026" -A Win32 -T ClangCL \
  -DRTS_BUILD_OPTION_CRASHPAD=ON \
  -DRTS_CRASHPAD_PACKAGE_ROOT="$PWD/build/crashpad-package/x86-windows-static-md" \
  -DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=Embedded
cmake --build build/crashpad --config Release --target g_generals z_generals --parallel 4
```

Omit `-T ClangCL` to build the games and adapter with MSVC. The dependency uses
vcpkg's MSVC/GN build. Both triplets must be native x86 for the pinned port's
Python helper. Configure checks the package's SPDX record against the pinned
port version, architecture and source revisions. Use the handler from that same
package. Debug builds need the Visual Studio debug CRT.

## Capture and fallback

The game loads `rts_crashpad.dll` by absolute path beside its executable. The DLL
isolates Crashpad's C++ requirements and CRT allocations from the game's memory
allocator. Zero Hour initializes it in WinMain. Generals uses MiniDumper until
its user-data path is available after GameData loading and startup options.

Startup waits up to five seconds for handler readiness. Failure restores the
previous exception handlers and selects MiniDumper. A handler that dies later
is not restarted and does not trigger automatic fallback. The DLL stays loaded
because upstream retains process-lifetime IPC pointers; the handler exits when
the game process exits.

Crashpad handles unhandled exceptions and both explicit fatal-error paths.
Explicit fatal errors capture once, before legacy diagnostics. A watchdog
terminates the game if that capture stalls for 15 seconds, provided the process
remains schedulable. Headless mode adds no dialogs. Crashpad reports omit the
legacy unhandled-exception text and do not provide the legacy full-memory dump.

## Reports and deployment

Reports stay in `<user-data>/CrashDumps/Crashpad/`. Uploads are disabled, with no
upload URL or attachments. Each report is a binary minidump with build annotations
including the game version, Git revision, dirty state, compiler and configuration.
Crashpad does not generate a readable diagnosis of the crash.

At startup, the database API prunes reports older than 30 days and older reports
beyond a 500 MiB report-size budget. Active reports remain under Crashpad's
control. This is not a hard disk quota; new reports can exceed it until the next
startup. The handler's default pruning policy is disabled.

After the game exits, copy the desired `.dmp` from the database's `reports`
directory to share it. Keep the original database files in place. Dumps contain
process memory; sharing is a separate user action.

Enabled game outputs and CMake installs include the matching handler, adapter
DLL/PDB, redistributable VC runtime DLLs and dependency notices. Set
`RTS_INSTALL_PREFIX_GENERALS` and `RTS_INSTALL_PREFIX_ZEROHOUR` to explicit staging
paths when testing installation. Runtime paths do not depend on the build tree.

Keep the exact game executable/PDB, adapter DLL/PDB and handler for each
distributed build, together with its source revision. Preserve the source
snapshot for dirty builds. Open the dump as a crash dump in WinDbg or Visual
Studio and load those matching PDBs to inspect the exception and call stack.
CI artifact retention remains 30 days; preserve release symbols separately.
