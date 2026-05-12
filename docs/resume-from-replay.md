# Resume From Replay (FF Resume)

## What it is

A host-only button in the LAN lobby that arms the last-recorded
replay (`00000000.rep`) for a fast-forward replay. On match start
the engine plays back the replay at the maximum FPS until it hits
the live frame the recording ended at, then drops to normal play
and hands control back to live commands.

In effect: "I crashed mid-game, let me resume from where we
left off."

The button is de-armed automatically on entering the lobby
(commit `11fc3d761`) so a host who has resumed once and then
returns to the lobby normally doesn't accidentally re-resume the
old recording.

## Why

Generals matches are long and one of the most common ways a LAN
session ends is a single player's machine crashing or a flaky
network drop. Without resume the only options are "start the
match over" or "let it end with that player gone." FF Resume
lets the lobby rebuild the disconnected player's session by
replaying the host's recording fast-forward to the disconnect
point.

This is also the foundation that LAN Observer Mode reused
(`RECORDERMODETYPE_LIVE_OBSERVER`): the live-observer path is
essentially "FF Resume but the host keeps appending to the `.rep`
file while we play it back."

## Code surface

Big initial commit (`a2b916c0c`) — about 600 lines of engine code
plus a regenerated `.wnd` and `generals.csf`:

- `GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp` — the
  bulk of the new logic. Adds the FF-resume playback mode,
  `m_TiVOFastMode` use, and the frame-target / catch-up handling
  that LAN Observer Mode later piggybacked on.
- `Core/GameEngine/Source/Common/FramePacer.cpp` — FPS boost
  during catch-up.
- `Core/GameEngine/Source/GameNetwork/Network.cpp` — frame-target
  coordination so commands queue against the catch-up cursor
  instead of the live wallclock.
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/LanGameOptionsMenu.cpp`
  — the lobby button and the host-only arm path. The de-arm-on-
  lobby-entry fix is in this file (`11fc3d761`).
- `assets/Data/English/generals.csf` — adds the resume-button
  GameText strings.
- `MISMATCH_NOTES.md` — the original commit also wrote the
  upstream mismatch-notes document (now lives in repo root).

## Concerns

- **Mismatch risk.** Resume replays the recording on the host
  binary in its current state; if any sim behavior has changed
  since the recording (e.g. an AI fix on a newer Zulu binary), the
  replay diverges. The `ZULU` magic + feature-version block in the
  replay header (see [tactical-ai.md](tactical-ai.md)) is what
  prevents this: a Zulu binary playing back a replay tagged with
  a newer feature version refuses, and a binary playing back its
  own version replay is by definition consistent.
- **Save / replay format.** No on-disk format changes from the FF
  Resume work itself. The `m_TiVOFastMode` flag is in-memory only.
- **Lobby button visibility.** Host-only. Non-host clients don't
  see the button. If a non-host had the most-recent `.rep`, they
  can't resume — only the host can, because the resumed match
  re-broadcasts as a fresh game with the host as the recording
  origin.
- **Single-replay assumption.** FF Resume always arms
  `00000000.rep`, the last-recorded replay slot. If the engine
  later supports rotating through multiple replay slots, the
  button needs a picker or has to be reworked.
- **Catch-up FPS spike.** FPS during catch-up is boosted (the
  exact value lives in `Recorder.cpp`). On weaker machines this
  can briefly stress the CPU; the catch-up duration is bounded by
  the replay length and is over within seconds for a typical
  disconnect.
