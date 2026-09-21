# 06 — Headless live matches, step mode, throughput

> Part of [overview.md](overview.md). Depends on: [02](02-observation.md), [03](03-commands-and-hardening.md), [04](04-bridge-protocol.md), [05](05-match-setup-and-lobby.md).

## Goal

Run bot matches with no window, faster than real time, many at once — the training and tournament path (RL needs
millions of frames; BWAPI needed OpenBW for this).

## Today

- `-headless` exists for replay simulation only; that path calls `TheGameLogic->UPDATE()` in a loop
  (`Core/GameEngine/Source/Common/ReplaySimulation.cpp:99`) and never runs `GameEngine::update`, the message stream or the network.
- Many subsystems already have headless dummies ([01](01-engine-seams.md) §Headless).

## Headless live skirmish

| Need | Approach |
|---|---|
| Start a skirmish without the shell | Session config via command line ([05](05-match-setup-and-lobby.md)) → `MSG_NEW_GAME` like `-file` (`GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp:712-730`) |
| Loop | Either (a) normal `GameEngine::update` with headless dummies, or (b) a `ReplaySimulation`-style loop that also drains the bridge and calls `processCommandList`. **Open:** (a) is less new code; (b) is faster |
| Uncapped speed | Skip frame pacing when headless + bridge (`GameEngine.cpp:816-893` accumulator) |
| Game end | Victory conditions still update headless (`GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp:3917`); score screen is skipped (`Core/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp:273`) → bridge reports result + stats and exits with a code |
| Ghost objects (bobtista) | Headless uses `GhostObjectManagerDummy` (`GeneralsMD/Code/GameEngine/Include/GameLogic/GhostObject.h:112-123`). Bridge fog memory is its own ([02](02-observation.md) §Visibility rule), so the dummy is fine — must be confirmed not to affect logic/CRC |
| Other dummies hit in live play (not replay) | Inventory needed: control bar, in-game UI, EVA, radar events. Each found gap = a small headless fix PR, like #2895 / #3030 / #3066 |

## Step mode

- `step {frames: n}` → run n logic frames with the bot's queued orders applied at the first, then return an observation.
- Skirmish/headless only ([04](04-bridge-protocol.md) §Timing).
- Gives deterministic `(obs, action) → obs'` transitions for Gymnasium.
- Seed: the session config fixes the game seed so episodes are reproducible.

## Throughput

| Lever | Note |
|---|---|
| Many processes | One bot per process, one bridge port per process (`-botapi 0`, [04](04-bridge-protocol.md) §Transport); multi-instance exists but is off by default — only `RTS_MULTI_INSTANCE` builds or `-replay` enable it (`Core/GameEngine/Source/GameClient/ClientInstance.cpp:27-31`, `Core/GameEngine/Source/Common/CommandLine.cpp:436`) → `-botapi` enables it the same way; a runner in the external repo spawns N (same idea as `-jobs`, `ReplaySimulation.cpp:130-201`) |
| Observation cost | Only compute encodings a client subscribed to; planes at reduced resolution |
| Frame skip | `every_n_frames` in `subscribe` (PySC2 `step_mul`, Pluto: one step / 6 frames) |
| Measure | Frames/s per core for headless skirmish with a no-op bot, published as a baseline |

## Bot vs bot locally

One process per bot, LAN game over loopback ([05](05-match-setup-and-lobby.md) scenario 4). No change to the
single-local-player model. **To verify:** LAN discovery and hosting between two headless instances on one host.

## Done when

- `generalszh.exe -headless -botapi 7777 -botSkirmish cfg.json` plays a full match against built-in AI to a result, no window.
- Step mode with a fixed seed produces identical observations across two runs.
- The replay of that match passes `-headless -replay` in CI.
