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

---

## 🏗 Architecture

### Component Map

| Subsystem | Files | Purpose |
|:---|:---|:---|
| **Metal Backend** | `Source/Metal/MetalDevice8.mm` (85KB+), 5 pairs .h/.mm | DX8 → Metal translator |
| **Entry Point** | `Source/Main/MacOSMain.mm` | NSApplication, game loop, factory functions |
| **Game Client** | `Source/Main/MacOSGameClient.mm` | W3D-compatible game client |
| **Window** | `Source/Main/MacOSWindowManager.mm` | NSWindow/NSView management |
| **Input** | `Source/Main/StdKeyboard.mm`, `StdMouse.mm` | Cocoa events → game input |
| **Audio** | `Source/Audio/MacOSAudioManager.mm` | AVAudioEngine backend |
| **Display** | `Source/Client/MacOSDisplay.mm` | CoreText rendering, W3D integration |
| **D3DX Shims** | `Source/Main/D3DXStubs.mm` | `D3DXCreateTextureFromFileEx` etc. |
| **Stubs** | `Source/Stubs/GameSpyStubs.cpp` | 170+ network/Win32 function stubs |
| **Shaders** | `Source/Main/MacOSShaders.metal` | Metal vertex/fragment shaders |

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

---

## 🔄 Development Workflow

### Typical cycle

```
1. Identify crash/issue (from logs or stack trace)
2. Analyze root cause (grep, view code, check headers)
3. Implement fix (minimal, often with #ifdef __APPLE__)
4. Build:  cmake --build build/macos
5. Test:   killall generalszh 2>/dev/null; sleep 1
           build/macos/GeneralsMD/generalszh > /tmp/test.log 2>&1 &
           sleep 30 && tail -20 /tmp/test.log
6. Check:  grep "Signal received" /tmp/test.log
7. Commit: git add -A && git commit -m "fix(macos): ..."
```

### Debugging with lldb

```bash
killall generalszh 2>/dev/null; sleep 1
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
# Count crashes
grep -c "Signal received" /tmp/test.log

# Check rendering is working
grep "Present #" /tmp/test.log | tail -5

# Check subsystem init
grep "initSubsystem" /tmp/test.log

# Monitor heartbeats
grep "heartbeat" /tmp/test.log | tail -5
```

---

## 📋 Backlog

| Task | Priority | Notes |
|:---|:---|:---|
| Input handling | **High** | Keyboard/Mouse via Cocoa events — verify event routing |
| Metal rendering full | **High** | Textures, shaders, 3D scene — verify under real rendering |
| UI / menu rendering | **High** | Interface elements, buttons, text — verify drawing |
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
