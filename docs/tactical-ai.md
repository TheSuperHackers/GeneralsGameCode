# Tactical AI

## What it is

A new `SLOT_TACTICAL_AI` lobby option that sits alongside Easy /
Medium / Brutal. Its combat strength matches Brutal
(`DIFFICULTY_HARD`), but it pulls in a stack of zulu-only behaviors
that vanilla AIs don't get:

- A 10-second idle-army-commit sweep that pushes built-but-unused
  units toward enemy buildings instead of stockpiling them at the
  base.
- Following ally beacons captioned `!attack` as an attack-target
  directive.
- Ally-visible chat for game-time milestones, attack commits, and
  distress under base damage. Includes a Syn/Dan easter egg.
- USA-family-specific build-order changes: power plant first, no
  parallel reactor builds via two dozers.
- A `SkirmishScripts.scb` override that swaps USA Hard combat
  compositions (Crusader / Paladin / Stealth Fighter / Microwave to
  Humvee + Missile Defender groups, with a Patriot-fallback tier).

The `PLAYER_IS_TACTICAL_AI` script condition (id 109) lets `.scb`
authors branch on this without hardcoding lobby state.

## Why

Vanilla skirmish AI has decent base-building and unit production but
weak strategic commitment: it parks finished units near its base
and forgets to attack with them. The Tactical AI slot is the umbrella
flag that lets us ship a much more aggressive AI without breaking
vanilla / Brutal lobbies that players expect to behave exactly like
retail.

The flag-gating discipline is what makes the rest of this safe.
Every behavior added under "Tactical AI" must early-return on
`!isTacticalAI()` so retail and the existing Easy / Medium / Brutal
AIs continue to behave bit-for-bit identically to upstream.

## Code surface

Plumbing (PR #5 — `4666cdab1`):

- `SLOT_TACTICAL_AI` appended to the `SlotState` enum in
  `Core/GameEngine/Include/GameNetwork/GameInfo.h`.
- `'T'` letter added to `GameInfo` string serialization so lobby
  strings round-trip the new slot type.
- `TheKey_playerIsTacticalAI` dict key (parallel to
  `skirmishDifficulty`) propagates the flag from `GameSlot` to
  `AIPlayer` via `Player::initFromDict`.
- `AIPlayer::xfer` bumped to version 2 so save-games preserve the
  flag.
- Skirmish / LAN / WOL lobby menus get a new combo entry. LAN/WOL
  use positional `combo->state` mapping so two new translation
  helpers (`slotStateFromLobbyComboPos`, `lobbyComboPosFromSlotState`)
  bridge around the `SLOT_PLAYER=5` reservation rather than
  refactoring those menus.
- `Recorder.cpp` writes a `ZULU` magic + feature-version block after
  `maxFPS` in the replay header so older Zulu binaries can play
  newer replays with feature gating, while still hitting the
  existing exeCRC rejection against non-Zulu binaries.

Behaviors:

- Idle-army commit (PR #2 — `6438712b7`, `44bdf6570`): per-skirmish
  AI 10s sweep in `AISkirmishPlayer`. `commitIdleArmy` early-returns
  unless `isTacticalAI()`.
- Beacon directive (PR #6 — `16a429627`): `Player::getAttackBeacon()`
  + `AISkirmishPlayer::commitIdleArmy` consult the most recent ally
  beacon caption `!attack` (case-insensitive) and use the beacon
  position as the attack-target override.
- Ally chatter (PR #13 — `2ad46514b`): time / health-ring-buffer
  hooks inside the deterministic sim tick that fire UI-only messages
  via `TheInGameUI->messageColor`. Phase milestones at 5 / 15 min,
  attack-commit map quadrant, distress on >=2 structure losses
  inside a 10s window, Syn/Dan easter egg if target's display name
  is `Syn`.
- USA build order (PR #16 — `f8c85c433`): hook in
  `AISkirmishPlayer::processBaseBuilding` that pre-scans the build
  list and forces the first `PowerPlant` to `AutomaticallyBuild=Yes`,
  and prevents two dozers from both starting reactors in parallel.
- `PLAYER_IS_TACTICAL_AI` (PR #15 — `3005bce33`): new script
  condition id 109, resolves a SIDE parameter and returns
  `Player::isTacticalAIPlayer()`. Used by `SkirmishScripts.scb` as
  an `Init TAI Flag` setup script.
- USA Hard wave swap (PR #15 — `30a8f0b9d`, PR #16 — `798e482f5`):
  ships a modified `SkirmishScripts.scb` that runs a clone of the
  existing USA Hard combat teams behind a flag-gate. Includes a
  Patriot-fallback preferred tier on 4W/5W and multi-pass LOAD
  retries to keep Humvee+MD pairs together after transport unload.

## Concerns

- **Save / replay compatibility.** Replays from vanilla AI lobbies
  do not embed Tactical AI state and play back identically. Replays
  with a `SLOT_TACTICAL_AI` slot record the flag via the appended
  `ZULU` block; a Zulu binary on a feature-version older than the
  flag's introduction skips it and falls back to Brutal behavior.
  Non-Zulu binaries can't play any of these replays because their
  exeCRC check has already rejected the file.
- **Lobby cross-talk.** The `'T'` letter in lobby strings is a Zulu
  extension. A vanilla client connecting to a Zulu host's lobby
  would fail string parsing before it could mismatch in sim. The
  exeCRC mismatch path catches this earlier.
- **Mismatch surface.** Every Tactical AI behavior touches the
  deterministic sim. Idle-army commit, beacon directive, and the
  USA power-plant hook all run inside the AI tick. They are
  lockstep-safe because they iterate stable orderings and use
  `GameLogicRandom` only; ally chatter is UI-only (`TheInGameUI`)
  and explicitly excluded from CRC. Anything added under this
  umbrella in the future has to keep that invariant.
- **`SkirmishScripts.scb` override.** Loose `.scb` files in
  `Data/Scripts/` take precedence over the ones inside `.big`
  archives via the per-path "archive-wins" exception list (see
  [mapcache-split.md](mapcache-split.md) for the related override
  mechanism). The modded script must remain a strict superset of the
  vanilla flow — all changes are inside `if PLAYER_IS_TACTICAL_AI`
  branches so vanilla AI USA, BAI, and humans hit unmodified
  compositions.
