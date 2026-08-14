# Major Runtime Hardening Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix the eight audited P0-P2 runtime, correctness, performance, and CI defects and publish a verified draft pull request followed by a second P0-P2 review.

**Architecture:** Add small shared validation and trigger-storage primitives that legacy code can call without changing wire or save layouts. Keep runtime changes local to packet parsing, wrapper assembly, receive pacing, object trigger tracking, script caching, and frame pacing. Exercise the pure boundaries through one Zero Hour extras regression executable and use the repository's dual-build and replay gates for integrated legacy behavior.

**Tech Stack:** C++98-compatible C++ compiled by Visual Studio 6 and C++20 compiled by Visual Studio 2022, Win32 timing APIs, CMake/Ninja, GitHub Actions YAML, repository replay harness.

## Global Constraints

- Preserve the existing network wire layout and command identifiers.
- Preserve the existing object trigger serialization order and signed byte count.
- Preserve unchanged retail-compatible simulation behavior.
- Add no third-party dependencies.
- Cap one wrapped command at 64 MiB and aggregate wrapper allocations at 256 MiB.
- Process no more than `MAX_MESSAGES` UDP datagrams per update.
- Retain five trigger records inline and allow at most 127 active records.
- Keep all implementation syntax compatible with Visual Studio 6.
- Run build and replay validation sequentially with at most two local build jobs.

---

### Task 1: Network allocation and wrapper validation

**Files:**
- Create: `Core/GameEngine/Include/GameNetwork/NetCommandValidation.h`
- Create: `GeneralsMD/Code/Tools/RuntimeRegressionTests/CMakeLists.txt`
- Create: `GeneralsMD/Code/Tools/RuntimeRegressionTests/RuntimeRegressionTests.cpp`
- Modify: `GeneralsMD/Code/Tools/CMakeLists.txt`
- Modify: `Core/GameEngine/Include/GameNetwork/NetworkDefs.h`
- Modify: `Core/GameEngine/Include/GameNetwork/NetPacketStructs.h`
- Modify: `Core/GameEngine/Source/GameNetwork/NetPacketStructs.cpp`
- Modify: `Core/GameEngine/Include/GameNetwork/NetCommandWrapperList.h`
- Modify: `Core/GameEngine/Source/GameNetwork/NetCommandWrapperList.cpp`
- Modify: `Core/GameEngine/Source/GameNetwork/ConnectionManager.cpp`

**Interfaces:**
- Produces: `IsValidNetworkPayloadLength(UnsignedInt, size_t, UnsignedInt)`.
- Produces: `WrappedCommandMetadata` and `IsValidWrappedCommandMetadata(const WrappedCommandMetadata&)`.
- Produces: `NetCommandWrapperList::processWrapper(NetCommandRef*) -> Bool`.
- Produces: `NetCommandWrapperList::getPercentComplete(UnsignedByte, UnsignedShort) -> Int`.
- Produces: `z_runtime_regression_tests` when `RTS_BUILD_ZEROHOUR_EXTRAS=ON`.

- [ ] **Step 1: Add the failing network validation tests and extras target**

Add a console regression executable containing these assertions:

```cpp
WrappedCommandMetadata metadata = { 1, 0, 1, 1024, 128, 0 };
CHECK(IsValidWrappedCommandMetadata(metadata));
metadata.numChunks = 0;
CHECK(!IsValidWrappedCommandMetadata(metadata));
metadata.numChunks = 1;
metadata.totalDataLength = MAX_WRAPPED_COMMAND_SIZE + 1;
CHECK(!IsValidWrappedCommandMetadata(metadata));
CHECK(!IsValidNetworkPayloadLength(4096, 16, MAX_WRAPPED_COMMAND_SIZE));
```

The CMake target must link `z_gameengine`, use the console subsystem on Windows, and be included from `GeneralsMD/Code/Tools/CMakeLists.txt` only for Zero Hour extras builds.

- [ ] **Step 2: Configure and build the test to verify RED**

Run:

```powershell
cmake --preset win32 -DRTS_BUILD_ZEROHOUR_EXTRAS=ON -DRTS_BUILD_CORE_TOOLS=OFF -DRTS_BUILD_GENERALS_TOOLS=OFF -DRTS_BUILD_ZEROHOUR_TOOLS=OFF
cmake --build --preset win32-debug --target z_runtime_regression_tests -j 2
```

