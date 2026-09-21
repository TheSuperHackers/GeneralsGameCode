# 00 — Prior art

> Part of [overview.md](overview.md). Depends on: none. Research only; informs every other slice.

Facts as of 2026-09-21. Each entry: transport · timing · observation · actions · fairness → **lesson for us**.

## BWAPI — StarCraft: Brood War (2009–)

Source read at `bwapi/bwapi@d727fed`.

| Aspect | How |
|---|---|
| Hook | DLL injected into the closed 1.16.1 binary; `CodePatch.cpp` patches the frame loop (`_nextFrameHook` → `GameImpl::update()`) and `QueueCommand` (`CommandFilter`) |
| Orders | Built as the game's own network command packets via `QueueGameCommand` → go through normal lockstep turns (hence *latency frames*: SP 2, LAN 5, B.net 14–24). `UnitImpl::issueCommand` → `canIssueCommand` legality check → `CommandOptimizer` → select + execute |
| Timing | `AIModule::onFrame` runs **inside** the frame → a slow bot stalls the game. The out-of-process client is also blocking: server writes a token to `\\.\pipe\bwapi_pipe_<pid>`, waits in `ReadFile` for the client's reply |
| Bridge | `BWAPIClient`: Windows shared memory (fixed `GameData` struct: `units[10000]`, `commands[20000]`, per-tile grids) + named pipe as sync token. Client must match `CLIENT_VERSION` exactly → ABI lock-in |
| Fog | `Flag::CompleteMapInformation` and `Flag::UserInput` default **off**; can only be enabled in `onStart`; **all flags auto-on for replays** |
| Referee | `TournamentModule` DLL vetoes actions via `onAction` (flags, pause, speed, GUI, frame skip) |
| Rules (AIIDE/CoG) | Lose if ≥1 frame > 10 s, ≥10 frames > 1 s, or ≥320 frames > 55 ms. Game cap ~86k frames. `bwapi-data/read` / `write` for learning between games. No APM cap |
| Headless | OpenBW (open reimplementation) + its BWAPI fork run headless on Linux |

**Lessons:** fog-respecting and no-human-input by default, locked after start · a referee layer vetoes fairness settings ·
don't copy the raw shared-memory struct with an exact version match — use a versioned schema · don't block the frame on the bot by default.

## Pluto — the 2026 BW self-play bot

- One neural network plays the whole game, trained by self-play RL (PPO, no imitation) — <https://github.com/tscmoo/pluto>.
- **Won the 2026 IEEE CoG StarCraft AI competition, 4–1 over PurpleWave in the final** — <https://davechurchill.ca/starcraft/cog/results/2026>.
- Architecture: 32-bit BWAPI module (`pluto.dll`) launches a **separate 64-bit inference process** (`pluto_infer.exe`);
  README says stdio pipes, the CoG build notes say shared memory. Paces itself by inference (one step every 6 frames).
