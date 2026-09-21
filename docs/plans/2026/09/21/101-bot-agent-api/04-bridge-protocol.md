# 04 — Bridge protocol

> Part of [overview.md](overview.md). Depends on: [02](02-observation.md), [03](03-commands-and-hardening.md).
> The engine side is in this repo; the protocol spec file and all clients live in the external repo ([07](07-sdk-gym-mcp.md)).

## Goal

One small, versioned, cross-platform protocol that serves a 33 ms reflex bot, a 10 Hz decision model and a
multi-second LLM without changes on the engine side.

## Transport

| Choice | Why |
|---|---|
| **TCP on `127.0.0.1`**, port from `-botapi <port>` | Same code on Windows, Linux (native or Wine) and macOS (Wine/CrossOver); crosses the Wine boundary to native bots; Winsock already linked (`Core/GameEngine/Source/GameNetwork/Transport.cpp:94`) |
| Game is the **server**; one client at a time | Mirrors SC2 `-listen`; the bot or a launcher can connect after start |
| **One port per game instance**; `-botapi 0` = OS picks a free port, reported on stdout (`BOTAPI_PORT=<n>`) and in a port file in the instance's user dir | Many instances per host (training, bot vs bot) without collisions; the runner reads the port back instead of pre-allocating |
| **Every connection authenticates**, loopback included: first frame must carry the token. Token from `-botapiToken <secret>`, or, if absent, the engine generates a random 128-bit token and writes it next to the port file (readable by the launching user only) | Loopback is not a trust boundary: any local process can find the port and race the intended client |
| First authenticated client owns the bridge for the whole match; later connections are refused; a dropped client may reconnect with the same token | No connection race, no hijack mid-match |
| Non-loopback bind only with `-botapiBind <addr>` | Remote bots (e.g. a GPU box) are possible but never accidental |
| Non-blocking socket, polled once per engine tick | No threads in the engine; no locks around logic |

