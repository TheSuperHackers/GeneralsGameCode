# Optional local Crashpad reports

`RTS_BUILD_OPTION_CRASHPAD` defaults to `OFF`. It enables a local-only prototype
for the modern MSVC and ClangCL Win32 game builds. WorldBuilder and other tools
keep their existing reporting behavior. VC6, MinGW and non-Windows builds do not
acquire or link Crashpad when the option is off. Explicit unsupported requests,
including `RTS_CRASHDUMP_ENABLE=OFF` and sanitizer/fuzzing builds, fail configure.

This prototype has been exercised on Windows 11. Its deployment includes the
x86 Visual C++ redistributable DLLs and expects the Windows 10 or newer Universal
CRT. It does not change the OS requirements of default-off builds. Older Windows
versions have not been qualified for the optional backend. Loading the adapter
can fail on an older OS, in which case the game tries MiniDumper.

## Build the pinned package

Use the separate dependency manifest so ordinary game builds do not install
Crashpad or change the root vcpkg dependency set. Run these commands from the
repository root in Git Bash, with `VCPKG_ROOT` pointing to vcpkg:

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

Omit `-T ClangCL` to build the games and adapter with MSVC. The package itself
uses vcpkg's MSVC/GN route. The baseline Python helper requires a native triplet,
so both target and host are explicitly x86 here. A copy of this installed prefix
can be supplied to a build that does not otherwise use vcpkg. Configure checks
the package's SPDX provenance, architecture and port version.

| Component | Pin / configuration |
| --- | --- |
| vcpkg baseline | `b02e341c927f16d991edbd915d8ea43eac52096c` |
| Crashpad port | `2024-04-11#7` |
| Crashpad source | `7e0af1d4d45b526f01677e74a56f4a951b70517d` |
| mini_chromium | `dce72d97d1c2e9beb5e206c6a05a702269794ca3` |
| Client | x86 static libraries, dynamic CRT, Debug `/MDd` and Release `/MD` |
| Handler | Release x86, from the same package and source revision |
| zlib and build helpers | Pinned transitively by the manifest baseline; SPDX and ABI records accompany deployment |

The baseline port exports `crashpad::crashpad` and packages the handler at
`tools/crashpad/crashpad_handler.exe`. Do not substitute a separately downloaded
handler. Debug builds require the Visual Studio debug CRT and are not release
packages.

## Startup and ownership

The game loads `rts_crashpad.dll` by absolute path beside its executable. That DLL
contains the statically linked client and uses its own CRT allocation functions.
It does not inherit the game PCH, forced includes, memory-pool macros or global
`new`/`delete` overrides. Only C functions and borrowed string arguments cross
the boundary. Tools have no import-library dependency on this DLL.

Zero Hour starts Crashpad at the previous MiniDumper initialization point.
Generals initially uses MiniDumper there because its user-data path is still
empty. After loading the GameData INIs and parsing engine startup options,
Generals replaces that temporary backend using the resolved user-data path.
This differs from the handoff's assumption that both games know the path in
WinMain. Early crashes remain outside Crashpad coverage.

Initialization converts the game's UTF-8 user path to UTF-16, creates the
database, disables uploads, and starts the handler asynchronously with a
five-second readiness wait. Success requires the upstream IPC ping to complete.
This bounds startup when a valid executable fails to service the handler pipe.
There is no upload URL, metrics directory
or attachment list. Build annotations contain the game, application version,
full Git revision, dirty state, x86 architecture, compiler, CMake configuration
and backend.

On failure the adapter restores the previous exception filter and abort handler,
removes its heap-corruption handler and stops its watchdog. The game logs one
failure and tries MiniDumper. `CrashReporting::backendName()` reports `crashpad`,
`minidumper` or `unavailable`; startup continues if reporting is unavailable.
Startup selection does not monitor a handler that dies later, and Windows does
not restart the handler in this revision.

Crashpad owns unhandled exceptions while active. The legacy WinMain filter does
not run first, so Crashpad reports omit the legacy `DumpExceptionInfo` text.
The debug library's `PreStaticInit()` registration runs before WinMain; the
WorldBuilder registration belongs to a separate executable. Neither replaces
Crashpad later in game startup. Normal shutdown restores the original filter.
The DLL stays loaded because upstream retains process-lifetime IPC pointers;
the handler exits when the game process exits.

