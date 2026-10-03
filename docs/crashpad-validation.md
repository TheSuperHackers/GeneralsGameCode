# Crashpad validation record

Local validation on 2026-10-01, based on upstream
`b0c29eba834e7030e5c4d5f86f985e8ff799c92c`. The implementation was an uncommitted
working tree, so generated build metadata correctly reports `dirty: true`.

Environment: Windows 11 build 26200, Visual Studio 18 2026, ClangCL 22.1.3,
Windows SDK 10.0.28000.0, CMake 4.4.0, Win32 targets. The dependency used the
separate pinned vcpkg manifest and `x86-windows-static-md` for target and host.
Historical builds used Visual C++ 6 SP6, compiler 12.0.8804.

## Completed checks

| Check | Result |
| --- | --- |
| Standalone pinned x86 client/handler | One local access-violation report; CDB resolved the intentional faulting function |
| Both ClangCL Release games, Crashpad on | Build and isolated startup passed with the normal game allocator enabled |
| Both ClangCL Release games, Crashpad off | Builds passed without a Crashpad package requirement |
| Both ClangCL Release games, master crash-dump switch off | Builds passed |
| Both VC6 Release games | Builds passed; Crashpad remains disabled |
| Unsupported explicit configurations | x64, VC6, ASan, and Crashpad-on/master-off rejected with the intended configure errors |
| Dedicated Release and Debug adapter tests | Main-thread and worker-thread access violations preserved their exception code and faulting thread ID |
| Stack overflow | Captured `0xc00000fd` in the dedicated test executable |
| Heap-corruption exception | Captured a deliberately raised `0xc0000374`; this was not a real damaged-heap experiment |
| Explicit diagnostic capture | One `0x0517a7ed` report on the requesting thread; repeated capture and a subsequent access violation produced no duplicate |
| Missing/invalid handler and blocked database path | Startup failed cleanly and restored the previous exception filter |
| Valid executable that never answers handler IPC | Readiness timed out in approximately five seconds, restored the previous filter and allowed fallback |
| Handler terminated before explicit capture | No report, game/test process exited with code 1 after approximately 15 seconds |
| Wide paths | Handler and database worked in paths containing spaces, accented Latin characters and CJK characters |
| Concurrent instances | Three processes wrote three reports into the same database |
| Size retention | Startup pruned three synthetic 200 MiB reports to two reports, below the 500 MiB budget |
| Manual export | Exported bytes matched the original report; database uploads remained disabled |
| Isolated game fatal paths | Both `ReleaseCrash` and the localized branch produced one report plus `ReleaseCrashInfo.txt` in both games, without dialogs |
| Legacy fallback in both games | Missing handler selected MiniDumper; explicit fatal capture produced the original minimal/full pair and text report |
| Game symbolization | CDB resolved `CrashpadGameTestFault` and its source line in both game executables using their matching PDBs |
| Install layout | CMake installed handler, adapter/PDB, report helper, VC runtime DLLs, notices and metadata into separate staging directories |
| Symbol archives | Both game archives passed SHA-256 checks and CodeView GUID/age comparisons for game and adapter PDBs |
| Source checks | `git diff --check` and exact-case validation of added includes passed |

The game fault tests used generated source copies under `build/game-validation`.
They redirected user-data paths into that directory, invoked the unchanged fatal
functions, and injected an access violation in a named test function. The
localized test supplied a small text-provider stub. They did not crash a player
session or modify the retail installation. The normal build trees were restored
to their production sources afterward; no fault-injection mode is compiled into
the shipped games.

The separate adapter executable rejects calls to its own global allocator while
Crashpad runs. This tests the DLL allocation boundary independently of the game
tests, which report that the normal game allocator is initialized.

## Existing gameplay crash

The replay and custom map attached to
[issue #3185](https://github.com/TheSuperHackers/GeneralsGameCode/issues/3185)
reproduced the Zero Hour Rebel Ambush crash twice on the same local build.
Each headless run produced one report and exited with `0xc0000005`, in 1.50 and
1.21 seconds. The exported sample was 464,244 bytes. CDB loaded the matching
private PDB and resolved the null-object read to
`AIGroup::groupDoSpecialPowerAtLocation` at `AIGroup.cpp:2744`, through
`Object::getControllingPlayer`. The fault attempted to read address `0x000001e0`.

This capture used normal game logic and WinMain sources. Its only additional
test change redirected user data through an environment variable in a generated
copy of `GlobalData.cpp`. No artificial fault was injected. A separate replay
script reproduced the crash again using the archived executable. The original
source configuration was restored afterward.

The report, exact executable/PDBs, dependency notices, source changes and replay
were packaged locally for developer feedback. Uploads remained disabled. The
sample confirms capture and symbolization; it does not establish better capture
reliability than MiniDumper. Global game state outside the captured memory could
not be inspected, so this is not evidence of full-memory parity.

## PR preparation checks

On 2026-10-03 the first repeated Release test run exited with Windows status
`0xc0000142` before the shutdown test entered `main`. The same binaries passed
the complete matrix on retry and 30 further fresh-process startup/shutdown
checks. The loader failure was not reproduced and its cause was not established.
The failed log is retained locally; the retry is not presented as a diagnosis.

## Replay comparison

The local ClangCL test runs did not pass retail CRC validation. They produced
the same failures with the feature enabled and disabled, using the same staged
assets and replay files:

| Game | Replay | Feature on and off |
| --- | --- | --- |
| Generals | `CW_050307_OoE_Silver_CvC_awesome.rep` | CRC mismatch at frame 110 |
| Zero Hour | `!Golden Replay #1.rep`, with the `tansooo` map supplied | CRC mismatch at frame 111 |

This comparison found no change in those outcomes. It does not establish retail
replay compatibility or resolve the existing local mismatches.

## Remaining qualification before default enablement

- Clean-machine testing without Visual Studio or an existing VC runtime,
  and OS versions other than the Windows 11 test host.
- Full Debug and Tracy game runs. The debug client/adapter was tested, but that
  is not a complete debug-game or profiling-game qualification.
- Multiplayer smoke testing and passing retail replay fixtures.
- Real allocator corruption, handler failure while it has suspended the game,
  and scheduler/starvation failures beyond the runnable-process watchdog case.
- Age-limit testing across 30 days. The configured upstream age policy is
  present; the automated retention test exercises the size policy.
- Network packet observation, prolonged resident-memory measurements and disk
  usage over real play sessions. Local-only settings and an empty upload URL
  were verified; no packet-capture claim is made.
- Full-memory dump parity. Keep the legacy backend available for this use.

Standalone startup samples were generally 15–100 ms on this host; those are not
representative gameplay benchmarks. Report persistence is verified by opening
and inspecting reports, not by treating `DumpWithoutCrash()` returning as proof.

The pinned [Windows client implementation](https://github.com/chromium/crashpad/blob/7e0af1d4d45b526f01677e74a56f4a951b70517d/client/crashpad_client_win.cc)
defines the blocking and lifetime behavior used by the adapter. See the
[build and deployment guide](crashpad.md) for reproduction commands and the
release-symbol retention procedure.
