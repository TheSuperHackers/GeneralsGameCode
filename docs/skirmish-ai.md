# Skirmish AI Improvements

## What it is

Three sim-side changes that apply to every skirmish AI (vanilla
Easy / Medium / Brutal **and** Tactical), all under
`AISkirmishPlayer`:

- An idle-army-commit sweep that pushes built units toward enemies
  every 10 seconds.
- A targeting-distance tweak in `acquireEnemy` that scores by
  closest-point on the enemy footprint instead of the bbox midpoint.
- A `countTeamInstancesAlive` helper that prevents depleted teams
  from holding `maxInstances` slots forever, so the AI keeps
  rebuilding losses instead of capping out on dead-team prototypes.

The idle-army-commit sweep is gated to `isTacticalAI()` in the
shipped build — vanilla AI sees only the targeting and team-counting
fixes.

## Why

The skirmish AI builds units and bases competently but had two
strategic blind spots in retail:

- It produced units and never moved them. Once production satisfied
  the team's slot quota, the army just sat at the base. Players
  describe this as "the AI gives up." Cause: no engine-side path
  to commit idle units to attack once their build orders complete.
- Once a team prototype's roster died, the slot stayed occupied in
  `countTeamInstances`. The AI saw it as "still alive" and
  refused to rebuild a replacement of the same type.

The targeting tweak is smaller: stock `acquireEnemy` used the bbox
midpoint for distance comparisons, which is wrong for long buildings
(a Supercenter looked further away than a smaller building tucked
behind it). Switching to closest-point distance makes the AI commit
to the obvious nearer target.

## Code surface

- `Skirmish AI: commit idle units to attack` (PR #2 — `6438712b7`):
  adds `commitIdleArmy()` to `AISkirmishPlayer`. Frame-based timer
  (every 10 sec), stable iteration order, no new RNG. Pulls the
  nearest enemy structure or beacon target and issues an attack
  command to anything currently `AI_IDLE`.
- `Skirmish AI: don't let depleted teams hold maxInstances slots`
  (PR #2 — `44bdf6570`): new `countTeamInstancesAlive()` in
  `AISkirmishPlayer`. Existing call sites that gate "should I build
  more of this team" switch to the alive variant; raw
  `countTeamInstances()` is retained for places that need the slot
  count regardless of liveness.
- `Targeting: closest-point distance` (in the same `6438712b7`
  commit): `acquireEnemy` distance score changes from bbox midpoint
  to closest-point on the enemy footprint.

All three changes live in `AISkirmishPlayer.cpp` /
`AIPlayer.cpp` under `Code/GameEngine/Source/GameLogic/AI/`.
Roughly 100 lines of new logic plus the call-site swaps.

## Concerns

- **Mismatch / lockstep.** All three changes are inside the
  deterministic AI tick. They iterate stable orderings (player
  index, team prototype list, object list), use `GameLogicRandom`
  only (no `rand()`, no `time()`), and don't read floats outside
  the existing pathing / distance math. Safe under lockstep.
- **Replay compatibility.** A replay recorded under the new AI
  targeting/team-count fixes will not play back identically on a
  binary that doesn't have these fixes — the AI's targeting picks
  and rebuild decisions diverge from frame one. The `ZULU` magic +
  feature-version block in the replay header (see
  [tactical-ai.md](tactical-ai.md)) is what gates this; older Zulu
  binaries refuse to play forward-versioned replays via the existing
  exeCRC path.
- **Vanilla AI behavior drift.** The targeting and team-count fixes
  intentionally affect Easy / Medium / Brutal AIs as well as
  Tactical. This is a "fix the engine bug" choice rather than a
  "ship a separate AI" choice. If a future need to preserve exact
  retail behavior for vanilla AI emerges, both can be gated on
  `isTacticalAI()` the same way `commitIdleArmy` is.
