# TECHBYSAKH Map Authoring Rules

The companion data pack should treat every map as a synchronized simulation asset. Map scripts must derive decisions from the shared game state and seed, use deterministic object ordering, and avoid unsynchronized wall-clock or local-machine inputs.

## Required assets

A complete map entry should include the map file, its companion INI data, a localized display name, a preview image, and any required texture or waypoint assets. Use short portable filenames and keep the full transfer set within the executable’s validated extensions and size limits.

## Slot and observer support

Define valid starting locations for every advertised player slot, provide observer-safe regions, and test with a mix of human players, AI players, and an observer. The map must still load if fewer than the maximum player count joins the lobby.

## Objective scripting

Use explicit objective identifiers and monotonic state transitions. Each objective should have a success condition, a failure condition, a visible marker, and a replay-safe completion event. Avoid relying on frame numbers that assume a fixed local render rate; use synchronized logic frames and game timers.

## LAN transfer and release checklist

Before release, verify that all required files announce and transfer successfully on a clean client, that the client rejects an unexpected or oversized file, that map CRCs agree across all players, and that a saved game and replay can be loaded after the mission ends.
