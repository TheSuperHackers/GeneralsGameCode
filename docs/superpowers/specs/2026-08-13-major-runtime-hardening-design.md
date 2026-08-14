# Major Runtime Hardening Design

## Objective

Fix the eight P0-P2 correctness, security, CI, and performance issues found in the repository audit, preserve valid retail-era network and save formats, publish the work as a draft pull request, and perform a second P0-P2 review of the resulting change and its surrounding code.

## Constraints

- Preserve the existing network wire layout and command identifiers.
- Preserve the existing save layout for objects with up to five overlapping trigger areas.
- Keep default retail-compatible builds deterministic for all unaffected scenarios.
- Compile with both the Visual Studio 6 compatibility toolchain and the modern C++20 toolchain.
- Avoid new third-party dependencies.
- Bound work and memory triggered by untrusted UDP traffic.
- Keep normal object memory and trigger processing cheap.
- Keep the pull request reviewable by separating documentation, runtime, performance, and CI changes into conventional commits.

## Considered Approaches

### Minimal line fixes

Add local checks at the reported lines, increase the trigger array to an arbitrary fixed size, and reduce the frame limiter's spin constant. This is small, but it leaves aggregate network memory unbounded, replaces one trigger ceiling with another expensive ceiling, and does not give the frame limiter a reliable high-resolution wait.

### Compatibility-first hardening

Validate network metadata at allocation boundaries, enforce per-command and aggregate limits without changing packet layouts, retain five inline trigger entries with overflow storage, and use a high-resolution Windows wait with a portable fallback. This fixes the root causes while keeping valid legacy data formats stable. This is the selected approach.

### Broad subsystem rewrite

Replace packet decoding with a new fallible parser, redesign file transfer ownership, replace object trigger tracking globally, and introduce a new timing service. This could provide stronger long-term interfaces, but it carries unnecessary deterministic and toolchain risk for the audited defects.

## Design

### 1. Network parsing and wrapped-command memory

Raw packet reads will validate declared payload lengths against the bytes remaining in `NetPacketBuf` before constructing `NetCommandDataChunk`. Wrapper chunks will additionally be limited to one packet payload. Invalid lengths will leave the command with empty data and consume only the available fixed metadata, so parsing cannot allocate from an attacker-controlled 32-bit length.

Wrapped-command assembly will validate the following before allocating a node:

- chunk count, total length, and chunk length are nonzero;
- chunk index is within the declared count;
- chunk payload is within the per-packet limit;
- offset and length fit within the declared total without unsigned overflow;
- the declared total is no larger than 64 MiB;
- the declared chunk count is plausible for the declared total;
- later chunks match the first chunk's player, total length, and chunk count.

The list will track allocated wrapped-command bytes and reject new nodes when accepting them would exceed 256 MiB in aggregate. Removing or resetting a node returns its allocation to the budget. A zero-chunk command can therefore never be complete, and malformed reconstructed data will be discarded if `ConstructNetCommandMsgFromRawData` returns null.

These limits do not modify the network format. The 64 MiB single-transfer limit is well above normal map-transfer payloads while preventing a single datagram from demanding gigabytes. The aggregate limit permits several large legitimate transfers while placing a hard ceiling on connection memory.

### 2. UDP receive work budget

`Transport::doRecv` will process at most `MAX_MESSAGES` datagrams per update and stop reading when no receive-buffer slot remains. The packet counter includes malformed and debug-dropped packets, so an invalid-packet flood cannot keep the main thread in the receive loop indefinitely. Valid packets already buffered continue through the existing processing path.

### 3. Trigger bookkeeping and overflow

The duplicated trigger-info record will move to a shared lightweight storage type used by both Generals and Zero Hour. It will retain five inline records for the retail-common case and allocate a vector only for entries beyond five. The existing signed byte active-count field and serialization order remain unchanged, which caps the representable count at 127 and preserves old saves and replays for the original range.

Compaction will read `isInside` from the source index, copy live records to the destination index, clear their one-frame transition flags, and shrink overflow storage after compaction. New entries will grow overflow storage before indexing it. Queries and serialization will access records through the storage abstraction.

Scenarios that actually exceed five overlapping triggers will intentionally gain correct state and therefore can diverge from retail simulation in that edge case. Existing replay compatibility will be checked by the repository replay corpus.

### 4. Script-condition cache

`evaluatePlayerHasUnitKindInArea` will mirror the adjacent working unit-type condition: cache `-1` or `1` in custom data and store `getFrameObjectCountChanged()` in custom frame. This restores cache hits until a relevant object-count or trigger transition invalidates the result.

### 5. Frame pacing

`FrameRateLimit` will own a Windows waitable timer. It will dynamically request a high-resolution timer where the operating system supports it, avoiding a link-time or header dependency that would break the Visual Studio 6 build. The destructor will close the handle.

Each capped frame will use the timer for the coarse wait and retain only a short final spin to absorb scheduler jitter. If timer creation or scheduling fails, the existing 1 ms multimedia timer setup allows a `Sleep` fallback. Uncapped and already-late frames will not wait. Timing remains anchored to the actual completion counter to avoid catch-up bursts after stalls.

### 6. Replay CI dependency

The replay job will declare both `detect-changes` and `build-generalsmd-vc6` in `needs`, making every context referenced by its condition available. No other workflow behavior changes.

## Verification

Focused regression coverage will be introduced where legacy dependencies permit direct testing:

- malformed network length and wrapper metadata checks;
- zero-chunk and null reconstruction behavior;
- trigger compaction with an exited first record and a live later record;
- trigger overflow beyond five entries;
- frame-wait calculation boundaries and an empirical CPU-time benchmark;
- static validation of the replay job dependency.

The final tree will then run resource-conscious validation sequentially:

1. focused regression targets and synthetic malformed-packet cases;
2. modern x86 Generals and Zero Hour game-engine builds with at most two build jobs;
3. Visual Studio 6 compatible builds when the configured toolchain is available;
4. replay compatibility checks after initializing the replay submodule when required assets and executables are available;
5. workflow syntax validation and a scoped diff review;
6. an independent code-review pass followed by a dedicated P0-P2 audit of the final diff and surrounding paths.

## Pull Request

The branch will be `codex/fix-major-runtime-issues`. It will be pushed to the user's fork and opened as a draft pull request against `TheSuperHackers/GeneralsGameCode:main`. The pull request will disclose AI-assisted implementation as required by `CONTRIBUTING.md`, list compatibility implications, and report exact verification evidence and any environment-limited checks.
