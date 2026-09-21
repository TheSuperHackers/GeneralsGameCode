# 05 — Match setup, lobbies, and playing with others

> Part of [overview.md](overview.md). Depends on: [04](04-bridge-protocol.md).

## Goal

A bot can do what a player does before and around a match: start a skirmish, host or join a LAN / direct-connect game,
pick faction, colour, team and map, and play against humans, built-in AI, or **other people's bots on their own machines**.

## Scenarios

| # | Scenario | Machines | Works because |
|---|---|---|---|
| 1 | My bot vs built-in AI (skirmish) | 1 | Local player = bot; `SLOT_*_AI` for opponents (`Core/GameEngine/Include/GameNetwork/GameInfo.h:36-44`) |
| 2 | **Me + my AI vs another player + their AI** | 2+ | Each client has exactly one local player; lockstep sends only commands, never state; each bot sees only what its own client knows |
| 3 | Human + AI assistant on the same player (co-pilot) | 1 | Shared player; `assist` flag keeps human input on ([§Human + AI](#human--ai)) |
| 4 | My bot vs my other bot, locally | 1 | Two game instances in a LAN game on loopback, each with its own bridge **and its own port** (`-botapi 0`, [04](04-bridge-protocol.md) §Transport). Multi-instance must be on ([06](06-headless-and-step.md) §Throughput). To verify: LAN discovery between instances on one host |
| 5 | Bot vs human online | 2+ | Same as 2; community decides where it's allowed ([§Online](#online)) |

Scenario 2 needs **no engine support for multiple bots**: every client is an ordinary client whose local player is driven
by its own bridge. That also answers bobtista's "more than one bot on a machine" concern — the answer is one bot per
game process, scale with processes.

## One agent, several player slots

Examples: one agent plays both allies in a 2v2, or controls both sides for self-play.

| Option | How | Where it works | Cost |
|---|---|---|---|
| **A — one brain, many seats (v1)** | The agent opens one bridge connection per game instance; each instance controls its own local player (scenarios 2 and 4). The SDK presents them as one multi-agent env (PettingZoo Parallel) | Everywhere, including network games; no engine change | One game process per slot |
| B — multi-seat process (later) | One game process; the bridge stamps each order with the target player's index via `GameMessage::friend_setPlayerIndex` (`Core/GameEngine/Include/Common/MessageStream.h:672`); one snapshot per seat, each with its own fog | Skirmish/headless only: over the network the index is replaced by the sender's slot (`Core/GameEngine/Source/GameNetwork/NetCommandMsg.cpp:156`) | Engine work; see below |

Option B would give the biggest self-play throughput (one simulation, two policies), but:
- the dispatcher accepts any valid player index (`Core/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp:363-368`) — *untested* that everything downstream is correct for a non-local sender;
- replays already store a player index per message (`GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp:758-759`), so a multi-seat game should still replay — must be proven by the round-trip test ([09](09-verification.md));
- client-side code assumes one local player (UI, radar, ghost objects), so every seat except the local one is observation-only through the snapshot writer;
- it is a new capability for a bot that no single human has → skirmish/headless only, refused in network games, flagged in `welcome`.

Fairness: one agent on two allied slots shares knowledge between them, like two humans on voice chat. Allowed, but every
controlled slot carries the `[BOT]` tag and the referee can forbid it per match.

## Session API (`session` requests, see [04](04-bridge-protocol.md))

| Action | Engine path |
|---|---|
| `maps.list` | Map cache used by the skirmish/LAN menus |
| `skirmish.create {map, slots:[{type: me|ai_easy|ai_med|ai_brutal|closed, faction, color, team, start}], options}` | `TheSkirmishGameInfo` as filled by `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/SkirmishGameOptionsMenu.cpp:432` |
| `lan.list` / `lan.host {name, map, options}` / `lan.join {game | ip}` | `LANAPI::RequestGameCreate`, `RequestGameJoin`, `RequestGameJoinDirectConnect` (`Core/GameEngine/Include/GameNetwork/LANAPI.h:75-85`) |
| `lobby.set {faction, color, team, start}` / `lobby.ready` / `lobby.chat` | Same messages the LAN lobby menu sends |
| `lobby.start` (host) | Same as the host pressing Start |
| `game.leave` / `game.surrender` | Same as the menu |

Rules:
- Drives the **same code paths as the menus** — no new lobby protocol, no new packets → retail peers see an ordinary player.
- Command-line equivalents for unattended runs: e.g. `-botapi 7777 -botSkirmish <config.json>` (exact flags open),
  reusing the existing `-map` / `-file` start path (`Core/GameEngine/Source/Common/CommandLine.cpp:400-408`,
  `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp:712-730`).
- Headless + session: skirmish only in v1 (the LAN lobby UI is dummied in headless; see [06](06-headless-and-step.md)).

## Disclosure — `[BOT]` in the name

Like GitHub's `name[bot]` accounts: anyone in the lobby, the in-game scoreboard and the replay can see a player is
bot-controlled.

| Rule | Detail |
|---|---|
| Engine appends `[BOT]` to the player name whenever the bridge controls that player; bot cannot remove it | Set once when the bridge attaches, before joining/hosting |
| Name budget | LAN names are max 12 chars (`Core/GameEngine/Include/GameNetwork/LANAPI.h:38`) → base name truncated to 7 + `[BOT]` |
| `assist` mode (human + AI) | Tag `[AI+]` instead (open: exact tag) |
| Why a name tag and not only a lobby flag | Visible to unmodified / retail clients too, and stored in every replay header for free |
| Future | A proper lobby flag once the project has its own lobby server (README "Multiplayer Improvements") |

## Human + AI

- Default: bridge attached → local human game input disabled (like BWAPI `UserInput` off), camera still free.
- `-botAssist`: human keeps full input; bot orders are interleaved. Selection collisions are expected (selection is
  per-player logic state, [03](03-commands-and-hardening.md)); the SDK re-selects before each order, so a bot order may
  change the human's selection — documented, not hidden.

## Online

- Nothing in v1 targets online matchmaking or ranked play.
- Private online games work mechanically the same as LAN (scenario 2).
- Policy (allowed? separate bot ladder?) is a community/maintainer decision; the `[BOT]` tag makes it enforceable socially
  now and technically later.
- `full` visibility is always refused in network games ([04](04-bridge-protocol.md) §Modes).

## Done when

- Scenario 1 from a script: create skirmish, play, get result, exit.
- Scenario 2 on two machines (one Windows, one Linux/Wine): two bots, two owners, one LAN game; both replays re-simulate identically.
- `[BOT]` visible in lobby, in-game, and in the replay player list.
