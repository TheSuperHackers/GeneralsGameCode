# 03 — Commands and hardening (Stage 2)

> Part of [overview.md](overview.md). Depends on: [01](01-engine-seams.md), [02](02-observation.md).

## Goal

A bot's orders become the **same `GameMessage`s the UI produces**, for the local player only, after the same checks the
UI would apply. The engine cannot tell a bot from a human → replays, CRC and network sync unchanged.

## Injection point

| Option | Where | Trade-off |
|---|---|---|
| **A (proposed)** | Append to `TheCommandList` right after `TheMessageStream->propagateMessages()` (`GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp:910`), before `TheNetwork->UPDATE()` | Bypasses UI translators (no cursor/selection side effects); same position translator output lands in; network picks it up normally (`Core/GameEngine/Source/GameNetwork/Network.cpp:456-475`) |
| B | `TheMessageStream->appendMessage` | Translators may rewrite or destroy bot messages; couples bot to UI state |

Messages are constructed normally, so they carry the local player index (`Core/GameEngine/Source/Common/MessageStream.cpp:57`).
Only network-range types (`1000–1999`) are accepted from the bot; everything else is rejected at the bridge.

During replay playback injected commands are culled by design (`GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp:1579-1605`) — the bridge refuses orders in replays.

## Order vocabulary

The bot never sends raw selection messages. It sends **orders**; the bridge expands each into the message sequence the UI would:

| Order | Expands to |
|---|---|
| `move {units, to}` | `MSG_CREATE_SELECTED_GROUP(true, units…)`, `MSG_DO_MOVETO(to)` |
| `attack {units, target}` | select, `MSG_DO_ATTACK_OBJECT(target)` |
| `attack_move`, `guard`, `stop`, `scatter`, `force_attack`, waypoints | select + matching message |
| `train {factory, template}` | select factory alone, `MSG_QUEUE_UNIT_CREATE(template, productionID)` |
| `build {dozer, template, at, angle}` | select dozer alone, `MSG_DOZER_CONSTRUCT(template, at, angle)` |
| `upgrade`, `special_power`, `sell`, `repair`, `enter`, `evacuate`, `rally` | select + matching message |
| `raw {type, args}` | **Escape hatch**, off unless the referee allows it; still UI-parity checked |

**Camera is not an order.** It is client-only view state, not a `GameMessage`: live camera movement goes through
`LookAtTranslator` (`Core/GameEngine/Source/GameClient/MessageStream/LookAtXlat.cpp`) and ends in `View::lookAt`
(`Core/GameEngine/Include/GameClient/View.h:143`); the network-range `MSG_SET_REPLAY_CAMERA` is replay-only. So the
bridge exposes a separate **`camera {to, zoom?}` request** ([04](04-bridge-protocol.md) §Requests) that calls
`TheTacticalView->lookAt()` directly, on the client, outside `TheCommandList`. It never enters logic, the network or
the replay, so it cannot desync. In `camera` mode the referee may rate-limit it (a human cannot teleport the camera
every frame either; open question).

Grouping: one batch of orders for a frame is expanded in order; consecutive orders on the same unit set reuse one selection.
Selection lives in logic per player (`GeneralsMD/Code/GameEngine/Include/Common/Player.h:828`), so the bot's selection
and a human's UI selection on the same player collide → **human input is disabled while a bot controls the player**
unless the launch flag `assist` is set ([05](05-match-setup-and-lobby.md) §Human + AI).

## UI parity — check before sending (no CRC impact)

Implemented in the bridge, **client-side**, before any message is created. Rejected orders never reach logic, so these
checks are free of retail-compatibility concerns.

| Check | UI equivalent |
|---|---|
| Every unit is owned by the local player and selectable | Selection translator (`Core/GameEngine/Source/GameClient/MessageStream/SelectionXlat.cpp`) |
| Target object is visible to the player in the current mode | Mouse picking skips shrouded drawables |
| Target location is on the map | Command translator |
| Command is on the unit's command set, prerequisites met, affordable | Control bar button state |
| Build site legal (reuse the placement check the UI uses) | Place-event translator |
| In `camera` mode, units and target are inside the tactical view | UI can only click what is on screen |

Each rejection returns a code + reason per order (batch continues). Late failures — the unit died, target vanished —
come back as events the next frame (SC2 lesson: report both).

## Logic-side hardening (separate, optional PRs)

Client-side parity protects bots using the bridge; it does not protect against a hand-modified client — that is true
today already. Logic-side checks close that hole for everyone:

| Candidate | Where |
|---|---|
| Reject `MSG_DO_ATTACK_OBJECT` / force-attack on a target shrouded for the sender | `Core/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp:1700-1717` |
| Explicit sender ownership in `onQueueUnitCreate`, `onDozerConstruct` | `GameLogicDispatch.cpp:1810-1837`, `:1873-1906` |

These change logic outcomes → guarded like other `RETAIL_COMPATIBLE_CRC` fixes, each its own small PR, each with a replay proving the change. They are **not prerequisites** for the bot API. Maintainers decide (overview open question 4).

## Masks (for ML)

With `planes`/`entities` observations the bridge also returns **legal-action masks**, computed from the same parity checks:

- per own unit: which order types are legal now;
- per factory: which templates are trainable now;
- a coarse build-site legality plane per buildable template on request (expensive → opt-in).

Invalid-action masking is what made RL in µRTS tractable ([00](00-prior-art.md)).

## Steps

1. Order → message expansion for `move`, `attack`, `stop`, `train`, `build`.
2. UI-parity checks + per-order results.
3. Remaining vocabulary; masks.
4. (Optional, separate) logic-side hardening PRs.

## Done when

- A skirmish driven only by bot orders records a replay that `-headless -replay` re-simulates with zero mismatches.
- Every order in the vocabulary has a test that a UI-impossible variant is rejected.
- With the bridge compiled in but disabled, replay CRCs in CI are unchanged.
