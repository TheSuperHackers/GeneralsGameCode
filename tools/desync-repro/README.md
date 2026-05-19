# Desync repro harness

This is what we have for testing the `MSG_CREATE_TEAM` desync fix. There is no
single-process, single-machine reproduction available, because the bug only
manifests during live multiplayer when at least two clients are running the
same buggy build and the runahead window is wide enough for game logic to
read a player's `m_squads` while it's mid-divergence. This harness raises
the per-minute probability enough to catch it within a few minutes.

## What's in here

- `repro.ahk` — AutoHotkey v1 script that spams `Ctrl+1`/`1`/`Ctrl+2`/`2`
  at the active game window. Toggle with `F8`, quit with `F12`.
- `check_desync.ps1` — PowerShell scanner that reads the `desyncGame` flag
  byte (offset `0x12`) out of every `.rep` in a Replays folder and prints a
  one-line verdict per file. Returns red `DESYNC` lines for any replay that
  recorded a CRC mismatch.

## Prerequisites

On the machine that runs the spam:

1. **AutoHotkey v1.1.x** (`https://www.autohotkey.com/`). The v2 syntax is
   incompatible; the script is written for v1.
2. **Clumsy** (`https://jagt.github.io/clumsy/`) for latency injection.
   Standalone, no installer, runs as Administrator.
3. The buggy build (i.e. `HEAD` before the `MSG_CREATE_TEAM` revert) and
   the patched build (`HEAD` with the revert applied), each capable of
   joining a multiplayer game with the other machine.

You need a second machine on the same LAN (or a Windows VM with bridged
networking) running the same build. The bug needs two peers; one machine
can't repro it.

## Running `repro.ahk` (operator instructions)

AutoHotkey is a Windows scripting tool that interprets `.ahk` files. The
script doesn't do anything by itself — AHK has to be installed first to
read it.

### One-time setup

1. Download AutoHotkey **v1.1.x** from `https://www.autohotkey.com/`. Use
   the v1.1 installer, **not** v2 — the script uses v1 syntax and won't run
   on v2.
2. Run the installer with default settings. This associates `.ahk` files
   with the AutoHotkey interpreter.

### Per-test run

1. Copy `repro.ahk` to the Windows test machine (the one you want to
   drive the input from).
2. Double-click `repro.ahk`. A green "H" icon appears in the system tray
   and a tray tip shows `F8 to toggle spam, F12 to quit. Currently OFF`.
3. Launch Generals, join the multiplayer game, manually select 3–5 of
   your own units (workers, infantry, anything you control).
4. Click the game window to give it foreground focus, then press **F8**.
   The tray tip switches to `Spam ON` and you'll see the hotkey numbers
   flicker in the unit selection bar at the bottom of the screen.
5. Play the game normally — build stuff, move units around. The script
   re-binds hotkey slots 1 and 2 to whatever you have selected at each
   tick, in the background.
6. **F8** again to pause the spam. **F12** to exit the script entirely.
   You can also right-click the tray icon and pick "Exit".

### Building a standalone `.exe`

If you'd rather not install AutoHotkey on the test machine (e.g. a clean
VM you want to keep minimal), AutoHotkey ships a converter:

1. On any machine with AutoHotkey installed: Start menu →
   `AutoHotkey` → `Convert .ahk to .exe`.
2. Source: `repro.ahk`. Destination: `repro.exe`. Click `Convert`.
3. Copy the resulting `repro.exe` to the test machine. It runs without
   AutoHotkey installed and behaves identically to the script.

### Troubleshooting

**F8 does nothing in-game.** The game must be in windowed or borderless
fullscreen mode (`Alt+Enter` to toggle). Exclusive-fullscreen DirectX 8
sometimes eats SendInput keystrokes. Also confirm the game window has
foreground focus — the script checks `WinActive("Command and Conquer")`
and silently no-ops when the game isn't focused.

**Window title doesn't match.** If you renamed your `.exe` or the
launcher gives the window a non-default title, the `WinActive(...)` check
in `repro.ahk` won't recognize it. Edit the line in `SpamCycle:` to match
whatever your window title actually says, e.g.
`if (!WinActive("Zulu"))`.

**Tray icon is missing.** Default AutoHotkey installs put the H icon in
the overflow ("show hidden icons") tray. Click the chevron in the system
tray to find it, or use Task Manager → Details → look for `AutoHotkey.exe`
to confirm the script is running.

**Script presses keys outside the game.** This shouldn't happen — the
script checks `WinActive` on every tick — but if it does, hit `F12`
immediately. The script will exit. Common cause: the game crashed but the
script is still running.

## Test procedure

### Phase 1 — confirm the harness can catch the bug

Use this against the **buggy** build first to verify the spam is actually
hitting the bug. If it doesn't repro on the buggy build, the harness isn't
working and a clean run on the patched build proves nothing.