Expected: compilation fails because `GameNetwork/NetCommandValidation.h` or its validation interfaces do not exist.

- [ ] **Step 3: Add bounded validation primitives**

Define these limits in `NetworkDefs.h`:

```cpp
static const UnsignedInt MAX_WRAPPED_COMMAND_SIZE = 64U * 1024U * 1024U;
static const UnsignedInt MAX_WRAPPED_COMMAND_MEMORY = 256U * 1024U * 1024U;
static const UnsignedInt MAX_WRAPPED_COMMAND_CHUNKS = 256U * 1024U;
```

Implement C++98-compatible inline validation that rejects zero fields, out-of-range chunks, payloads larger than `MAX_PACKET_SIZE`, totals larger than 64 MiB, chunk counts above the configured ceiling, and offset/length pairs checked as `dataLength > totalDataLength - dataOffset` after proving `dataOffset < totalDataLength`.

- [ ] **Step 4: Validate packet payload lengths before allocation**

In wrapper and file `readMessage` implementations, validate the declared length against `buf.offset(fixedSize).size()` before constructing `NetCommandDataChunk`. Wrapper chunks use `MAX_PACKET_SIZE`; file commands use `MAX_WRAPPED_COMMAND_SIZE`. On failure, log the malformed metadata, leave data empty, and consume the remaining buffer without allocating the declared size.

- [ ] **Step 5: Harden wrapper assembly**

Store the originating player ID and allocation size in each node. Reject invalid first chunks before node construction, require subsequent chunks to match player ID, total size, and chunk count, key lookup by player plus wrapped command ID, and account for both data and chunk-presence allocations. Reject a new node when its allocation would exceed the aggregate budget.

Make completion require nonzero chunk count and exact presence count. In `getReadyCommands`, guard a null reconstructed command before dereferencing it and always remove malformed completed nodes.

- [ ] **Step 6: Verify GREEN**

Run:

```powershell
cmake --build --preset win32-debug --target z_runtime_regression_tests -j 2
& build/win32/GeneralsMD/Code/Tools/RuntimeRegressionTests/Debug/z_runtime_regression_tests.exe
```

Expected: executable exits 0 and reports all network validation checks passed.

- [ ] **Step 7: Commit the network hardening**

Inspect `git status --short`, `git diff --check`, and the scoped diff, then commit with:

```text
fix(network): validate wrapped command metadata

- reject impossible payload lengths before allocating buffers
- bound per-command and aggregate wrapper memory
- isolate wrapper assembly by player and discard malformed results
- add focused validation regressions to the Zero Hour extras build
```

### Task 2: UDP receive work budget

**Files:**
- Modify: `Core/GameEngine/Include/GameNetwork/NetCommandValidation.h`
- Modify: `GeneralsMD/Code/Tools/RuntimeRegressionTests/RuntimeRegressionTests.cpp`
- Modify: `Core/GameEngine/Source/GameNetwork/Transport.cpp`

**Interfaces:**
- Produces: `ShouldReceiveNetworkMessage(UnsignedInt processed, Bool hasCapacity)`.
- Consumes: `MAX_MESSAGES`.

- [ ] **Step 1: Add failing receive-budget tests**

Add these assertions:

```cpp
CHECK(ShouldReceiveNetworkMessage(0, TRUE));
CHECK(ShouldReceiveNetworkMessage(MAX_MESSAGES - 1, TRUE));
CHECK(!ShouldReceiveNetworkMessage(MAX_MESSAGES, TRUE));
CHECK(!ShouldReceiveNetworkMessage(0, FALSE));
```

- [ ] **Step 2: Verify RED**

Build `z_runtime_regression_tests` and confirm compilation fails because `ShouldReceiveNetworkMessage` is missing.

- [ ] **Step 3: Bound the receive loop**

Add a processed-datagram counter incremented immediately after every successful socket read, including malformed or debug-dropped packets. Stop reads after `MAX_MESSAGES`. Track whether a free normal or delayed receive-buffer slot remains and stop when it does not. Do not drain, decrypt, or checksum additional packets after capacity is exhausted.

- [ ] **Step 4: Verify GREEN and commit**

Run the regression executable, build the affected Zero Hour game-engine target, inspect the diff, then commit with:

```text
perf(network): bound receive work per update

- cap socket reads even when packets are malformed or dropped
- stop receiving when the game-side input buffer has no capacity
- keep the main loop responsive during sustained UDP traffic
```

