# TECHBYSAKH Cooperative Canyon

## Mode

**Co-op Mission**, 2 to 4 players.

## Mission fantasy

A canyon network protects an emergency communications site from escalating enemy waves. Players start with modest bases on separate canyon shelves, then must secure the relay, escort a construction convoy, and hold the completed uplink until extraction.

## Objective chain

1. **Secure the relay.** Human players must occupy the marked relay zone together or keep it clear for a short capture timer.
2. **Open the convoy route.** Destroy or bypass two scripted blockade groups while protecting the neutral engineering convoy.
3. **Build the uplink.** The map script unlocks the final construction objective after the route is safe. Each player can contribute units, builders, or protection.
4. **Hold for extraction.** Reinforcement waves use multiple entry regions and scale with the number of human players. The mission succeeds when the uplink survives the final timer.

## Co-op design rules

The map should provide at least one safe rally position for each player, a shared but not unlimited resource pool, and clear objective markers. Enemy waves must have a readable warning period and must not spawn inside a player’s base perimeter. A player who loses a command center should remain useful as an observer or support participant when the companion data pack supports that behavior.

## Difficulty knobs

| Knob | Easy | Standard | Hard |
| --- | --- | --- | --- |
| Initial enemy pressure | Delayed | Moderate | Early scout pressure |
| Wave interval | Long | Medium | Short |
| Reinforcement size | Small | Mixed | Combined-arms |
| Final hold | Short | Medium | Long |
| Resource support | Generous | Balanced | Tight |

All objective state must be deterministic and replay-safe. Do not use wall-clock randomness in mission scripts; derive variation from the game seed and synchronized frame state.
