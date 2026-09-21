# 09 — Verification

> Part of [overview.md](overview.md). Runs alongside every stage.

## Tests in this repo

| Test | Proves | How |
|---|---|---|
| **Replay round-trip** | Bot games are ordinary games | Record a bot-driven skirmish, add the `.rep` to `GeneralsReplays/GeneralsZH/1.04`, run the existing `.github/workflows/reusable-check-replays.yml` (`-jobs 4 -headless -replay`) → zero mismatches |
| **Off = identical** | Zero cost when disabled | Existing replay CI job on a build with the bridge compiled in, flag off → CRCs unchanged; frame time within noise |
| **Observer purity** | Snapshot writer never mutates logic | Replay with snapshots every frame in `full` mode vs without → identical CRC at **every** frame (`-ReplayCRCInterval 1`, `Core/GameEngine/Source/Common/CommandLine.cpp:327-335`, debug CRC builds) |
| **Banned calls** | No mutating getter sneaks in later | Grep-based CI check on `BotBridge/` for the list in [01](01-engine-seams.md) §Unsafe getters (`getShroudedStatus`, `GameLogicRandomValue`, `createGroup`, …) |
| **Fog leak** | `player` mode never exposes shrouded objects | Replay in `player <n>` mode; for every frame, every non-own object in the snapshot sits on a cell `CLEAR` for n (or is a `stale` structure previously seen) |
| **UI parity** | UI-impossible orders are rejected | One case per order type: unowned unit, shrouded target, off-map, unaffordable, illegal build site |
| **Determinism of step mode** | Reproducible episodes | Same seed + same actions → identical observations |

## Tests in the external repo

- Protocol schema conformance of real engine output (a pinned game build in CI).
- Every recipe's fenced calls replayed against a headless skirmish; every `docs://` link resolves.
- Example bots finish a match.

## Cross-platform matrix

| Game binary | Bot runs on | Transport |
|---|---|---|
| Windows native | Windows | loopback TCP |
| Windows `.exe` under Wine on Linux | Linux native (Python/Rust) | loopback TCP across Wine |
| Native Linux (when #2088 lands) | Linux | loopback TCP |
| Wine/CrossOver on macOS | macOS native | loopback TCP |

## Done when

All rows above are automated or have a documented manual run with results linked from `status.yml` `evidence`.
