# Player Color Readability

## What it is

Two related changes to player-color handling, plus a follow-up
removal:

- **`GameMakeColorReadable`** (PR #11 — `54573eb3a`): an additive
  luminance lift applied at the three text-render call sites that
  use the raw player color. Computes Rec. 601 luminance and bumps
  to a `MIN_LUMINANCE = 80` floor so dark colors are still legible
  on dark backgrounds.
- **`ZuluColors.ini` rename + new colors** (PR #11 — `a33458dfa`,
  `96f654892`): the multiplayer color identifiers are renamed to
  match the base `multiplayer.ini` convention (`ColorRed`,
  `ColorBlue`, ...). The `Silver` entry is replaced with
  `ColorMetallicGrey` at `#708090`.
- **`ColorBlack` removed** (follow-up): the entry was dropped from
  `ZuluColors.ini` to take Black out of the lobby color picker.
  Black was the last entry in the file so no other color's index
  shifts. PR #11 originally shipped `ColorBlack` as `RGB(1,1,1)` to
  dodge a W3D sentinel that renders pure black as red; that
  workaround is no longer needed.

## Why

Two long-standing player-color complaints:

- A player who picks one of the dark colors (dark blue, brown) ended
  up invisible in chat boxes and barely visible in the end-of-game
  score screen, because both render player names against a dark
  backdrop.
- `ColorBlack` rendered as red anywhere W3D dispatched the color
  through its sentinel-aware path. Players selecting Black saw a
  Black swatch in the lobby but Red on the battlefield. (Black is
  no longer a selectable color.)

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
- `assets/Data/ZuluColors.ini` — the color list itself (including
  `ColorMetallicGrey`).
- `assets/Data/English/GeneralsExtras.str` — the `Color:MetallicGrey`
  tooltip string.

Three call-site wraps was a deliberate choice over wrapping the
color earlier in the pipeline: identity-bearing surfaces (minimap
dots, beacons, unit health bars, infantry chevrons) **must** retain
the player's chosen color exactly, because they're how teammates
identify each other on the map. Only the text-against-dark
surfaces get the lift.

## Concerns

- **Mismatch risk.** None — the readability lift runs in render
  paths only and never feeds back into sim state.
- **Replay compatibility.** None for the readability lift — it runs
  in render paths and never feeds back into sim state. Removing
  `ColorBlack` does mean an old replay whose slot picked Black will
  hit the `which >= getNumColors()` branch in `getColor()` and
  receive `nullptr`; downstream callers dereference without a null
  check (e.g. `GameLogic.cpp:1463`), so such replays can crash.
  Acceptable trade — Black was Zulu-only, so only Zulu replays from
  the brief window when it shipped are affected.
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
