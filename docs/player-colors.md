# Player Color Readability

## What it is

Three related changes to player-color handling:

- **`GameMakeColorReadable`** (PR #11 — `54573eb3a`): an additive
  luminance lift applied at the three text-render call sites that
  use the raw player color. Computes Rec. 601 luminance and bumps
  to a `MIN_LUMINANCE = 80` floor so dark colors are still legible
  on dark backgrounds.
- **`ZuluColors.ini` rename + new colors** (PR #11 — `a33458dfa`,
  `96f654892`): the multiplayer color identifiers are renamed to
  match the base `multiplayer.ini` convention (`ColorRed`,
  `ColorBlue`, ..., `ColorBlack`). The `Silver` entry is replaced
  with `ColorMetallicGrey` at `#708090`.
- **`ColorBlack` rendering fix** (PR #11 — `91cdc8d98`): `ColorBlack`
  ships as `RGB(1,1,1)` instead of `RGB(0,0,0)` because the W3D
  asset manager treats pure black as a sentinel meaning "use the
  default color," which rendered as red.

## Why

Two long-standing player-color complaints:

- A player who picks one of the dark colors (Black, dark blue,
  brown) ended up invisible in chat boxes and barely visible in the
  end-of-game score screen, because both render player names
  against a dark backdrop.
- `ColorBlack` rendered as red anywhere W3D dispatched the color
  through its sentinel-aware path. Players selecting Black saw a
  Black swatch in the lobby but Red on the battlefield.

The rename to `Color*` prefix isn't player-facing but cleans up the
ini convention so future tooling can rely on `Color` being the
identifier prefix.

## Code surface

About 200 lines total across the engine and assets:

- `Core/GameEngine/Include/GameClient/Color.h` —
  `GameMakeColorReadable` + the `MIN_LUMINANCE` constant. The single
  tuning knob if `(80, 80, 80)` needs to change.
- `GeneralsMD/Code/GameEngine/Source/GameNetwork/ConnectionManager.cpp`
  (in-game LAN chat — `processChat`).
- `GeneralsMD/Code/GameEngine/Source/GameNetwork/LANAPICallbacks.cpp`
  (lobby chat — `onChat`).
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/ScoreScreen.cpp`
  (`populatePlayerInfo`, `populateSideInfo`).
- `assets/Data/ZuluColors.ini` — the color list itself, including the
  `RGB(1, 1, 1)` Black workaround and the new `ColorMetallicGrey`.
- `assets/Data/English/GeneralsExtras.str` — the `Color:Black` /
  `Color:MetallicGrey` tooltip strings.

Three call-site wraps was a deliberate choice over wrapping the
color earlier in the pipeline: identity-bearing surfaces (minimap
dots, beacons, unit health bars, infantry chevrons) **must** retain
the player's chosen color exactly, because they're how teammates
identify each other on the map. Only the text-against-dark
surfaces get the lift.

## Concerns

- **Mismatch risk.** None — the readability lift runs in render
  paths only and never feeds back into sim state.
- **Replay compatibility.** None — replays don't store
  text-rendering colors. The `ColorBlack` `RGB(1,1,1)` change is in
  `ZuluColors.ini` which ships inside `Zulu.big`; a replay from a
  Zulu client played on a vanilla client wouldn't pick up the
  override, but the existing exeCRC mismatch path already blocks
  cross-binary playback.
- **`ZuluColors.ini` identifier rename.** The earlier Zulu builds
  used `Silver`, `Black`, etc. directly. Lobby slot strings encode
  the color by index, not by name, so the rename doesn't break
  serialized lobby state, save games, or replays. It does mean any
  external tooling that hard-coded the old identifier strings would
  need updating; nothing in this repo does.
- **Tuning the floor.** `MIN_LUMINANCE = 80` was picked by eye
  against the actual chat/score widgets. If a future widget renders
  player text against a lighter backdrop, this lift may push light
  player colors past white and clip; consider clamping or per-widget
  thresholds before reusing the helper there.