Rejected: shared memory (BWAPI-style — OS-specific, doesn't cross Wine, fixed ABI); named pipes (Windows-only semantics);
HTTP server in engine (heavier; can be added by the SDK as a proxy, like NewShoes' Go bridge).

## Seat scoping

**One connection = one token = one seat.** The seat is the player that game instance controls; it is fixed by the engine
in `welcome` (`seat: {player_index, name, team}`) and cannot be chosen or changed by the client.

- Every `observe` is that seat's view (its fog, its money, its production); every `act` is that seat's orders.
- A client cannot address another seat — there is no `player` field in `act` or `observe` to get wrong or abuse.
- Multi-seat (option B in [05](05-match-setup-and-lobby.md)) keeps the rule: one token per seat, even when the seats share a process.
- An agent driving several seats holds several connections; combining their knowledge happens in the agent, visibly, never in the engine.

## Framing and encoding

- Frame = `uint32 length (LE) | uint8 kind | payload`, where **`length` = number of bytes after the length field**
  (`1 + payload bytes`). `length = 0` is invalid.
- **Receive limits:** the engine rejects frames above `max_frame_bytes` (default 1 MiB inbound — orders are small) and
  advertises it in `welcome`; clients declare their own limit in `hello` (default 64 MiB, since observations with planes
  are larger). A frame over the limit, an unknown `kind`, or invalid UTF-8/JSON → error frame, then close. Never allocate
  from an unchecked length.
- `kind`: `1 = JSON`, `2 = BIN`, `3 = ERROR`; other values reserved.
- `kind = JSON` (UTF-8) for control, orders, records, events — debuggable with any language, written with a small
  hand-rolled writer (C++98, no new dependency).
- `kind = BIN` for bulk numeric data (`planes`, `entities`, masks, shroud grids): header (`dtype`, `shape`, name) + raw
  little-endian array. Zero-copy into NumPy/PyTorch (`np.frombuffer`) — the ML fast path.
- Schema: one JSON Schema file per message in the external repo is the contract; engine + SDKs are tested against it.

## Handshake

```
C→S  hello {protocol: "zh-bot/1", token: "<secret>", client: "<name/version>", max_frame_bytes, want: {mode, encodings, timing}}
S→C  welcome {protocol, game_version, player, mode, timing, capabilities[], limits{max_orders_per_frame, …}}
```

The server answers with what it **grants**, not what was asked (referee/launch flags win). Mode and timing never change
after `welcome`. Unknown major version → error + close.

## Requests

| Request | Does |
|---|---|
| `observe {encodings, filters?, fields?, since_frame?}` | **Batch read.** One snapshot or delta for the latest frame. Filters: owner, template/kind, area (rect/circle), status. `fields` projects columns. `since_frame` → only changes |
| `catalog {}` / `map {}` | Once per game: templates, commands, feature-plane layout; terrain, passability, start spots |
| `act {frame_hint?, orders:[…]}` | **Batch write.** Up to `max_orders_per_frame`; expanded and applied in one engine tick; result per order `{ok}` or `{error, reason}` |
| `step {frames}` | Step mode only: advance N frames, reply with an observation (see Timing) |
| `camera {to, zoom?}` | Move the tactical view (client-only, never in logic/replay; see [03](03-commands-and-hardening.md) §Order vocabulary) |
| `subscribe {events, every_n_frames?, encodings?}` | Server pushes observations/events; the default for realtime bots |
| `session {…}` | Lobby / match setup ([05](05-match-setup-and-lobby.md)) |
| `bye {}` | Leave; in a match = surrender unless the referee says otherwise |

All requests carry an `id`; responses echo it. Clients may pipeline.

## Timing — answers to bobtista's questions

| Question | Realtime mode (default) | Step mode |
|---|---|---|
| Which frame does the bot observe? | End of the latest completed frame N ([02](02-observation.md) §Where) | Same |
| When do its actions execute? | Skirmish: frame N+1 if received before the next logic tick, else later. Network: `N + runAhead` (`Core/GameEngine/Source/GameNetwork/Network.cpp:477-484`), exactly like a human's | Next frame of the `step` |
| What if it stalls? | **Nothing** — the game never waits. The latest observation replaces the previous one (queue depth 1, as in OpenRA-RL); orders apply when they arrive; a missed frame means no orders | Game waits, up to the referee's budget ([08](08-referee-and-tournament.md)); then continues as a no-op frame or the bot forfeits |
| Where allowed? | Everywhere | Skirmish and headless only; **never** in network games (would stall other players) |

Delivered frame numbers let the bot measure its own lag. `limits` in `welcome` include the current run-ahead.

## Batch-first design (why)

BWAPI's API is one call per unit per frame, cheap only because it is in-process. Over a process boundary, and certainly
for a model with 100 ms–seconds per decision, per-unit calls are the bottleneck. So:

- one `observe` returns everything the bot asked for, filtered and projected;
- deltas (`since_frame`) keep 30 Hz subscriptions small;
- one `act` carries all orders for a tick — matches Jev's "many questions in parallel, one answer set"
  and an RL policy's per-step action vector;
- a limit per frame is published, not hidden (referee may lower it for APM fairness).

## Modes and safety

| Rule | Why |
|---|---|
| Bridge is off unless `-botapi` is passed; compiled in behind a CMake option (open question 8) | Zero cost and zero surface by default |
| **Fairness limits are enforced in the engine**, from launch flags, not only by the referee: `-botapiMode player|camera`, `-botapiMaxOrders <n>`, `-botapiNoRaw`, `-botapiNoSession`. `hello` can only ask for *less* | A bot that reaches the port directly still cannot get more than the match allows; the referee is defence in depth, not the only gate |
| `full` visibility refused if the game is a network game | No maphack in multiplayer — the Pluto ladder incident |
| Orders only for the local player | Engine already enforces per-slot identity over the network (`Core/GameEngine/Source/GameNetwork/NetCommandMsg.cpp:156`) |
| Network games: bot flag announced + `[BOT]` name tag | [05](05-match-setup-and-lobby.md) |
| Bridge never touches logic state except by appending `GameMessage`s | No new desync surface |

## Done when

- A Python client (≈100 lines, stdlib only) connects on Windows and on Linux to a Wine-run game, observes and acts.
- Protocol schema in the external repo; engine responses validate against it in CI.
- A stalled client does not change frame time in realtime mode (measured).
