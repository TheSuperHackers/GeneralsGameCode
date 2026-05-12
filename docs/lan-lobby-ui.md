# LAN Lobby UI

A grab-bag of LAN lobby improvements that don't ship enough code
each to warrant their own doc.

## Map-select player-count filter (PR #9)

A dropdown above the map list filters by required player count.
"Any" disables the filter; "2 Players" through "8 Players" filter
the list to maps that match. Stacks with the existing name-search
box.

While the map-select panel is open, all four bottom-row buttons on
`LanGameOptionsMenu` are greyed out via a new `gadgetsToDisable`
helper, since the panel doesn't physically cover them and clicking
through caused state confusion.

Code touchpoints: `LanMapSelectMenu.cpp`, `LanGameOptionsMenu.cpp`
(disable helper), plus the new `.wnd` entries for the dropdown. No
sim-side changes.

## Lobby map preview overlay (PR #10)

The map preview window in the lobby now draws two icon types over
the preview image:

- A yellow gas-pump derrick icon on every map cell containing a
  `KINDOF_TECH_BUILDING` (typically the Oil Derrick).
- A blue `$` crate icon on every Salvage / Veterancy / Money crate.

`MapCache.ini` (now `ZuluMapCache.ini` — see
[mapcache-split.md](mapcache-split.md)) gained two new per-map fields,
`techDerrickPosition` and `cratePosition`, populated during the
mapcache rebuild scan and read by the preview renderer.

A `MapCacheFormatVersion = 2` sentinel was added so the cache
auto-rebuilds when the format bumps; the old `-buildmapcache` CLI
flag was removed in favor of this.

Code touchpoints: `MapUtil.cpp`, `MapPreview.cpp`, the new icon
assets baked into `Zulu.big`. About 300 lines.

## LAN Randomize button and team balancing (PR #1, #12, #17, #19)

The original Zulu Randomize button (commit `00f8423e6`) does
three things on host-press:

1. **Balance teams via API** — POSTs `{players: [{name, general,
   team}]}` to the radarvan `balance_teams` endpoint and applies
   the returned team assignments. The team field was added in
   PR #17 (`9c2ba0c88`) so the server can balance around existing
   team locks. A separate fix (`2677deda9`) handles the case where
   the server canonicalizes player names (e.g. "Pan" → "panda")
   so the returned roster still matches lobby slots.
2. **Randomize unconditional slots** — every slot that doesn't have
   a locked faction / color / start position gets a randomized
   assignment under `performRandomAssign`.
3. **Fetch and broadcast `map_summary` blurb** — once balance + random
   succeed, POST the post-randomize roster to the radarvan
   `map_summary` endpoint and echo each `\n`-split line of the
   response into the lobby chat. PR #19 (`54876c1ce`) extended
   this from "host-only echo" to "broadcast to all players in the
   lobby."

Code touchpoints: `LanGameOptionsMenu.cpp` (Randomize handler),
`GlobalData.{cpp,h}` (`m_balanceTeamsUrl`, `m_mapSummaryUrl`),
`StatsUploader.cpp` (the actual HTTPS calls).

## Concerns (across all of the above)

- **Backwards compatibility.** None of these touch sim or replay
  state. `ZuluMapCache.ini` is intentionally separate from vanilla
  `MapCache.ini` so vanilla clients sharing a Maps/ dir stay
  unaffected.
- **Mismatch risk.** None — all UI-side, host-side, or pre-game.
  The team-balance API result is applied to slot state before
  `RequestGameStart` so by the time the sim runs every client sees
  the same slots.
- **API-server dependencies.** `balance_teams` and `map_summary`
  are radarvan endpoints. If they're down or slow, Randomize still
  randomizes locally but the chat blurb is skipped; the network
  error is shown as a SYSTEM line in lobby chat. The default URLs
  live in `GlobalData.cpp` and can be overridden via the
  `-balanceTeamsUrl` / `-mapSummaryUrl` command-line flags.
- **Discord post timing.** The Discord webhook post (see
  [lan-lobby-discord.md](lan-lobby-discord.md)) also fires from the
  Randomize handler, after `lanUpdateSlotList()`. It's blocking
  HTTP and adds a few hundred ms to the Randomize click pause.
