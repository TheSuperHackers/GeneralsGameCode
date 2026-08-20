# TECHBYSAKH Frontline

## Mode

**Team Assault**, 2v2 to 4v4.

## Map fantasy

A broken industrial frontier surrounds a contested logistics corridor. Two team bases sit on opposite elevated plateaus. A neutral rail hub and two side depots create meaningful pressure away from the safest base routes without forcing a single one-tile choke.

## Layout contract

The map should use three broad movement lanes. The center lane is the shortest route and contains the primary resource objective. The north lane has rough terrain and infantry cover. The south lane has a longer vehicle route and a pair of destructible bridges that can be repaired or bypassed. Every main base needs at least two practical exits so early harassment does not become a permanent lockout.

The neutral objective should reward control rather than provide an immediate win. A controlled rail hub grants periodic supply crates or a temporary radar pulse through map scripting. Side depots should be valuable enough to contest but not mandatory for every build order. Expansion locations should be mirrored in travel time and resource value.

## Balance targets

| Area | Target |
| --- | --- |
| Main-base distance | Symmetric travel time on the central lane |
| First contact | Fast enough to create tension, late enough for an opening build |
| Neutral objective | Contestable by both teams; no permanent owner advantage |
| Terrain | Cover for infantry without fully blocking vehicles |
| Bridges | Tactical choices, not mandatory single points of failure |
| Late game | Multiple approach routes remain usable after superweapons appear |

## Authoring checklist

Use deterministic object and script names, place valid supply and waypoint markers for every player slot, include observer-safe camera regions, and test map transfer with the executable’s allowed extension and size limits. Verify that the map plays correctly with map ownership, replay recording, and save/load enabled.