- Reported wins over top human players (Larva, Iris, Ret, Paralyze) came from an **unauthorized** copy on the
  StarCraft: Remastered ladder, which reportedly also ran a **maphack** — secondary sources only
  (<https://www.dexerto.com/gaming/ai-bot-destroys-top-starcraft-pros-after-invading-ladder-and-using-absurd-strategies-3410863/>).

**Lessons:** ZH is 32-bit too → ML inference must be out-of-process anyway · a strong bot plus hidden-information access
is exactly the abuse case, so fog enforcement belongs in the engine side of the bridge, not in the bot.

## StarCraft II API — s2client-proto / PySC2 / python-sc2 / AI Arena

| Aspect | How |
|---|---|
| Transport | Protobuf over WebSocket (`-listen 127.0.0.1 -port N`, path `/sc2api`); responses in request order; pipelining encouraged |
| Timing | **Step mode**: sim advances only when all players send `RequestStep{count}`. **Realtime mode**: 22.4 loops/s, no step |
| Observation | Raw / feature layers / rendered — same info, different encodings. `disable_fog` off by default; fogged units come back as `DisplayType Snapshot` |
| Actions | `ActionRawUnitCommand{ability_id, target, unit_tags[], queue}`; errors reported immediately *and* later in `action_errors` |
| Fairness | AI Arena's `rust-arenaclient` proxy sits between bot and game: strips debug requests, per-request `max_frame_time`, ties at max game time, hides the random race, saves the replay |

**Lessons:** one protocol, two timing modes · report order failures twice (validation + execution) · fairness enforced by a
proxy/referee that the bot cannot configure · multiple observation encodings from one state.

## 0 A.D. — RL interface (open-source RTS, closest in spirit)

- `pyrogenesis --rl-interface=127.0.0.1:6000` → HTTP server: `/reset`, `/step` (lines of `playerID;commandJSON`), `/evaluate` (runs JS), `/templates`.
- Sim advances **only on `/step`**, one 200 ms turn; `--autostart-nonvisual` for headless.
- Returns the full (apparently unfogged) state; any player ID can be commanded — <https://github.com/0ad/0ad/blob/master/source/rlinterface/RLInterface.cpp>.

**Lessons:** a small local step/reset server is easy to adopt · but "whole state + command any player + eval" is a research
tool, not a fair interface — keep god-mode a separate, explicit capability.

## Spring / Recoil engine

In-process Skirmish AI shared libraries (`init` / `handleEvent` / `release`), C interface wrapped for Java/Python.
Explicit cheat API (`Cheats_isEnabled`, `isOnlyPassive`). **Lesson:** make cheating a declared, queryable capability.

## Gym-style interfaces

- **Gymnasium** `reset(seed) → (obs, info)`, `step(a) → (obs, reward, terminated, truncated, info)`.
- **PettingZoo** AEC vs Parallel multi-agent; action masks in obs/info.
- **Gym-µRTS**: invalid-action masking is central — the key to RL in RTS at all (<https://arxiv.org/abs/2105.13807>).
- **Lux AI S3**: JSON over stdin/stdout, hidden values = `-1` + `sensor_mask`, per-step limit **plus a time bank**.
- **AlphaStar**: human-like limits (camera view, ≤22 actions / 5 s, ~110 ms reaction) negotiated with a pro.

**Lessons:** ship legal-action masks · time bank is kinder than a hard per-frame cutoff · human-like limits are a referee setting, not an engine rule.

## NewShoes / Generals.zone — an existing Zero Hour agent bridge

The most relevant prior art. EA ZH source compiled to WebAssembly, plus an optional Go bridge —
<https://github.com/Agusx1211/NewShoes> (umbrella issue #75), README mirror
<https://github.com/playerx/GeneralsZH/blob/main/AgentBridge/README.md>.

- Browser game connects **out** to the bridge; bridge serves REST on `127.0.0.1:18888`, Bearer token, protocol `cnc-agent/1`.
- Engine stays authoritative for snapshots, fog/stealth filtering, stable **opaque IDs** (assigned only once observable,
  so counts don't leak), terminal results. Sugar (diffs, SSE events, filters) lives in Go.
- Modes fixed at launch: `global` (whole map, fog-safe) and `camera` (observation *and* orders limited to the tactical view).
- Orders: advertised command names, engine validates prerequisites/funds/placement, then posts the real game message;
  group orders all-or-nothing unless `bestEffort`.
- Strictly opt-in, zero per-frame cost when off.

**Lessons:** validates the whole shape on this exact engine · reuse its vocabulary where sensible and talk to its author
before inventing a second, incompatible protocol · unverified whether it supports lockstep stepping or native headless.

## LLM agents in RTS

| Project | Takeaway |
|---|---|
| TextStarCraft II (<https://arxiv.org/abs/2312.11865>) | Text observations + macro actions; beat built-in Harder AI but ~7 h per game → needs paused/step mode |
| LLM-PySC2 (<https://arxiv.org/abs/2411.05348>) | Async queries keep latency flat with many agents; invalid/hallucinated actions must be reported back |
| SwarmBrain (<https://arxiv.org/abs/2401.17749>) | LLM strategist + fast rule-based reflex layer |
| OpenRA-RL (<https://github.com/kunrenzhilu/OpenRA-RL>) | gRPC → Gymnasium → **MCP** (~50 tools). Observation channel holds only the newest state; actions buffered; game never waits → a 40 ms bot and a 2 s LLM drive the same engine |
| RTSGameBench (<https://arxiv.org/abs/2606.18950>) | Pauses between steps to measure decision quality, not reaction time |

## Jev — fast typed decisions (TypeSafe AI, released 2026-09-21)

- "System One model": **unstructured state in, typed probabilistic decisions out** — `choice` (≤255 options), `score`,
  `noul` (yes/no); every answer carries probabilities. 70–500 ms end-to-end. Questions in one request are evaluated
  **in parallel**, so adding questions barely changes latency. Doom demo at ~10 queries/s on structured (non-pixel) state.
  <https://typesafe.ai/blog/introducing-system-one-models-and-jev> · <https://www.langchain.com/blog/building-a-harness-with-jev>
- Intended split: LLM for open-ended reasoning, Jev for fast structured decisions along the way.

**What it needs from us:** a compact JSON/text state · closed candidate lists (units, targets, build sites) small enough for a
`choice` · batch orders so N parallel answers become one submission · a ~10 Hz loop that doesn't block the game.
That is the middle tier in [overview.md](overview.md) §Architecture, and why [04](04-bridge-protocol.md) is batch-first.

## World-model / pixel agents (later, not v1)

V-JEPA 2-AC, Genie 3, SIMA 2 consume rendered frames + keyboard/mouse. Cheapest support here is not live pixels but
**replay → (frame, action) export**, since the simulation is deterministic. Noted in [02](02-observation.md) §Later.

## Synthesis → decisions taken in this plan

| Decision | From |
|---|---|
| Orders only as `GameMessage`, after UI-parity checks | BWAPI `canIssueCommand`, NewShoes, #3328 |
| Fog-respecting default; full info only for replays/analysis | BWAPI flags, SC2 `disable_fog`, Pluto maphack incident |
| Out-of-process, versioned schema, loopback | BWAPIClient ABI lock-in, Pluto 32/64-bit split, SC2, xezon |
| Realtime (never waits) + step (lockstep) modes | SC2, OpenRA-RL, 0 A.D. |
| Batch reads/writes, deltas, action masks | µRTS, Jev, LLM-PySC2, BWAPI round-trip cost over a bridge |
| Referee outside the engine | TournamentModule, rust-arenaclient |
| MCP + Gym as SDK layers, not engine features | OpenRA-RL, xezon |
