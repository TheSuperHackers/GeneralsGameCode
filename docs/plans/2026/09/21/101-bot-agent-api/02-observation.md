# 02 — Observation (Stage 1: read-only)

> Part of [overview.md](overview.md). Depends on: [01](01-engine-seams.md). Wire transport in [04](04-bridge-protocol.md).

## Goal

Every logic frame, the bridge can publish a **snapshot of what one player may legitimately know**, built only from pure
reads, at a hook that fires in normal play, skirmish, network games and headless replay simulation.

## Where

| What | Where | Why |
|---|---|---|
| Hook | End of `GameLogic::update`, just before `m_frame++` (`GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp:3935`) | Logic is settled for frame N; also runs in headless replay, which skips `GameEngine::update` (`Core/GameEngine/Source/Common/ReplaySimulation.cpp:99`) |
| Snapshot writer | New: `Core/GameEngine/Source/Common/BotBridge/BotSnapshot.cpp` | Only file that reads logic for the bot |
| Cost when off | One `if (TheBotBridge)` branch per frame | Opt-in, like NewShoes |

The bot observes **the end of frame N**. Orders it sends in response can execute at the earliest in frame N+1
(skirmish) or N+1+runAhead (network). See [04](04-bridge-protocol.md) §Timing.

## Visibility modes

Fixed at launch, reported in the handshake, never changeable mid-game (BWAPI lesson).

| Mode | Sees | Allowed in |
|---|---|---|
| `player` (**default**) | Own objects; others only where the player's shroud is `CLEAR`; fogged structures as last-seen snapshots marked `stale` | Everything |
| `camera` (xezon's suggestion) | `player` ∩ current tactical view; bot must move the camera via an order | Everything; referee may require it |
| `full` | All objects, no fog | Replay observation, local analysis, single-player training. **Refused in any network game** |

Replay observation defaults to `full` — as in BWAPI, where all flags turn on for replays — and can be switched to `player <n>`.

## Visibility rule (pure)

- Cells: `ThePartitionManager->getShroudStatusForPlayer(botIdx, x, y)` — pure (`PartitionManager.cpp:3121-3137`).
- Objects: **do not call** `Object::getShroudedStatus` (mutates, see [01](01-engine-seams.md) §Unsafe getters).
  Recompute the same rule from cell status: visible if any occupied cell is `CLEAR`; enemy mobile units in fog → hidden;
  mines in fog → hidden; `KINDOF_ALWAYS_VISIBLE` → visible.
- "Ever seen" memory for fogged structures lives **in the bridge** (bot-side memory keyed by opaque ID), not in logic.
- Stealth / detection: follow the same flags the client uses to draw an object for that player. Exact source is an open question.

## Snapshot content (v1)

| Group | Fields | Notes |
|---|---|---|
| Frame | `frame`, `mode`, `player`, `game_state` (`loading/playing/ended`), `result` on end | |
| Self | money, power produced/used, rank, science points, sciences, upgrades, general powers + cooldowns | Only own |
| Objects | `id`, `template`, `owner`, `pos`, `angle`, `health/max`, `status` bits, `veterancy`, `contained_by`, `stale` | Own: + orders, production queue, weapons ready, ammo, `selectable` |
| Map (once) | name, size, heightmap, passability grid, start positions, supply/oil positions | Sent at game start, not per frame |
| Visibility | Shroud grid (`CLEAR/FOGGED/SHROUDED`) | Delta per frame |
| Catalog (once) | Templates: cost, build time, prerequisites, weapons, armor; buttons/commands per template | Lets bots and masks avoid hard-coding INI |
| Events | created, destroyed, damaged, attacked, construction done, upgrade done, EVA-style alerts | Since last read |

**Opaque IDs.** Expose `ObjectID` for own objects; for others, the bridge maps to IDs assigned on first sighting so ID gaps
don't reveal enemy production (NewShoes lesson). Open question whether this is necessary in v1.

## Encodings — one state, several views

| Encoding | For | Shape |
|---|---|---|
| `records` | Scripted bots, LLMs, Jev | Object list + filters/fields (see [04](04-bridge-protocol.md) §Batch reads) |
| `planes` | **ML (CNN/transformer policies)** | Fixed `C × H × W` feature planes on a downsampled grid: own/enemy/neutral occupancy, unit class, health, shroud, height, passability. `float32`/`uint8`, little-endian |
| `entities` | **ML (set/transformer policies)** | Padded `N × F` array + mask; fixed column order published in the catalog |
| `text` | LLMs | Built **in the SDK**, not the engine ([07](07-sdk-gym-mcp.md)) |

`planes` and `entities` are the ML path: fixed shapes, no parsing, copy straight into a tensor. They are computed in the
bridge from the same pure reads as `records` and sent binary ([04](04-bridge-protocol.md) §Encoding).
Legal-action masks come with them ([03](03-commands-and-hardening.md) §Masks).

## Stage 1 steps

1. Snapshot writer + `full` mode, used only from `-headless -replay`.
2. Dump to the bridge (or a file sink) per frame; external client reads it.
3. Add `player` mode; test for fog leaks ([09](09-verification.md)).
4. Same hook live in skirmish (still read-only; no orders yet).

## Later (not v1)

- Rendered-frame channel (world-model / pixel agents). Cheapest form: offline **replay → (frame, action) export**,
  deterministic, no live pixels needed.
- Human-style input channel (camera/mouse/hotkeys) for SIMA-like agents.

## Done when

- Snapshots of a replay in `full` mode are byte-identical across two runs.
- `player` mode never contains an object in a `SHROUDED` cell for that player (fog-leak test).
- Replay CRC unchanged with the hook on versus off (the existing CI job).