### Task 3: Trigger storage and script cache correctness

**Files:**
- Create: `Core/GameEngine/Include/GameLogic/TriggerInfo.h`
- Modify: `Core/GameEngine/CMakeLists.txt`
- Modify: `Generals/Code/GameEngine/Include/GameLogic/Object.h`
- Modify: `GeneralsMD/Code/GameEngine/Include/GameLogic/Object.h`
- Modify: `Generals/Code/GameEngine/Source/GameLogic/Object/Object.cpp`
- Modify: `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Object.cpp`
- Modify: `GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptConditions.cpp`
- Modify: `GeneralsMD/Code/Tools/RuntimeRegressionTests/RuntimeRegressionTests.cpp`

**Interfaces:**
- Produces: `TriggerInfoStorage` with `InlineCapacity`, `MaxCapacity`, `resize`, `operator[]`, and `clearTransitionsAndRemoveExited`.
- Preserves: `Object::m_numTriggerAreasActive` as `Byte` and its existing xfer order.

- [ ] **Step 1: Add failing trigger regression tests**

Add tests that create two records where record 0 exited and record 1 remains inside, compact them, and assert the live record moves to index 0 with transition flags cleared. Add six records and assert index 5 is addressable and retained after compaction.

- [ ] **Step 2: Verify RED**

Build the regression target and confirm compilation fails because `GameLogic/TriggerInfo.h` and `TriggerInfoStorage` do not exist.

- [ ] **Step 3: Implement shared inline-plus-overflow storage**

Move `TTriggerInfo` to the shared header. Store five inline records plus `std::vector<TTriggerInfo>` overflow. Reject sizes outside 0 through 127, resize overflow only beyond five, and compact with distinct source and destination indices:

```cpp
for (Int source = 0; source < size; ++source)
{
    if (!(*this)[source].isInside)
        continue;
    if (destination != source)
        (*this)[destination] = (*this)[source];
    (*this)[destination].entered = false;
    (*this)[destination].exited = false;
    ++destination;
}
resize(destination);
```

- [ ] **Step 4: Integrate both Object implementations**

Replace each fixed array with `TriggerInfoStorage`, retain `MAX_TRIGGER_AREA_INFOS` as an alias for the storage maximum, call `resize(active + 1)` before adding a newly entered trigger, call storage compaction from `updateTriggerAreaFlags`, and call `resize` after validating a loaded count before indexing xferred records.

- [ ] **Step 5: Correct the Zero Hour condition cache**

After calculating `comparison`, set custom data to `-1` or `1` and set custom frame to `TheScriptEngine->getFrameObjectCountChanged()`, matching `evaluatePlayerHasUnitTypeInArea`.

- [ ] **Step 6: Verify GREEN and compatibility builds**

Run the regression executable, then build both `g_gameengine` and `z_gameengine` in the modern x86 configuration with `-j 2`.

- [ ] **Step 7: Commit the game-logic fixes**

Inspect the full scoped diff and commit with:

```text
fix(script): preserve trigger and condition state

- compact trigger records from the correct source index in both games
- retain five inline trigger entries and allocate overflow on demand
- keep trigger save ordering and byte count compatible
- cache Zero Hour unit-kind condition results in the intended fields
```

### Task 4: Frame limiter CPU optimization

**Files:**
- Modify: `Core/GameEngine/Include/Common/FrameRateLimit.h`
- Modify: `Core/GameEngine/Source/Common/FrameRateLimit.cpp`
- Modify: `GeneralsMD/Code/Tools/RuntimeRegressionTests/RuntimeRegressionTests.cpp`

**Interfaces:**
- Produces: `FrameRateLimit::~FrameRateLimit()`.
- Produces: a pure wait-duration calculation exposed for regression checks.
- Preserves: `FrameRateLimit::wait(UnsignedInt) -> Real` and reset behavior.

- [ ] **Step 1: Add failing timing regressions**

Add boundary checks for no wait when elapsed time meets the target and a positive coarse wait when sufficient time remains. Add a 480 FPS benchmark that compares process CPU time with wall time over at least 200 waits and fails when CPU time exceeds 60 percent of wall time.

- [ ] **Step 2: Verify RED against the busy-spin implementation**

Build and run the regression executable. Confirm either the missing timing interface causes compilation failure or the empirical CPU ratio exceeds the threshold with the current 2 ms spin.

- [ ] **Step 3: Implement high-resolution coarse waiting**

