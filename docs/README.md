# Zulu Client Feature Docs

Per-feature notes for the changes Zulu makes on top of the
TheSuperHackers `GeneralsGameCode` upstream. Each doc covers what
the feature does, why it exists, a rough idea of the code surface
required to implement it, and any concerns around multiplayer
mismatch or backwards compatibility (saves / replays / lobby
strings).

This is a developer-facing inventory, not a player-facing changelog.

## Gameplay / AI

- [tactical-ai.md](tactical-ai.md) — `SLOT_TACTICAL_AI` lobby option,
  ally beacon `!attack` directive, ally chatter (milestones, attacks,
  distress), `PLAYER_IS_TACTICAL_AI` script condition, USA Hard wave
  swap, USA power-plant-first build order.
- [skirmish-ai.md](skirmish-ai.md) — Idle-army commit sweep,
  depleted-team `maxInstances` fix, distance-scoring tweak in
  `acquireEnemy`.

## LAN multiplayer

- [lan-observer-mode.md](lan-observer-mode.md) — Late-joiner
  spectate-in-progress flow over TCP replay streaming, FPS catch-up,
  Fast-Forward lockout.
- [lan-lobby-discord.md](lan-lobby-discord.md) — Webhook posts of
  upscaled labeled map preview and matchup-preserving mirror image
  on Randomize.
- [lan-lobby-ui.md](lan-lobby-ui.md) — Map-select player-count
  filter, lobby map preview derrick/crate overlays, Randomize button,
  `map_summary` blurb echoed to lobby chat.

## Visuals / UI

- [player-colors.md](player-colors.md) — `GameMakeColorReadable`
  luminance lift, `ZuluColors.ini` rename + Metallic Grey,
  `ColorBlack` removal.
- [terrain-and-camera.md](terrain-and-camera.md) —
  `VERTEX_BUFFER_TILE_LENGTH` bump for `MaxCameraHeight=450` and
  `farZ` scaling fix for the black-top-band artifact.
- [slide-anim-snap.md](slide-anim-snap.md) — Snap-to-final-position
  fix for `SlideFromRight/Left` window animations.

## Replays / Telemetry

- [telemetry-uploads.md](telemetry-uploads.md) — radarvan replay /
  map / stats uploads, `ZUTG` trailer, X-API-Key auth, 2+ human
  gate.
- [resume-from-replay.md](resume-from-replay.md) — `00000000.rep`
  fast-forward resume into a fresh lobby.
- [headless-replay.md](headless-replay.md) — `[HEADLESS RESULT]`
  winner line for CI / batch replay processing.

## Build / Launcher / Cache

- [launcher-auto-update.md](launcher-auto-update.md) — `ZuluLauncher.exe`
  + `latest.json` auto-update with argv preservation.
- [mapcache-split.md](mapcache-split.md) — `ZuluMapCache.ini` split
  from vanilla `MapCache.ini`, auto-rebuild sentinel.
- [installer-build.md](installer-build.md) — `make installer-release`
  pipeline, `BuildVersion.h` plumbing, replay header version from
  `APPVERSION`, RC dependency tracking.

## Backwards compatibility ground rules

Everything in this directory has been designed to keep retail
replays, retail-format `.sav` files, and existing lobby strings
deserializable. The two main rules followed:

1. **Enums are appended, never inserted or renumbered.** New
   `SlotState`, new `RECORDERMODETYPE`, new script-condition IDs,
   etc., go at the end of their enum.
2. **Anything new in the replay header sits inside the existing
   `ZULU` magic + feature-version block** added after `maxFPS`.
   Older Zulu binaries skip features above their feature version;
   non-Zulu binaries can't play Zulu replays at all because the
   existing exeCRC check rejects them.

See [MISMATCH_NOTES.md](../MISMATCH_NOTES.md) for upstream's running
list of fixed and likely-still-broken mismatch sources. None of the
features here intentionally touch deterministic sim ordering, RNG,
or CRC inputs, but any change that adds an AI tick (Tactical AI,
beacon directive, USA build-order hook) is by definition sim-side
and gated on the Tactical AI flag so vanilla AI matches retail
behavior.
