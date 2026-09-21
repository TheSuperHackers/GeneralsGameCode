# 07 — SDKs, Gym env, MCP server, recipes (external repo)

> Part of [overview.md](overview.md). Depends on: [04](04-bridge-protocol.md). **Edited from the external repo, not this one** (xezon on #3328).

## Goal

Make the protocol easy for three very different consumers — ML training code, fast decision models, and LLMs that have
never heard of Command & Conquer — without adding anything to the engine.

## Layout (external repo)

```
protocol/            JSON Schemas + BIN layouts — the contract; versioned zh-bot/<major>
clients/python/      stdlib-only client; numpy optional
clients/rust/        client crate
clients/cpp/         header-only client (C++17)
gym/                 Gymnasium + PettingZoo envs, vector env, wrappers
mcp/                 MCP server (LLM agents)
recipes/             markdown recipes (served by mcp, readable by humans)
referee/             match runner / proxy (see 08)
examples/            reference bots: scripted rush, random-legal, RL baseline, LLM, Jev harness
```

## ML path (not LLMs)

| Need | Provided by |
|---|---|
| Fixed-shape tensors | `planes` (C×H×W) and `entities` (N×F + mask) in BIN frames → `np.frombuffer`, no parsing ([02](02-observation.md) §Encodings) |
| Legal-action masks | Per unit / per factory masks ([03](03-commands-and-hardening.md) §Masks) |
| Gym API | `ZHEnv(Gymnasium)`: `reset(seed, options={map, faction, opponent})`, `step(action) → obs, reward, terminated, truncated, info` |
| Multi-agent | PettingZoo Parallel env for scenario 4 (two local processes) |
| Action space | `Dict`/`MultiDiscrete`: order type × unit slot × target (grid cell or entity slot) — masked |
| Reward | Pluggable: win/loss (default), plus shaped helpers (resources, army value, damage) from score stats in `info` |
| Throughput | `VectorEnv` over N headless processes; step mode; frame skip ([06](06-headless-and-step.md)) |
| Imitation data | Replay → `(obs, action)` dataset exporter using replay observation in `full` or `player` mode (Stage 1 is useful on its own here) |

## Fast decision models (Jev and similar)

Jev takes a state and many typed questions (`choice` ≤255 options, `score`, `noul`) and answers them in parallel in
70–500 ms. The harness in `examples/jev/`:

1. `observe` (records, filtered) → compact state text.
2. Build one question per idle unit/group: "target?" (`choice` over visible enemies + "hold"), "retreat?" (`noul`), per factory "train what?" (`choice` over mask-legal templates).
3. One Jev call → all answers → one `act` batch.
4. Loop at ~10 Hz in realtime mode; an LLM (below) sets strategy every few seconds.

That is why the protocol is batch-first and publishes legal choices: the candidate lists *are* Jev's options.

## LLM path — MCP server

Few parameterized tools, constant size regardless of how many orders/templates exist
(pattern: <https://github.com/developerz-ai/gold-standards-in-ai/blob/main/docs/ai-agents/mcp-docs-for-agents.md>):

| Tool | Does |
|---|---|
| `docs({q?, uri?})` | Recipes and reference. `docs({})` = the map. Search inlines the best hit; a miss returns nearest matches, never empty |
| `list_resources({})` | Object kinds, order types, map info, what this match allows |
| `describe_resource({resource})` | Fields/enums for one order or object kind, loaded lazily |
| `query({filters, fields, area?, since_frame?, format: "text"|"json"})` | Batch read; `text` = summaries sized for context windows |
| `act({orders:[…]})` | Batch orders; per-order result; every error carries a `docs://` URI to the recipe that fixes it |
| `session({…})` | Create/join games ([05](05-match-setup-and-lobby.md)) |
| `step({frames})` / `wait({until_event?, max_ms})` | Step mode / realtime wake-up on events |

Text summaries are built here, not in the engine: counts by type, base status, visible threats, idle units, economy — a
"chain of summarization" as in TextStarCraft II.

## Recipes — for models that know nothing about C&C

The model may never have seen Generals. Recipes teach the game as well as the API. Plain markdown, one file per recipe,
served verbatim by `docs`, mirrored as MCP resources.

| Kind | Examples | Source |
|---|---|---|
| **Game primer** | `docs://game/basics` (factions USA/China/GLA, generals, money from supply, power, tech tree, win condition) · `docs://game/<faction>` | Hand-written, short |
| **Facts** | `docs://ref/templates/<name>` (cost, build time, prereqs, weapons, what it counters) | **Generated** from the `catalog` request (INI data), so never stale |
| **How-to recipes** | `docs://recipes/start-a-skirmish`, `…/set-up-economy`, `…/build-a-base-<faction>`, `…/scout-the-map`, `…/attack-a-target`, `…/defend-a-base`, `…/use-a-general-power`, `…/retreat-and-repair` | Hand-written; each with *use when*, literal `act` calls, `→` gives, one branch per step, *done when*, tips |
| **Gotchas** | "you can only target what you can see", "one dozer builds one thing at a time", "orders execute next frame (or after run-ahead online)", "low power slows production" | Hand-written; linked from error codes |

Example shape:

```
docs://recipes/set-up-economy-usa — get money flowing as USA

Use when the game just started and you have a Command Center and a Dozer.
Money comes from supply docks via supply trucks/chinooks; power plants keep production fast.

1. query({filters:{owner:"me", kind:"dozer"}, fields:["id","pos"]})  → dozer.id
2. act({orders:[{order:"build", dozer:"<dozer.id>", template:"AmericaPowerPlant", at:"<near base>"}]})
   error BUILD_SITE_BLOCKED? pick another spot 60+ units away, retry once.
3. act({orders:[{order:"build", dozer:"<dozer.id>", template:"AmericaSupplyCenter", at:"<near nearest supply pile>"}]})
   → the Supply Center spawns a Chinook that harvests automatically.

Done when query({filters:{owner:"me", template:"AmericaSupplyCenter"}}) returns one object.

Tips
- Build power first: production is slow while power is negative.
- Never send the only Dozer across the map.
```

Template names above are illustrative; generated reference pages hold the real names.

CI in the external repo replays every fenced call in every recipe against a headless skirmish, and link-checks every `docs://` URI.

## Done when

- `pip install` + 20 lines → a random-legal bot plays a full skirmish via Gym.
- An MCP client (e.g. Claude Code) with no C&C knowledge, given only the MCP server, builds a base and wins against Easy AI using recipes.
- Jev example runs at ≥5 decisions/s in realtime mode.
