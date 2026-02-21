# macOS Port — Development Guide

This document covers architecture decisions, coding conventions, known gotchas, and golden rules for working on the macOS port.

---

## 📜 Golden Rules

1. **Minimize changes to `Core/`** — Platform code lives in `Platform/MacOS/Source/`. Only touch Core with `#ifdef __APPLE__` guards when absolutely necessary.
2. **Minimal `windows.h` shims** — Add stubs only when a build error demands it, never proactively.
3. **`d3d8_stub.h` is the source of truth** — All DX8 interfaces on macOS go through the stub, not the original SDK.
4. **Unified rendering pipeline** — All rendering goes through `MetalDevice8`. No side-channels.
5. **Iterate on crash logs** — Runtime issues are debugged iteratively: crash → log → fix → test.
6. **PRIVATE deps for `zi_always`** — Never let Zero Hour defines leak into the Generals target.
7. **Always `killall generalszh`** — Kill stale processes before launching; Metal layer doesn't release cleanly.
8. **`calloc`, not `malloc`** — On macOS, global allocations use calloc for zero-initialization.
9. **NEVER set `m_breakTheMovie = TRUE`** — `W3DDisplay::draw()` line 1849 checks this flag. When TRUE, `WW3D::Begin_Render()` is skipped and **all 3D rendering is disabled**.
10. **Use `printf` + `fflush(stdout)` for logs** — `fprintf(stderr)` may not appear in redirected logs due to buffering.

---

## 🏗 Architecture

### Component Map

| Subsystem | Files | Purpose |
|:---|:---|:---|
| **Metal Backend** | `Source/Metal/MetalDevice8.mm` (85KB+), 5 pairs .h/.mm | DX8 → Metal translator |
| **Entry Point** | `Source/Main/MacOSMain.mm` | NSApplication, game loop, factory functions |
| **Game Client** | `Source/Main/MacOSGameClient.mm` | W3D-compatible game client, shell map bypass |
| **Window** | `Source/Main/MacOSWindowManager.mm` | NSWindow/NSView management |
| **Input** | `Source/Main/StdKeyboard.mm`, `StdMouse.mm` | Cocoa events → game input |
| **Audio** | `Source/Audio/MacOSAudioManager.mm` | AVAudioPlayer backend |
| **Display** | `Source/Client/MacOSDisplay.mm` | CoreText rendering, W3D integration |
| **D3DX Shims** | `Source/Main/D3DXStubs.mm` | `D3DXCreateTextureFromFileEx` etc. |
| **Stubs** | `Source/Stubs/GameSpyStubs.cpp` | 170+ network/Win32 function stubs |
| **Shaders** | `Source/Main/MacOSShaders.metal` | Metal vertex/fragment shaders |

### Shell Map Loading (macOS specific)

On macOS there's no video player. The intro/sizzle movie state machine in `GameClient::update()` doesn't complete properly. `MacOSGameClient::update()` bypasses it:

```
callCount == 0:
  m_playIntro = FALSE
  m_afterIntro = FALSE
  → GameClient::update()    (state machine skipped)
  → TheShell->showShellMap(TRUE)  (loads ShellMapMD.map)
  → TheShell->showShell()        (pushes MainMenu.wnd)
```

**Key insight:** On Windows, `W3DGameClient::update()` also just calls `GameClient::update()` — no extra logic. The movie state machine works on Windows because `VideoPlayer::open()` returns a valid stream.

### Stubs Overview

`GameSpyStubs.cpp` disables all online functionality (~170 functions):
- **Multiplayer:** Online, LAN lobby — not functional
- **Patch system:** Disabled
- **IME:** Disabled
- **Implication:** Engine runs in **single-player mode only**

---

## ⚠️ Critical Gotchas

### 1. Vtable Mismatch (CMake)

`macos_platform` is a STATIC library linked by BOTH targets (Generals and Zero Hour).

**Problem:** Zero Hour's `GameClient.h` has 2 extra virtual methods (`notifyTerrainObjectMoved`, `createSnowManager`) guarded by `#if RTS_ZEROHOUR`. If `RTS_ZEROHOUR` is not defined when compiling `MacOSGameClient.mm`, the vtable layout differs from what the engine library expects → vtable dispatch jumps to wrong addresses.

**Solution:**
- `zi_always` (provides `RTS_ZEROHOUR=1`) must be a **PRIVATE** dependency of `macos_platform`
- Include paths must point to `GeneralsMD/` (not `Generals/`)
- See `Platform/MacOS/CMakeLists.txt`

### 2. Global Memory Allocator Conflict

The game overrides global `operator new`/`delete` to route through `DynamicMemoryAllocator`, which prepends a `MemoryPoolSingleBlock` header.

**Problem on macOS:** System frameworks (Metal, AppKit, libdispatch) allocate with system `malloc` but free through our overridden `delete`. The allocator tries to read the header from system-allocated memory → crash.

**Solution:**
```cpp
#ifdef __APPLE__
void* operator new(size_t size) {
    void *p = ::calloc(1, size);  // Zero-init! Game relies on this.
    if (!p) throw std::bad_alloc();
    return p;
}
void operator delete(void *p) noexcept {
    ::free(p);
}
#endif
```

