# LAN Lobby Discord Posts

## What it is

When the host hits Randomize in the LAN lobby, two images are posted
to a build-time-configured Discord webhook:

1. An upscaled lobby map preview with each occupied start position
   labeled with the player's name and faction abbreviation.
2. A second "Mirror" image where each occupied start position is
   reflected across the team-split axis so the matchup reads the
   same on either side (1↔3, 2↔4 straight-across for 2v2,
   etc.).

The Mirror post is skipped on FFA, >2 teams, unbalanced teams,
missing waypoints, or coincident team centroids; in `-zulu_debug`
mode the skip reason is written as a debug-chat line.

The webhook URL is baked at build time. Builds without a webhook
configured no-op cleanly.

## Why

LAN sessions often run alongside a Discord channel where spectators
or commentators watch matchups. The preview-with-labels image lets
that channel see the matchup and lobby state without anyone having
to screenshot and paste. The mirror image makes 2v2 matchups easier
to read at a glance for tournaments where one team always plays the
"red" side and one always plays the "blue" side.

The trigger was moved from the Start button to the Randomize button
(PR #21 — `31605cf7d`) so the post fires once the host commits the
randomized roster, before the countdown begins. This matches the
existing flow where `map_summary` and `balance_teams` also fire on
Randomize.

## Code surface

About 1480 lines for the original preview post (PR #10 —
`28b87685c`) plus the mirror image (PR #11's mirror — `04d89eab1`)
across 12 files:

- `GeneralsMD/Code/GameEngine/Source/Common/LobbyDiscord.cpp` —
  ~1320 lines. Builds the preview image: takes the engine's
  `mapPreviews/<map>.tga`, upscales it, paints background-darkened
  player labels over the start positions, then encodes PNG and
  POSTs as `multipart/form-data` via WinINet.
- `GeneralsMD/Code/GameEngine/Include/Common/LobbyDiscord.h` —
  `PostLanLobbyMapToDiscord(LANGameInfo *game)` entry point.
- `cmake/discordwebhook.cmake` + `cmake/DiscordWebhook.h.in` —
  generate `DiscordWebhook.h` with the baked URL from
  `ZULU_DISCORD_WEBHOOK_URL` cmake var. Mirrors the existing
  radarvan client-key mechanism (`cmake/zuluclientkey.cmake`).
- `scripts/docker-build.sh` — picks up the webhook URL from the env
  before invoking the cmake configure step.
- `GeneralsMD/Code/GameEngine/Source/Common/GlobalData.{cpp,h}` —
  `m_zuluDebug` flag.
- `GeneralsMD/Code/GameEngine/Source/Common/CommandLine.cpp` —
  `-zulu_debug` flag handler. Currently the only consumer is the
  webhook gate (drops minimum humans from 2 to 1 so a solo host
  can iterate the rendering).
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/LanGameOptionsMenu.cpp`
  — the call site, now in the Randomize handler after
  `lanUpdateSlotList()`.

The image-rendering pipeline reuses the same map-preview reader
used by the lobby UI, which is why this feature depends on the
mapPreview + ZuluMapCache work in [mapcache-split.md](mapcache-split.md)
and [lan-lobby-ui.md](lan-lobby-ui.md).

## Concerns

- **Backwards compatibility.** None — purely host-side outbound HTTP,
  no protocol or replay/save state involved. Builds without a webhook
  URL skip the call entirely.
- **Mismatch risk.** None. The call runs after `lanUpdateSlotList()`
  on the host only, before any sim runs. It's blocking HTTP and
  takes a few hundred ms, but that's during the lobby Randomize
  pause, not during the game.
- **One-client-per-match gating.** The call is in the host-only
  Randomize handler, which is the only client running that branch.
  No de-duplication beyond that.
- **Privacy.** Player names from the lobby are POSTed to the webhook
  in the image text and (separately) `map_summary` body. Same
  surface as the existing radarvan integrations.
- **Failure mode.** WinINet errors are written to the debug chat
  ring in `-zulu_debug` mode and otherwise swallowed. The post is
  not retried; a transient network blip means no image gets posted
  for that match. Acceptable for a "nice to have" channel post.