Dynamically resolve `CreateWaitableTimerExW` from `kernel32.dll` and request `CREATE_WAITABLE_TIMER_HIGH_RESOLUTION` using a locally defined flag when old headers do not provide it. Fall back to `CreateWaitableTimerW`, then to `Sleep` if timer setup or waiting fails. Store the timer as an opaque pointer in the header and close it in the destructor.

Calculate a relative 100 ns due time for all but a short final spin. Guard `maxFps == 0`, avoid overflow, skip coarse waiting when already late, and preserve the final QPC anchor.

- [ ] **Step 4: Verify GREEN and measure**

Run the regression executable twice, record its CPU-to-wall ratios at 60, 144, 240, and 480 FPS, and confirm every automated check exits 0. Build both game-engine targets with at most two jobs.

- [ ] **Step 5: Commit the frame pacing optimization**

Inspect timing changes and commit with:

```text
perf(frame-pacing): replace long busy waits

- use high-resolution waitable timers for coarse frame delays
- retain a short spin only for final scheduler jitter
- fall back safely on older Windows timer APIs
- add timing boundary and CPU-usage regression coverage
```

### Task 5: Replay CI dependency

**Files:**
- Modify: `.github/workflows/ci.yml`

**Interfaces:**
- Produces: `replaycheck-generalsmd.needs = [detect-changes, build-generalsmd-vc6]`.

- [ ] **Step 1: Demonstrate the missing dependency**

Run this PowerShell assertion and confirm it exits nonzero on the original scalar dependency:

```powershell
$workflow = Get-Content -Raw .github/workflows/ci.yml
$job = [regex]::Match($workflow, '(?ms)^  replaycheck-generalsmd:\r?\n(?<body>.*?)(?=^  \S|\z)').Groups['body'].Value
if ($job -notmatch 'needs:\s*\[[^\]]*detect-changes[^\]]*build-generalsmd-vc6[^\]]*\]') { exit 1 }
```

- [ ] **Step 2: Add the dependency**

Change the replay job to:

```yaml
needs: [detect-changes, build-generalsmd-vc6]
```

- [ ] **Step 3: Validate and commit**

Re-run the assertion and available workflow syntax validation, inspect the one-line diff, and commit with:

```text
ci(replays): restore change detection dependency
```

### Task 6: Full verification, review, and draft pull request

**Files:**
- Modify only files required to address verified review findings.
- Read: `docs/superpowers/specs/2026-08-13-major-runtime-hardening-design.md`.
- Read: this plan.

**Interfaces:**
- Produces: a pushed `codex/fix-major-runtime-issues` branch and draft PR against upstream `main`.
- Produces: a final P0-P2 review report.

- [ ] **Step 1: Run focused and modern build verification**

Run the regression executable and fresh modern x86 builds for `g_gameengine`, `z_gameengine`, and the test target, sequentially with at most two jobs. Run `git diff --check` and inspect status.

- [ ] **Step 2: Run legacy and replay compatibility verification**

Initialize the `GeneralsReplays` submodule if necessary. Configure and build the available VC6 presets sequentially, then run the repository's headless Zero Hour replay command. If the local VC6 compiler, game assets, or replay executable is unavailable, capture the exact limitation and rely on the restored draft-PR CI jobs rather than claiming the check passed.

- [ ] **Step 3: Request independent code review**

Use the code-review template with base `ae07b29f4f9df14b2266fc40444d3b1b17b1872e`, the current head, this design, and this plan. Resolve every critical or important finding and re-run affected verification.

- [ ] **Step 4: Perform the requested P0-P2 audit**

Review the final diff and surrounding network parsing, wrapper ownership, trigger serialization, condition caching, timing lifetime, and workflow condition paths. Search for newly exposed P0, P1, and P2 issues. Fix findings within the approved scope, add regression coverage first where possible, and document any valid residual findings.

- [ ] **Step 5: Verify the exact final tree**

Run every available focused test and build again after review fixes. Inspect `git status --short`, the full branch diff from the base SHA, and the conventional commit history. Confirm no generated build files or unrelated changes are tracked.

- [ ] **Step 6: Push and open the draft PR**

Push the branch to `origin`, create a draft pull request targeting `TheSuperHackers/GeneralsGameCode:main`, disclose AI-assisted implementation and human-polishing status per `CONTRIBUTING.md`, and include exact verification results, compatibility notes, and the final P0-P2 audit outcome.