1. Check out the buggy commit on both machines:
   ```
   git checkout 67f50d17b^      # one commit before the revert in this branch
   ```
   Then `make installer-release` (or whatever your Zulu build command is) and
   copy the resulting `generalszh_zulu.exe` + `Zulu.big` to both test
   machines.

2. On machine A (the spam machine), launch Clumsy as Administrator. Use
   the filter:
   ```
   udp and (outbound or inbound)
   ```
   Tick `Lag` and set it to `200` ms. Hit `Start`. This adds 200ms one-way
   latency to all UDP, which is what Generals uses for in-game peer
   traffic. Don't lag TCP — the game's matchmaking will time out.

3. Launch Generals on both machines, host a LAN game with machine A as the
   host (or as a client, either works), set up a 1v1, pick small maps
   like `Tournament Desert`, and start.

4. On machine A, manually select 3-5 of your own units (any units —
   workers, infantry, whatever). Leave them selected.

5. Switch to windowed mode if you aren't already (`Alt+Enter` should
   work). AutoHotkey is unreliable against exclusive-fullscreen DirectX 8
   surfaces.

6. Run `repro.ahk` (double-click it, or right-click → "Run script"). You
   should see a brief tray tip.

7. Click back into the game window to give it focus, then press `F8`. The
   tray tip should now say `Spam ON`. Watch the unit selection bar at the
   bottom — you'll see the hotkey numbers flicker as the script presses
   `Ctrl+1`, `1`, `Ctrl+2`, `2` on a loop.

8. Continue playing normally on both machines for 3–10 minutes. Build
   stuff, move units around, the script will keep re-binding the hotkey
   slots in the background.

9. Watch for one of the two clients to pop the "CRC mismatch" dialog
   (it loads `Menus/CRCMismatch.wnd`). When it appears, the test machine
   has caught the bug. Note which machine popped it.

10. End the game on both machines. The host's `.rep` will be in
    `%USERPROFILE%\Documents\Command and Conquer Generals Zero Hour Data\Replays\`.

11. On machine A, run:
    ```
    powershell -ExecutionPolicy Bypass -File tools\desync-repro\check_desync.ps1
    ```
    Confirm the latest replay shows a red `DESYNC` line.

If you don't get a desync within ~10 minutes:

- Increase Clumsy latency to 300ms.
- Make sure you're actually picking up new units when the spam runs (the
  spam binds whatever is currently selected — if you let it run with
  nothing selected, the CREATE_TEAM messages are empty and the bug can't
  fire because there's nothing to diverge over).
- Try a faction with `ReplaceObjectUpgrade` structures (Demo GLA, Stealth
  GLA, Chemical GLA — their Command Centers, Supply Stashes, etc. all
  use `ReplaceObjectUpgrade`) and trigger one of those upgrades while
  the spam is hammering. That's the most direct path into the
  `m_squads`-reading code that turns the divergence into a CRC mismatch.

### Phase 2 — verify the fix

Once Phase 1 confirms you can reproduce the bug, switch to the patched
build:

```
git checkout <this branch>      # the one with the revert + Network.cpp tweak
```

Rebuild on both machines, repeat Phase 1 steps 3–11. The test passes if
both clients stay in sync for the full duration of the spam run, and
`check_desync.ps1` shows no `DESYNC` line on either machine's replay
folder.

I'd run the patched build for at least 2x as long as the worst-case repro
time you observed in Phase 1, just to be sure. If you saw the bug in ~3
minutes on Phase 1, run ~10 minutes on Phase 2.

### Phase 3 — long-run sanity

Optional: leave the spam running through a full game (20–40 minutes) on
the patched build with Clumsy still injecting 200ms. This is the
high-confidence smoke test that the fix doesn't just push the problem
further out. If a full-length game completes without any CRC mismatch
dialog, ship it.

## Reading the `.rep` desync flag manually

If you want to spot-check a single replay without PowerShell:

```
certutil -f -encodehex "path\to\replay.rep" hex.txt 0x40
type hex.txt
```

Look at the very first row. Byte 0x12 (the 19th byte from the start) is
the desync flag. `00` = clean, `01` = recorded a CRC mismatch sometime
during the game.

## What this can't test

This harness exercises the input-driven path. It does **not** exercise:

- ReplaceObjectUpgrade's `getSquadNumberForObject` read in isolation
  (that requires an upgrade to actually fire on a unit that was just
  grouped — the spam approximates this statistically but doesn't force
  it).
- The optimistic-hotkey-table behavior from Option C. When Option C
  lands, you'd want a separate test that verifies the local UI selects
  the new team within one render frame of `Ctrl+N` while the network
  copy is still in flight. That test is best done by eye, possibly with
  a frame-counting overlay.
- Anything specific to a particular GenTool / Sentry / EOSSDK overlay
  combo. If your repro environment has those running, leave them on so
  you exercise the same surface area as the real bug report.