Both explicit fatal-error variants capture before best-effort diagnostics and
keep their existing exit paths. The pinned `DumpWithoutCrash()` waits without a
timeout, so the adapter creates a watchdog thread during healthy startup. If an
explicit fatal capture does not return within 15 seconds, that thread terminates
the game with exit code 1. In that case later text diagnostics may be absent.
This bound depends on the game process remaining schedulable; it cannot promise
recovery from arbitrary corruption or a handler that leaves the process
suspended. The diagnostic capture is one-shot, and a subsequent reporting fault
terminates without generating a second report. Headless mode adds no dialogs.
Upstream unhandled-exception handling has its separate 60-second termination
fallback.

## Reports, export and retention

The database is `<user-data>/CrashDumps/Crashpad/`. Crashpad produces one minidump,
not the legacy minimal/full-memory pair. Full-memory parity is unproven. Build
with the option off when an investigation needs the old full dump.

At startup, Crashpad's database API prunes reports older than 30 days and older
reports beyond a 500 MiB report-size budget. Active/locked reports remain under
Crashpad's control. The handler's different default pruning policy is disabled.
The budget excludes database metadata and is not a hard disk quota; concurrent
runs and newly written reports can exceed it until the next startup or explicit
prune. The legacy filename cleanup does not enter this subdirectory.

Use the installed helper instead of manipulating database metadata:

```bash
rts_crashpad_reports.exe "C:/path/to/user-data/CrashDumps/Crashpad" list
rts_crashpad_reports.exe "C:/path/to/user-data/CrashDumps/Crashpad" export "C:/path/to/new-export"
rts_crashpad_reports.exe "C:/path/to/user-data/CrashDumps/Crashpad" prune
```

Export enumerates finalized reports through the database API and copies them
without marking them uploaded. It refuses to overwrite an existing destination
file and reports concurrent-pruning failures. Dumps contain process memory even
without attachments; manual sharing is a separate user action.

## Deployment and symbols

Each enabled game output and CMake install includes the matching handler,
adapter DLL/PDB, report helper, redistributable VC runtime DLLs, dependency
notices and `*-crashpad-build.json`. Set `RTS_INSTALL_PREFIX_GENERALS` and
`RTS_INSTALL_PREFIX_ZEROHOUR` to explicit staging paths when testing installation.
No build-tree path is used to locate the runtime handler or database.

Keep the exact executable, game PDB, adapter DLL/PDB, handler and metadata for
every distributed build. With LLVM tools on `PATH`, verify CodeView GUID/age
against each PDB and create an archive:

```bash
python Dependencies/Crashpad/archive_symbols.py \
  build/crashpad/Generals/Release/g_generals-crashpad-build.json \
  build/symbols/generals-crashpad.zip
```

Use `GeneralsMD/Release/z_generals-crashpad-build.json` for Zero Hour. The helper
also verifies the build's recorded SHA-256 hashes and refuses to overwrite an
archive. Keep the source commit with the archive; preserve the source snapshot
as well for a build marked dirty. Open an exported dump in WinDbg/CDB or Visual
Studio with the archived PDBs and verify a known function and source line.

CI collection includes the notices and metadata and retains artifacts for 90
days. That is temporary storage. Attach the verified symbol archive to the
corresponding release, or copy it to the project's durable symbol archive,
before distributing a Crashpad-enabled release. No reporting server or release
upload is configured by this change.

## Validation

Enable `RTS_BUILD_CRASHPAD_TESTS=ON` in the Crashpad build tree, then run:

```bash
cmake --build build/crashpad --config Release \
  --target rts_crashpad_test rts_crashpad_seed rts_crashpad_reports --parallel 4
python Dependencies/Crashpad/tests/validate.py \
  build/crashpad/Dependencies/Crashpad/tests/Release/rts_crashpad_test.exe \
  build/crashpad-validation
```

The script runs only the dedicated test executable in fresh directories under
`build/`. It checks original exception codes and thread IDs, one report per
event, fatal timeout after handler death, startup failure/filter restoration,
paths with spaces and non-ASCII characters, concurrent instances, disabled
uploads, export identity, and pruning of a 600 MiB synthetic database. The
executable forbids use of its global allocator during adapter operations.

See [the validation record](crashpad-validation.md) for local build and runtime
results and the remaining qualification work. Keep the option off by default
until those limitations and full-memory requirements have been resolved.
