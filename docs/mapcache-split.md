# MapCache Split: ZuluMapCache.ini

## What it is

Zulu reads and writes its own `ZuluMapCache.ini` in both the
install `Maps/` directory and `<UserData>/Maps/`, and never
touches the vanilla `MapCache.ini`. Auto-rebuild is driven by a
sentinel comment on the first line of the file
(`; MapCacheFormatVersion = N`). When `updateCache()` reads a
file whose sentinel is missing or has an older version number, it
rebuilds from scratch.

The old `-buildmapcache` CLI flag was removed (PR #10) in favor
of the sentinel — the cache rebuilds itself whenever the schema
bumps, no manual flag needed.

## Why

Earlier Zulu builds wrote new map-cache fields (`cratePosition`,
`techDerrickPosition`, see [lan-lobby-ui.md](lan-lobby-ui.md))
directly into the vanilla `MapCache.ini`. Two problems:

- Players who alternated between Zulu and a vanilla build (or
  another mod sharing the same Maps/ dir) had the vanilla build
  reject `MapCache.ini` because of unknown fields, and rebuild
  the cache from scratch every time it launched. The rebuild
  scans every `.map` file in the install and is slow (seconds to
  tens of seconds depending on map collection size).
- Forward-format Zulu writes were destructive — if a player
  rolled back, the new fields got dropped on the next vanilla
  rebuild.

Splitting to `ZuluMapCache.ini` (PR #18 — `0376ba3ba`) means Zulu
and vanilla each maintain their own cache independently. Each
side self-heals on its own startup via its own sentinel.

The sentinel mechanism replaces the previous "missing-sentinel
=> force rebuild" trigger, which was firing on every vanilla
launch because vanilla's cache by definition doesn't have a
sentinel.

## Code surface

About 200 lines in
`Core/GameEngine/Source/GameClient/MapUtil.cpp`:

- All read / write paths swap `MapCache.ini` for
  `ZuluMapCache.ini`.
- `updateCache()` reads the first 1KB of the cache file looking
  for `; MapCacheFormatVersion = N`; rebuilds if missing or
  outdated.
- The `-buildmapcache` flag parser is removed from
  `CommandLine.cpp`.

The new map-cache fields themselves (`cratePosition`,
`techDerrickPosition`) live in this same file. The icon rendering
that consumes them is in
`Code/GameEngine/Source/GameClient/MapPreview.cpp`.

## Concerns

- **MapCache cleanup on install.** Bumping the format version
  forces a rebuild on every machine the next time Zulu launches.
  Fast on SSDs with a normal map set, slower on HDDs / large
  collections. Acceptable tradeoff for not corrupting vanilla
  state.
- **Admin-on-Windows quirk.** Writing the cache to the install
  directory needs admin permissions on a standard Windows
  install. The truncate-`fopen` fails silently when unelevated, so
  a player running Zulu without admin will rebuild every launch
  and never persist. See memory
  `project_mapcache_cleanup_admin.md`. The user-dir cache
  (`<UserData>/Maps/`) doesn't have this problem; for now the
  install-dir cache is treated as best-effort.
- **Mismatch / save / replay.** None. The map cache is purely a
  client-side parse cache; the actual `.map` files are what
  matter for multiplayer, and those are unchanged.
- **Backwards compatibility.**
  - Vanilla clients reading their own `MapCache.ini` see no Zulu
    fields and continue working normally.
  - Zulu reading an old-version `ZuluMapCache.ini` rebuilds
    automatically.
  - A pre-PR-#18 Zulu binary running against a post-split install
    will find `MapCache.ini` missing the new fields, fail its
    sentinel check, and rebuild the vanilla cache (writing the
    new fields back in). This is harmless but mildly wasteful;
    the previous Zulu build is now superseded by the split build.
