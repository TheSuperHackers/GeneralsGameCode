# Bot / Agent API for Zero Hour — Overview

> Spec only. No code is changed by this plan. Relates to #3328.
> Layout (`docs/plans/<YYYY>/<MM>/<DD>/<1NN>-<slug>/`, one file per slice, `status.yml` tracker) follows the
> developerz-ai planning convention — see [Plan layout](#plan-layout). Move or reshape freely.

## TL;DR

- **What:** let an external program (scripted bot, RL agent, LLM, fast decision model) play a Zero Hour match as a
  normal player — it *observes* what that player can see and *issues* the same `GameMessage`s the mouse and keyboard produce.
- **Why:** BWAPI turned Brood War into the standard RTS AI testbed (AIIDE, CoG, SSCAIT, 2026's self-play bot Pluto).
  This repo already has what BWAPI had to reverse-engineer: source, a deterministic lockstep simulation, a headless
  mode and a replay checker in CI.
- **How (shape):**
  1. Engine gets a small, **off-by-default** bridge in `Core/`: read-only snapshots + a command inbox. Nothing runs when disabled.
  2. Bot runs in **its own process**, any language, over **loopback TCP** — same on Windows, on Linux (native or under Wine) and macOS (via Wine/CrossOver today).
  3. Commands enter through the **existing `GameMessage` path** → replays, CRC and network sync are unchanged; a bot can do nothing a player can't.
  4. Observation is **fog-respecting by default**; full information only for replays/analysis, never for a ranked game.
  5. **Batch-first protocol:** one request reads a filtered snapshot or delta; one request submits many orders for one frame. Built for 10 Hz decision models and multi-second LLMs, not just C++ callbacks.
     ML gets fixed-shape binary tensors + legal-action masks; LLMs get text summaries and recipes that teach the game.
  6. **SDKs, Gym env, MCP server, referee** live in a **separate repo** (per maintainer feedback on #3328), not here.
- **Play like a player:** create skirmishes, host/join LAN games — *my AI vs your AI*, each on its own machine —
  with every bot visibly tagged `[BOT]` in its name. One bot per game instance, one port per instance.
- **Order of work:** agree on this spec first — a plan is far cheaper to change than code. Only then the first demo:
  observe replays headlessly → one bot-driven skirmish whose replay `-headless -replay` re-simulates with
  **zero CRC mismatches** in the existing CI job.
- **Ask of maintainers now:** the open questions at the bottom. No code exists and none is proposed in this PR.

## Goal

A stable, versioned, cross-platform contract for driving one Zero Hour player from an external process, which cannot
desync a game, cannot see through the shroud, and adds zero cost when unused.

## What maintainers already said (#3328) → where this plan answers it

| Feedback | Answered in |
|---|---|
| xezon: bot is a separate process; simple pipe to the client; bot code in a separate repo, any language | [04-bridge-protocol.md](04-bridge-protocol.md), [07-sdk-gym-mcp.md](07-sdk-gym-mcp.md) |
| xezon: new files go into `Core`; ZH first, then replicate to Generals | [01-engine-seams.md](01-engine-seams.md) §Placement |
| xezon: bot posts `GameMessage`s + observes → no logic change | [03-commands-and-hardening.md](03-commands-and-hardening.md) |
| xezon: no shroud; maybe camera-bound like a human | [02-observation.md](02-observation.md) §Visibility modes |
| xezon: scripted `AIPlayer` stays separate | Out of scope, below |
| bobtista: which frame does the bot observe, when do actions execute, what if it stalls | [04-bridge-protocol.md](04-bridge-protocol.md) §Timing |
| bobtista: headless gaps (ghost objects, game end) | [06-headless-and-step.md](06-headless-and-step.md) |
| bobtista: getters must not mutate state | [01-engine-seams.md](01-engine-seams.md) §Unsafe getters, [09-verification.md](09-verification.md) |
| bobtista: >1 bot per machine needs changes | [05-match-setup-and-lobby.md](05-match-setup-and-lobby.md) §Scenarios — one bot per game process |
| bobtista: `MSG_DO_ATTACK_OBJECT` on shrouded objects possible in code but not UI → harden | [03-commands-and-hardening.md](03-commands-and-hardening.md) §UI parity |
| bobtista: start with replay observation + minimal client; keep TSH diff minimal; demo | Stage 1 = [02](02-observation.md) + [04](04-bridge-protocol.md) read-only |

## Architecture

```
 ┌──────────────── game process (generalszh.exe, 32-bit) ────────────────┐
 │                                                                        │
 │  GameEngine::update                                                    │
 │    GameClient → MessageStream::propagateMessages ──┐                   │
 │                                   BotBridge inbox ─┤ (2) orders as     │
 │                                                    ▼  GameMessages     │
 │    Network::update ─── TheCommandList ──► GameLogic::update            │
 │                                              processCommandList        │
 │                                              ... object/AI updates     │
 │                                              (1) end of frame N:       │
 │                                                  BotBridge snapshot ───┼──┐
 └────────────────────────────────────────────────────────────────────────┘  │
                         loopback TCP, length-prefixed, versioned             │
 ┌──────────────── bot process (any language, any bitness) ──────────────┐  │
 │  client SDK (C++ / Python / Rust …)  ◄─────────────────────────────────┼──┘
 │    ├─ raw bot (BWAPI-style callbacks)                                  │
 │    ├─ Gymnasium / PettingZoo env  (RL)                                 │
 │    ├─ MCP server  (LLM agents: docs · query · act · step)              │
 │    └─ referee / match runner  (timeouts, capability veto, results)     │
 └────────────────────────────────────────────────────────────────────────┘
```

Three consumer tiers, one wire protocol:

| Tier | Latency budget | Example | Needs |
|---|---|---|---|
| Reflex (scripted / RL) | ≤ 33 ms (one logic frame) | BWAPI-style bot, PPO policy | deltas, masks, step mode |
| Fast decisions | 70–500 ms (~10 Hz) | TypeSafe **Jev** (state in → typed choices out, questions evaluated in parallel) | compact text/JSON state, closed choice lists, batch orders |
| Strategy (LLM) | seconds | Claude / GPT via MCP | summaries, macro orders, docs, paused/step mode |

## Plan files, in execution order

| # | File | Hook |
|---|---|---|
| 00 | [00-prior-art.md](00-prior-art.md) | BWAPI, SC2 API, 0 A.D., Spring, OpenRA-RL, NewShoes (existing ZH agent bridge), Pluto, Jev, LLM-RTS — and the lesson each gives us |
| 01 | [01-engine-seams.md](01-engine-seams.md) | Where commands, frames, fog, headless, CRC and match setup live today (`file:line`); what an observer must never call; platforms |
| 02 | [02-observation.md](02-observation.md) | **Stage 1.** Read-only snapshot, fog-respecting; replay observation first; records / planes / entities encodings |
| 03 | [03-commands-and-hardening.md](03-commands-and-hardening.md) | **Stage 2.** Orders → `GameMessage`; UI-parity validation; action masks; optional logic hardening |
| 04 | [04-bridge-protocol.md](04-bridge-protocol.md) | Loopback TCP, JSON + binary frames, handshake, batch `observe`/`act`, realtime vs step timing, stalls |
| 05 | [05-match-setup-and-lobby.md](05-match-setup-and-lobby.md) | Skirmish / LAN host & join; **my AI vs your AI on separate machines**; human + AI co-pilot; `[BOT]` name tag |
| 06 | [06-headless-and-step.md](06-headless-and-step.md) | Headless live match, uncapped speed, step mode, throughput, bot vs bot locally |
| 07 | [07-sdk-gym-mcp.md](07-sdk-gym-mcp.md) | **Separate repo.** SDKs, Gymnasium/PettingZoo (ML), Jev harness, MCP server, recipes for models that don't know C&C |
| 08 | [08-referee-and-tournament.md](08-referee-and-tournament.md) | **Separate repo.** Referee proxy, time budgets, capability veto, match runner |
| 09 | [09-verification.md](09-verification.md) | Replay round-trip in CI, observer purity, fog-leak, UI parity, cross-platform matrix |

Stages map to #3328: Stage 1 = 01+02+04 (read-only) · Stage 2 = 03 · Stage 3 = 04 full + 05 + 07 · Stage 4 = 06+08. 09 runs across all.

## Done when

- A bot process on Windows, Linux and macOS (latter two via Wine where the game is not native) connects to a running
  Zero Hour skirmish, observes only what its player sees, issues orders, and wins or loses a match.
- That match's replay re-simulates with `-headless -replay` with zero CRC mismatches, in the existing
  `reusable-check-replays.yml` job.
- With the bridge disabled (default), CRC, replays and frame time are identical to a build without it.
- Two people, each with their own bot on their own machine, play one LAN game; both bots are tagged `[BOT]`.
- An MCP client (LLM) with no prior C&C knowledge and a Gym env (ML) both drive a match through the same protocol.
- ZH first; Generals replica of any non-`Core` change follows per `CONTRIBUTING.md`.

## Out of scope (v1)

- Changing `AIPlayer` / `AISkirmishPlayer` (stays separate — xezon).
- Online matchmaking / ranked play for bots — a community policy decision. LAN, direct-connect and private games between
  people who each run their own bot **are** in scope ([05](05-match-setup-and-lobby.md)).
- Pixel / rendered-frame observation and mouse-level input (world-model agents like SIMA / Genie). Noted as a later
  channel in [02](02-observation.md) §Later.
- Several bots inside one game process.

## Risks

| Risk | Mitigation |
|---|---|
| Observer mutates logic state → desync | Snapshot built only from pure reads; banned-call list in [01](01-engine-seams.md); purity test in [09](09-verification.md) |
| Bot sees through shroud (maphack) — the reported Pluto ladder incident is exactly this | Fog-respecting default; full-info only in replay/analysis mode, refused in network games |
| Bot sends commands the UI can't (shrouded attack) | UI-parity checks client-side in the bridge first (no CRC impact); logic-side fixes guarded like other `RETAIL_COMPATIBLE_CRC` changes |
| Slow bot freezes the game | Realtime mode never waits; step mode only in skirmish/headless, with a referee time budget |
| Diff too big to review | Hooks in engine ≈ a few small files; everything else external |

## Open questions for maintainers

1. **Where do orders enter?** Inject into `TheCommandList` right after `propagateMessages()` (`GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp:910`) — bypassing UI translators — or into `TheMessageStream` so translators see them? Plan assumes the former.
2. **Observation hook:** end of `GameLogic::update` just before `m_frame++` (`GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp:3935`) so it also fires in headless replay simulation, which never calls `GameEngine::update`. OK?
3. **Object visibility:** `Object::getShroudedStatus` mutates `PartitionData` caches. Reimplement its rules as a pure read, or accept calling it from logic in fixed order? Plan prefers pure.
4. **UI-parity hardening in logic** (reject attacks on shrouded targets etc.) — separate PRs behind `RETAIL_COMPATIBLE_CRC`, or bridge-side only until retail compatibility can be broken?
5. **Camera-bound mode** as xezon suggested — required, optional, or later?
6. **Wire format:** length-prefixed JSON first (C++98/VC6-friendly, no new deps) with a binary encoding later — acceptable?
7. **Separate repo name/ownership** under the org, or a personal repo until it proves itself?
8. **Build gating:** CMake option (off by default) + command-line flag, or command-line flag only?
9. **Disclosure:** `[BOT]` suffix in the player name (visible to retail clients and in replays) — OK, or another form?
10. **Plan location:** keep `docs/plans/…` in-tree, move to the wiki, or keep only in the external repo?

## Plan layout

This directory uses the planning convention we use at developerz-ai for AI-assisted work: dated dir, `overview.md`
as the map, one independently executable slice per separable area, `status.yml` as the only tracker, `file:line`
references instead of pasted code. Background: <https://github.com/developerz-ai/gold-standards-in-ai>
([workflow-commands.md](https://github.com/developerz-ai/gold-standards-in-ai/blob/main/docs/writing-for-agents/workflow-commands.md)).
If the project prefers a different shape or place, move it — the content matters, not the layout.

All `file:line` references verified against `main` at `241f01926` (2026-09-21).
