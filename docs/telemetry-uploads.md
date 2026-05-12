# Telemetry and Replay Uploads

## What it is

The Zulu client uploads replays, map files, and end-of-game stats
to a backend ("radarvan") so post-game analysis, leaderboards, and
map history blurbs work. The pipeline:

- **Replay upload** (commit `371d89b5d`): on game-over, the host
  POSTs the just-recorded `.rep` to `m_replayUrl` (default
  `https://www.radarvan.com/api/upload_replay`) as
  `multipart/form-data`.
- **Map upload** (commit `d90bbedb8`): on game-over, the host
  POSTs the `.map` file (and its sibling assets) so the backend
  can render previews / parse start positions without scraping
  every player's install.
- **Stats exporter** (commit `5ba918ba2`): in-game and end-of-game
  events get serialized and posted as cncstats-format payloads for
  leaderboards.
- **`ZUTG` trailer** (PR #14 — `076cfc2da`): the radarvan replay
  upload buffer gets an 8-byte trailer appended in memory before
  POST. Layout (little-endian): `"ZUTG"` magic (4 B) + version
  `0x0001` (2 B) + payload-len `0x0000` (2 B). The on-disk `.rep`
  is untouched — the trailer exists only in the upload buffer.

## Why

- **Replay upload** lets the radarvan site show recent games per
  map, per player, per general matchup without players having to
  hand-share `.rep` files.
- **Map upload** is what enables the lobby `map_summary` blurb,
  the labeled Discord preview, and the in-progress observer
  flow (LAN observer mode reads the host's `mapPreviews/<map>.tga`
  rather than the `.map`, but the backend's preview is what the
  Discord image starts from).
- **Stats exporter** is the leaderboard / ELO data feed.
- **`ZUTG` trailer** lets the radarvan server distinguish replays
  uploaded directly by the Zulu client from re-uploads of the
  same `.rep` file by third-party tools (notably gentool's
  parser, which whitelists `CE`/`CM`/`CH` slot types and rewrites
  the new `CT` Tactical AI slot to `X`, producing a "missing
  computer player" replay that the backend would otherwise have
  no way to tell apart from the Zulu-client copy).

## Code surface

- `GeneralsMD/Code/GameEngine/Include/Common/StatsUploader.h` and
  `Source/Common/StatsUploader.cpp` — the WinINet `multipart/form-data`
  POST helpers, plus the `AppendZuluUploadTag` helper that builds
  the `ZUTG` trailer. About 700 lines total.
- `GeneralsMD/Code/GameEngine/Include/Common/StatsExporter.h` plus
  the cncstats serialization in `StatsExporter.cpp`.
- `GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp` — the
  default URLs (`m_replayUrl`, `m_balanceTeamsUrl`,
  `m_mapSummaryUrl`).
- `GeneralsMD/Code/GameEngine/Source/Common/CommandLine.cpp` —
  matching `-replayUrl`, `-balanceTeamsUrl`, `-mapSummaryUrl`
  overrides so QA / staging can redirect away from production.
- `cmake/zuluclientkey.cmake` — bakes the build-time radarvan auth
  key into `ZuluClientKey.h`, which `StatsUploader.cpp` injects as
  an `X-API-Key` header.
- `GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp` — invokes
  the upload helpers on game-over, gated on `>= 2 human players`
  (commit `9bc0a2a8d`).

## Concerns

- **Privacy.** The replay and stats uploads include player names,
  game outcome, and the chosen factions / colors. Map files
  uploaded include the host's local copy verbatim. This is the
  same surface as gentool / GameSpy stats; players who don't want
  it can run an unsigned build with the URLs blanked, or use the
  command-line overrides.
- **Gating on 2+ humans.** Solo / AI-only games don't upload
  (commit `9bc0a2a8d`). This is policy, not a network concern;
  match results for AI-only games aren't leaderboard-meaningful
  and would just pollute the data.
- **Mismatch risk.** None. Uploads run after game-over from the
  host only. The `ZUTG` trailer is appended to a malloc'd upload
  buffer; the on-disk `.rep` file is never modified, so a replay
  played back locally is byte-identical to the upload-minus-trailer.
- **Backwards compatibility.**
  - Replay format on disk is unchanged.
  - `ZUTG` is a forward-compatible block: the server treats
    files lacking the trailer as third-party re-uploads.
  - The `ZULU` magic + feature-version block in the replay header
    (separate from `ZUTG` — that one is in the header, this one
    is appended to the buffer) is what gates AI-feature
    compatibility across Zulu binary versions.
- **Auth failure mode.** WinINet errors and HTTP non-2xx
  responses are logged to debug chat in `-zulu_debug` mode and
  otherwise swallowed. Uploads are not retried. A flaky network
  means that match's data is lost; the game itself is unaffected.
