# Multiplayer Mismatch Notes

A summary of fixed mismatch sources in this codebase, the in-game actions that triggered them, and likely-unfixed sources worth investigating.

## Categories of fixed mismatch causes

### 1. Uninitialized variables
Reading garbage memory diverges per machine.
- `Pathfinder::classifyFence` cellBounds (#1748, VC6 revert in #1075)
- `Pathfinder::tightenPathCallback` (#2309)
- `ObjectCreationList` (#2096)
- `AiPathFind::findGroundPath` reverted to match retail (#904)

### 2. Out-of-bounds / heap corruption in logic
- `DumbProjectile` with very high speed weapons at tiny hit distances (#2087)
- new/delete and array-new/scalar-delete mismatches (#463, #547, #606, #947, #1223, #2287)

### 3. RNG and seeding
- Skirmish replay restart not restoring game seed (#2270)
- Slot values not reset on skirmish restart (#2373)
- `InitGameLogicRandom` vs `InitRandom` audio/client seed inconsistency (#2339)
- Runahead causing mismatch (14b81b45f)

### 4. CRC computation flaws
- `PathFinder::m_wallPieces` not CRC'd correctly (#2575)
- Scripted audio events leaking into CRC (#2075)
- `NameKeyGenerator` coupling File System into CRC (#1516)
- `GlobalData` Exe CRC computed at wrong time (#1197)
- Division by zero / minimum CRC interval guards (05957b65b, 9878fdd80)

### 5. State leakage between games or map loads
- `ThingTemplate` copy for shell and custom maps (#1234)
- Next `ThingTemplate` ID not reset after custom map overrides cleared (#2034)
- Wrong `INIZH.big` picked up from `Data/INI` (#1879)
- `StateMachine` refcount diverging from retail (#1078)
- Power sabotaged player carrying state into next game (#1238)

### 6. Per-player and index bugs
- `BuildAssistant` using wrong player index for shroud check on placement (#1646)
- Surrender in team games with previously cleared base structures (#546)

### 7. Non-deterministic ordering or timing
- Aircraft takeoff order (#1297)
- GLA Battle Bus initial death damage calc (#1603)
- ParticleUplinkCannonUpdate refactor regression (#1012)
- Sabotage Crate Collide retail-CRC drift (#1571)
- WW3D sync coupling to game logic step (#1579, #1451)
- `Network::getExecutionFrame` side effects (98cdf422c)

### 8. Build and toolchain divergence
- 32 bit vs 64 bit `time_t` in replay header (#765)
- Logging changing release-build behavior (#759)
- Calling conventions, variadic macros, static/extern linkage for MinGW (#2067)
- Signed/unsigned loop index warnings (#643, #804)

## In-game actions that have caused mismatches (fixed)

### Combat and weapons
- Firing very-high-speed weapons at extremely close range. `DumbProjectile` flight-path was indexed with fewer than 2 segments, reading uninitialized memory past the vector. Triggered by any unit firing point-blank with a fast projectile (#2087).
- Black Lotus or hacker units being targeted by an enemy attempting to place a building over them. The "busy unit" check ran inside the shroud check and changed CRC depending on what each player could see (#1646).
- Anything that causes the GLA Battle Bus first death (transition to its second life). Damage calc was non-deterministic vs retail (#1603).

### Building placement
- Placing any structure where part of the placement footprint overlaps fogged or shrouded objects only some players can see. The original code used `getDrawable()->getFullyObscuredByShroud()`, which is per-client visual state, not per-player logical shroud. Each client returned a different `LBC_SHROUD` result for the same command (#1646).

### Sabotage crates
- A worker or saboteur entering enemy Power Plant, Supply Center, Supply Dropzone, Command Center, Internet Center, Military Factory, Superweapon, or Fake Building. Code paths in retail had unintentional state that the cleaner refactor was changing (#1571).

### Surrender or player kill in team games
- Surrendering or being killed when you own a base structure (typically a garrisoned building like a Command Center or Barracks) that previously had infantry inside but is now empty. `becomingLocalPlayer(TRUE)` reset the contained-team for empty contains only on the local client (#546).

### Sabotage and Power Plant interaction
- A player gets their power plants sabotaged, then the match ends or that player exits. The next match retained that sabotage state on energy, so totals diverged across clients (#1238).

### Aircraft (Airfield, Comanche, Helix)
- Multiple aircraft taking off from the same airfield in the same frame. Iteration order over parking spaces was deterministic but allowed higher-indexed spaces to grab the runway first if the lower-indexed plane wasn't ready, producing different orders depending on micro-state (#1297).

### Particle Uplink Cannon (China superweapon)
- Firing PUC was mismatching after a refactor. Trigger was simply the superweapon firing (#1012).

### Pathfinding
- Any unit pathing across a fence cell on the map. `Pathfinder::classifyFence` had an uninitialized `cellBounds` (#1748). Same class of bug in `tightenPathCallback` (#2309), triggered by any path the pathfinder decides to "tighten."
- The map containing wall pieces. `m_wallPieces` wasn't included in CRC properly (#2575).

### Object spawning
- Anything that produces objects via `ObjectCreationList` with uninitialized fields (#2096). Broad class, anything from death effects to upgrades to script-spawned objects can trip it.

### Map and session lifecycle
- Loading the shell map and then a custom map, or vice versa. Custom maps' `ThingTemplate` overrides leaked into the next session and shifted template IDs (#1234, #2034).
- A multiplayer session where one host has `Data/INI/INIZH.big` and another doesn't. Filesystem search order picked different files, producing different INI CRCs (#1879).

### Replay and skirmish restart
- Restarting a skirmish, then watching the replay of the second run. Game seed wasn't restored (#2270, #2373).

## In-game actions still likely to trigger mismatches (unfixed or under-tested)

Based on the fix patterns above plus known fragile areas of the codebase.

- Garrisoned-building handover edge cases beyond surrender: capture with Hijacker or Defector, Black Lotus capture, Salvage Crate giving the building to someone else, neutral structure capture. The Contain class still has the underlying team-tracking bug that #546 just disables one branch of.
- Disguise units (Terrorists, Disguised Bombtruck, Hijacker) when a player becomes an observer mid-game or when fog state changes. The reverted `becomingLocalPlayer` call was originally there to refresh disguises.
- Stealth detection flips driven by per-player shroud (mirror of the build placement bug). Anything in logic that reads `getDrawable()` shroud state instead of `getShroudedStatus(playerIdx)`.
- Hero abilities and ability-busy state affecting other logic decisions (the relocated busy-unit check shows this category exists).
- Carpet-bomb, cluster munitions, MIRV, Tomahawk volleys. Same family as DumbProjectile. Anything iterating sub-projectiles or splitting payloads near terrain.
- Helicopter rappel, Chinook-Overlord transport, Comanche rocket-pod salvos firing many things in one frame.
- Demo Trap, GLA Tunnel Network. Multiple objects sharing state, inter-tunnel teleport ordering depends on container iteration order.
- Salvage Crate, Veterancy Crate, Money Crate pickups when multiple units enter the trigger same frame (resolution order).
- AI Skirmish opponents issuing many simultaneous orders. Script execution order has bitten this codebase before.
- Power-down or power-up cascades on partial power loss (radar, superweapon timers, stealth detector range). Same family as the sabotaged-power bug.
- GPS Scrambler, EMP, Neutron effects that flip many objects' logical state in one frame.
- Repair via Drone vs. Worker vs. Repair Bay racing on the same building.
- Kill-net or death-cleanup on game-end frame. Anything that reads state after `killPlayer` runs.
- Map transitions during co-op campaign if used in multi.
- Dropping a player mid-match. Surviving clients' resource and score recalculation paths historically diverge.
- Custom maps with overridden ThingTemplate fields (armour, weapons) when one player has the map cached and another freshly downloaded it. Same family as #1234 and #2034 if any path was missed.
- Audio-driven triggers the codebase has been actively decoupling (#2075). Anything where a script calls play-sound and that sound's completion is read by logic could still leak.
- Aircraft re-arming and landing. The deterministic-takeoff fix only covered takeoff order. Landing, re-rearming, and transferring runway reservations on death of a queued plane still rely on the same iteration logic.

## Other classes of likely mismatch sources

- Floating-point divergence: x87 control word, SSE vs x87 codegen, `fast-math`, denormals-are-zero. Different compilers or optimizers reorder FP ops. Verify FPU control word is set identically on every host at game start.
- STL container iteration order: anything iterating a `std::map` or `std::set` keyed by pointer addresses, or any `hash_map` not seeded identically, will diverge.
- `time()`, `GetTickCount()`, or `rand()` ever feeding into logic. Logic must use `GameLogicRandom` only. Audio and UI seeds must not flow back.
- Locale, codepage, or `tolower` affecting string compares used as map keys (e.g. `NameKeyGenerator` lookups).
- Filesystem case-sensitivity or directory ordering: traversal order on Linux vs Windows can change INI load order, file-id assignment, or override resolution.
- Threading: any worker thread mutating logic-visible state, especially audio callbacks, pathfinder helpers, or network thread writing into game-frame data.
- Static or global initializer order across translation units, especially in modules touched by recent refactors.
- Pathfinder caches that persist across maps without being reset, or that depend on insertion order.
