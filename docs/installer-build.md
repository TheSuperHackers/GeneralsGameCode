# Installer and Build System

A cluster of build-system changes that turn the upstream
"VS6 / VC6 / cl.exe" build into a Make-driven pipeline that
produces a versioned NSIS installer.

## `make installer-release` (PR #3 — `b362cb123`, `7f2c0e80b`)

The top-level `Makefile` builds the EXE, packs the `Zulu.big`
asset archive from `assets/`, and runs the NSIS installer
script. `installer-release` additionally publishes the result
plus the auto-update manifest (see
[launcher-auto-update.md](launcher-auto-update.md)) to the
release bucket.

`Zulu.big` is built directly from the asset tree by the
`big-zulu` target; the previous workflow required hand-editing
the archive contents.

## `BuildVersion.h` and APPVERSION plumbing (`692615395`, `d55f4c445`)

The replay header records the EXE version, and the exeCRC
mismatch check uses it to reject cross-version play (which is
what makes the Zulu replay-feature-version scheme safe — see
[tactical-ai.md](tactical-ai.md)). `BuildVersion.h` is generated
during the build with the `APPVERSION` string from
`installer/Zulu.nsi`, so a single source-of-truth bump in the
`.nsi` script propagates to:

- The replay header's recorded version.
- The EXE's `VS_VERSION_INFO` resource.
- The launcher's installed-version check.
- The NSIS installer's display version.

A subtle dependency the build was missing: `RTS.RC` references
`BuildVersion.h` via `#include`, but `RTS.RC`'s own mtime doesn't
change when the header content does. Ninja therefore skipped
recompiling `RTS.RC.res` on version bumps, leaving the EXE's
version resource stale. Fix (`d55f4c445`): add an
`OBJECT_DEPENDS BuildVersion.h` on the RC target so ninja sees
the dep edge.

## `cmcldeps` disable for RC compiles (`4c2d9af78`)

CMake's Windows-MSVC platform module wraps the RC compile in
`cmcldeps`, which doesn't work right with the resource compiler
in our toolchain (mis-parsed preprocessor output, missing
includes). `CMAKE_NINJA_CMCLDEPS_RC` is disabled at the top of
`CMakeLists.txt` so RC compiles run through `rc.exe` directly.

The `BuildVersion.h` dep fix above is what makes this safe — with
`cmcldeps` disabled, ninja has no auto-detection of `RC.RES`
dependencies, so the explicit `OBJECT_DEPENDS` is load-bearing.

## Concerns

- **Mismatch / replay / save.** None directly, but the version
  plumbing is what makes the existing replay-version system
  trustworthy. A failure where `BuildVersion.h` is out of sync
  with the shipped EXE would mean the replay header says "Zulu
  1.1.5" when the EXE is actually 1.1.4, which would silently
  let two binaries with different sim behavior into the same
  lobby. The RC dep fix exists specifically because we hit this
  drift in practice.
- **Build reproducibility.** `APPVERSION` is the only place a
  release version is set. CI bumps it once and every downstream
  consumer picks it up on rebuild. The launcher's
  `latest.json` is published with the same version string the
  installer was built with.
- **Backwards compatibility.** The installer is fully replaces
  any previous Zulu install in place. There is no per-version
  side-by-side install. This is by design; multi-version
  installs would defeat the auto-update flow and the exeCRC
  mismatch system both. Vanilla / non-Zulu installs are
  untouched.
- **Win32 only.** The `make installer-release` flow assumes
  `makensis.exe` is available and that the build is producing
  Win32 binaries via `cl.exe`. Linux / WSL development
  iterations are supported, but `installer-release` only runs to
  completion under MSYS/MinGW with NSIS installed.
