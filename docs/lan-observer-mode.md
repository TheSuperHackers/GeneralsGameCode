# LAN Observer Mode

## What it is

A late-joiner spectate flow: a player who tries to join a LAN game
that has already started gets a "Watch as observer?" prompt instead
of a flat rejection. On accept, the host streams its in-progress
`.rep` file over TCP, and the joiner plays it back as a live replay
that holds at EOF when caught up and resumes when new frames arrive.

The observer sees full god-vision (the engine already auto-switches
to the `ReplayObserver` player on `GAME_REPLAY`). Fast-Forward is
disabled while observing live, since racing past the live edge would
desync the replay and brick the session.

## Why

LAN play sessions are long and people drop in late. Without
observer-mode the only options are "the host quits and restarts
with everyone in the lobby" or "the new player waits and watches
later from the post-game `.rep`." Both are bad. This feature lets
spectators (or stream commentators) join a game that has already
started and watch in near-realtime.

## Code surface

About 1180 lines added across 10 files, all in the `Core/` and
`GeneralsMD/` engine layers, not in upstream-touching paths:

- `Core/GameEngine/Source/GameNetwork/LANObserverStream.{h,cpp}` —
  the TCP listener / sender / receiver. Host listens on
  `NETWORK_BASE_PORT_NUMBER + 100` (8188). Streams the in-progress
  `.rep` file plus a tail of new bytes as they're written, with an
  EOF-vs-active distinction so the joiner knows when catch-up
  finishes.
- `Core/GameEngine/Source/GameNetwork/LANAPI.cpp` /
  `LANAPICallbacks.cpp` / `LANAPIhandlers.cpp` — adds the
  "offer-observer" handshake on top of the existing LAN protocol
  (request-to-join arrives after game start, host replies with a
  port + accept token, client connects).
- `GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp` — new
  `RECORDERMODETYPE_LIVE_OBSERVER` plus `playbackFileLiveObserver()`
  on `RecorderClass`. `readNextFrame` treats EOF as "wait for more
  bytes" while the stream is open; `updatePlayback` closes and
  reopens the engine `File` on retry to defeat stdio buffer
  staleness. FPS boosted to 1000 while draining the snapshot, then
  drops back to the saved limit on first EOF (the "catch-up done"
  signal). `m_TiVOFastMode` is forced off every tick.
- `GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp`
  — observer-mode entry path and the FF lockout.

Pumping subtlety worth knowing about: `TheLAN->update()` is only
called from `LanLobbyMenu.cpp`, so once playback starts the lobby
tick stops. `RecorderClass::update()` pumps `TheLAN->updateObserver()`
every frame in `LIVE_OBSERVER` (joiner) and `RECORD` (host) modes so
accept / send / recv keep flowing during the game itself.

The host broadcasts a system chat line for each new observer
connection. A diagnostic `LANObsLog` writes `ObserverLog.txt` next
to the `.exe` with fflush per line; this is retained while the
feature stabilizes and should be stripped or config-gated before
shipping a release build.

## Concerns

- **Backwards compatibility.** Strictly Zulu-to-Zulu; the new LAN
  messages are appended to the existing protocol and a vanilla
  client receiving the offer-observer messages would drop them.
  No save / replay format changes — observers play back the host's
  normal `.rep` file as it gets written.
- **Mismatch risk.** None, because the observer is a replay viewer.
  It runs `GAME_REPLAY` mode, can't issue commands, and never feeds
  state back into the host's sim. Force-disabling
  `m_TiVOFastMode` is the load-bearing guarantee that the joiner
  can't FF past the host and starve the replay stream.
- **Missing localization.** Two GameText strings fall back to
  hardcoded English:
  `LAN:OfferObserveBody` ("This game is already in progress. Watch
  it as an observer?") and `GUI:LiveObserverNoFF` ("Fast Forward is
  unavailable while observing a live game.").
- **Untested edge cases.** Multiple simultaneous observers,
  observer disconnect mid-game, host quitting while observers are
  connected. None of these are believed to crash, but they aren't
  exercised by current testing.
- **Port.** Host binds `8188`. A LAN that already uses this port
  for something else, or a firewall that blocks it, breaks
  observation. Vanilla games are unaffected because the listener
  is only opened when the host enters game.