**Why `calloc`?** The original custom allocator zeroed memory on allocation. Many game classes have constructors that don't initialize all members (relying on zeroed memory). `malloc` would leave garbage → crashes in constructors like `W3DBridgeBuffer`, `Pathfinder`, etc.

### 3. Null Globals During Shell Phase

During menu/shell phase, many game globals are null: `TheGameLogic`, `TheInGameUI`, `TheTacticalView`, `TheScriptEngine`, `TheTerrainVisual`. Additionally, `TheTerrainVisual` is explicitly set to null after init throws `ERROR_BUG`.

**Pattern:** Always guard access:
```cpp
if (!TheGameLogic || !TheInGameUI) return;
```

### 4. `win_compat.h` / Metal Header Conflicts

Windows API stubs (`LoadResource`, `GetCurrentThread`) conflict with macOS system headers.

**Solutions used:**
1. `MetalDevice8.h` uses `void*` for all Metal/ObjC types (avoids importing `<Metal/Metal.h>`)
2. In `.mm` files, macOS framework headers imported **before** `win_compat.h`
3. Conflicting functions wrapped in `#if !defined(__OBJC__)`

### 5. Mouse Coordinate 2x Scaling

`Mouse::reset()` sets `m_inputMovesAbsolute = FALSE` (relative mode). In relative mode, `moveMouse()` adds coords to position. Since `StdMouse::addEvent()` also sets position = coords, the position gets doubled.

**Fix:** `StdMouse::reset()` restores `m_inputMovesAbsolute = TRUE` after calling base `Mouse::reset()`.

### 6. m_breakTheMovie Flag

`W3DDisplay::draw()` (W3DDisplay.cpp line 1849) checks:
```cpp
if ((TheGlobalData->m_breakTheMovie == FALSE) && (TheGlobalData->m_disableRender == false) && WW3D::Begin_Render(...) == WW3D_ERROR_OK)
```

If `m_breakTheMovie` is TRUE, **no 3D rendering happens**. On Windows this flag is used during movie playback to suppress rendering. On macOS, since we have no movie player, this flag should **never** be set to TRUE.

### 7. stderr vs stdout in Log Files

When redirecting output with `> file 2>&1`, `fprintf(stderr)` output may not appear in the log due to buffering differences. Always use `printf()` + `fflush(stdout)` for diagnostic logs.

---

## 🔄 Development Workflow

### Typical cycle

```
1. Identify crash/issue (from logs or stack trace)
2. Analyze root cause (grep, view code, check headers)
3. Implement fix (minimal, often with #ifdef __APPLE__)
4. Build:  ninja -j$(sysctl -n hw.ncpu) -C build/macos GeneralsMD/generalszh
5. Test:   kill $(pgrep generalszh) 2>/dev/null
           export GENERALS_INSTALL_PATH="/Users/okji/dev/games/Command and Conquer - Generals/Command and Conquer Generals/"
           build/macos/GeneralsMD/generalszh > Platform/MacOS/Build/Logs/game.log 2>&1 &
           sleep 20; grep "KEYWORD" Platform/MacOS/Build/Logs/game.log
6. Check:  grep "Signal received" Platform/MacOS/Build/Logs/game.log
7. Commit: git add -A && git commit -m "fix(macos): ..."
```

### Debugging with lldb

```bash
kill $(pgrep generalszh) 2>/dev/null
cd build/macos/GeneralsMD
lldb ./generalszh
# In lldb:
(lldb) run
# After crash:
(lldb) bt         # backtrace
(lldb) frame select 2
(lldb) p variable
```

### Useful log analysis

```bash
# Last lines before crash
tail -30 Platform/MacOS/Build/Logs/game.log

# Check rendering frames
grep "Present #" Platform/MacOS/Build/Logs/game.log | tail -5

# Check 3D rendering
grep "fvf=0x252\|DIP_3D" Platform/MacOS/Build/Logs/game.log | head -10

# Check shell map status
grep "SHELLMAP:" Platform/MacOS/Build/Logs/game.log

# Monitor FPS  
grep "W3DDisplay::draw" Platform/MacOS/Build/Logs/game.log | tail -5

# Check subsystem init
grep "initSubsystem" Platform/MacOS/Build/Logs/game.log
```

---

## 📋 Backlog

| Task | Priority | Notes |
|:---|:---|:---|
| Terrain textures black | **Critical** | 3D draws confirmed but texture data appears black |
| Cursor texture | **Medium** | Green square instead of proper cursor texture |
| Crash on exit | **Medium** | SIGSEGV exit code 11, inconsistent |
| Audio playback | **Medium** | Wrapped in @try/@catch, needs .big file loading verification |
| .big archives mounting | **Medium** | Asset loading works (INI files load), completeness unverified |
| WOL authorization | Low | Browser excluded. Possibly REST API. |
| Cross-platform LAN | Low | wchar_t 4B on macOS vs 2B on Windows |

---

## 📚 External References

| Project | Link | Description |
|:---|:---|:---|
| **TheSuperHackers** | [GitHub](https://github.com/TheSuperHackers/GeneralsGameCode) | Upstream, modernized C++20 |
| **Fighter19 (Linux)** | [GitHub](https://github.com/Fighter19/CnC_Generals_Zero_Hour) | Native Linux port reference |
| **GeneralsGamePatch** | [GitHub](https://github.com/TheSuperHackers/GeneralsGamePatch/) | Game data & assets |
