# 08 — Referee and tournaments (external repo)

> Part of [overview.md](overview.md). Depends on: [04](04-bridge-protocol.md), [05](05-match-setup-and-lobby.md), [06](06-headless-and-step.md). **Edited from the external repo.**

## Goal

Fair, repeatable bot-vs-bot matches without trusting the bots — the role BWAPI's TournamentModule and AI Arena's
`rust-arenaclient` proxy play.

## Shape

The referee is a **proxy** between the bot and the game's bridge port. The bot connects to the referee; the referee
connects to the game.

It is **not** the only enforcement layer (a local bot could try the game's port directly):
- the referee launches the game with the match limits as engine flags (`-botapiMode`, `-botapiMaxOrders`, `-botapiNoRaw`,
  `-botapiNoSession`, [04](04-bridge-protocol.md) §Modes and safety), so the engine enforces them itself;
- the referee alone holds the per-match token and connects first; the bot never sees the token or the game's port;
- for public ladders, bot processes run in a sandbox/container without access to the game's network namespace.

| Referee does | How |
|---|---|
| Pins visibility mode (`player` or `camera`) | Rewrites `hello.want`; rejects clients asking for more |
| Time budget | Step mode: per-step budget **plus a time bank** (Lux AI); realtime: nothing to enforce (the game doesn't wait) |
| Forfeit rules | BWAPI-style graded limits as a preset (≥1 step > 10 s, ≥10 > 1 s, ≥320 > 55 ms) |
| APM / order caps | Lowers `max_orders_per_frame`; optional AlphaStar-like caps as a preset |
| Denies escape hatches | `raw` orders, `assist`, session control mid-match |
| Game cap | Max frames → result by score |
| Artifacts | Replay, per-bot logs, result JSON (winner, frames, reason, score stats) |
| Persistent bot storage | `read/` and `write/` dirs between games (BWAPI `bwapi-data` convention) |

## Match runner

- Spawns two headless game instances in a LAN game ([05](05-match-setup-and-lobby.md) scenario 4) or skirmish vs built-in AI.
- Map pool, faction selection, seeds from a config file.
- Round-robin + Elo; results as JSON.
- Runs on Linux (Wine or native) and Windows; container recipe for CI and ladders.

## Done when

- Two example bots play a 10-game series unattended; every replay re-simulates cleanly.
- A bot requesting `full` visibility through the referee is refused.
