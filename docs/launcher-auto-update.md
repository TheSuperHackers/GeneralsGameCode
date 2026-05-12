# Launcher Auto-Update

## What it is

`ZuluLauncher.exe` is a small WIN32 process that ships alongside
`generalszh.exe` and becomes the new start-menu / desktop shortcut
target. On every cold start it:

1. Reads the installed game's `VS_VERSION_INFO` to get the
   currently installed Zulu version.
2. Fetches `https://storage.googleapis.com/zulu-installer/latest.json`
   to see what's been published.
3. If the published version is strictly greater than what's
   installed (so dev builds never get rolled back), prompts the
   user, downloads the new installer to `%TEMP%`, runs it silently
   (`/S /D=<install dir>`), waits for it to finish, then continues
   into `launchGame()` with the original launcher argv intact.
4. If versions match or the user declines, hands off directly to
   `generalszh.exe` with the launcher's argv.

## Why

Zulu ships often. Without auto-update, every player needs to
manually visit a download page when they want the new client,
which means LAN sessions split between a host running v1.1.5 and
players still on v1.1.3 — the exeCRC check then refuses to let
them play together. The launcher closes that gap: hit play, get
the latest, join the lobby.

The argv preservation work (PR — `2d129251f`) fixes a subtle
problem from the original launcher commit. The first version
handed control to the NSIS installer's silent post-install `Exec`
line, which used the start-menu shortcut's hardcoded `LAUNCHARGS`
("-mod Zulu.big"). Any extra args the user passed on the
launcher command line were dropped. The fix has the calling
launcher wait for the elevated installer (`ShellExecuteEx` with
`SEE_MASK_NOCLOSEPROCESS`), then fall through to the normal
`launchGame` path with the launcher's original argv. NSIS's silent
post-install `Exec` is removed so it doesn't race the calling
launcher.

## Code surface

- `launcher/ZuluLauncher.cpp` — ~380 lines. Manifest, version
  compare, JSON parsing (minimal — only the `version` and `url`
  fields are read), WinHTTP download with progress, ShellExecuteEx
  wait-handoff, fallthrough to `CreateProcess(generalszh.exe, ...)`
  with original argv.
- `launcher/CMakeLists.txt` / `launcher/ZuluLauncher.rc.in` /
  `launcher/ZuluLauncher.manifest` — build, version-resource, and
  manifest plumbing.
- `installer/Zulu.nsi` — removes the silent post-install `Exec`
  and adds the manifest-publishing step.
- `Makefile` — `installer-release` target publishes the manifest
  with no-cache headers; downloads pinned to the
  `zulu-installer` GCS bucket as defense-in-depth.
- `GeneralsMD/Code/Main/RTS.RC` and the build glue around it pull
  `APPVERSION` from `installer/Zulu.nsi` so the EXE
  `VS_VERSION_INFO` and the installed-version check always agree.

## Concerns

- **Backwards compatibility.** Players upgrading from a pre-launcher
  build to a launcher-shipping build need to run the new installer
  once (shortcut still points at `generalszh.exe`). After the first
  install, the shortcut points at `ZuluLauncher.exe` and the
  manifest check kicks in.
- **Update channel risk.** All installs follow the same
  `latest.json` channel. There is no opt-out and no per-user
  channel selection. Dev builds protect themselves by having a
  version higher than the published manifest.
- **Mismatch / replay / save.** None — the launcher is a separate
  process and never touches game state.
- **Bucket lock-in.** The download URL is currently hardcoded to
  the `zulu-installer` GCS bucket. Moving providers would require
  a new launcher build to ship before the migration. Worth noting
  rather than urgent.
- **Failure-to-launch path.** If `latest.json` fetch fails (offline
  / DNS / 503), the launcher logs and falls through to launching
  the installed binary. The player isn't blocked.
