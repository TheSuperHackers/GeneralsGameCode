# macOS Port — Changelog

## Current Status (2026-02-22)

🎉 **GAME IS PLAYABLE!** — Full game loop stable: shell map, cutscenes, mission loading, 3D terrain with units/buildings. 5500+ loop iterations without crash. Audio stubbed (no sound), some textures render white.

---

## Resolved Runtime Issues (Phase 12) — Game Loop Stabilization ⭐ MAJOR MILESTONE

### #18: SIGSEGV in MacOSAudioManager::processRequestList()
- **Symptom:** Game crashed on 3rd frame update in execute() loop. Exit code 139 (SIGSEGV).
- **Root Cause:** `AudioEventRTS` pointers in the audio request queue were corrupted/dangling. When `processRequestList()` tried to access `getEventName().str()`, `AsciiString::str()` dereferenced invalid memory.
- **Fix:** Stubbed `processRequestList()` — clears the request queue without accessing event data. Audio is non-functional but game loop is stable.
- **Side effect:** No audio (music, effects, voice). Needs AudioEventRTS lifetime investigation.
- **Files:** `Platform/MacOS/Source/Audio/MacOSAudioManager.mm`

### #19: macOS Silent Process Termination
- **Symptom:** Game process disappeared ~10s after menu loaded. No crash signal, no log output, exit code 0.
- **Root Cause (2 issues):**
  1. **Automatic Termination:** macOS considered the game "idle" (no NSRunLoop activity) and silently terminated it.
  2. **NSApp terminate:** `[NSApp terminate:nil]` calls `exit(0)` by default during event pumping — uninterceptable without a delegate.
- **Fix:**
  - `[[NSProcessInfo processInfo] disableAutomaticTermination:@"Game is running"]`
  - `[[NSProcessInfo processInfo] disableSuddenTermination]`
  - Added `applicationShouldTerminate:` → `NSTerminateCancel` on app delegate
  - Set NSApp delegate before `finishLaunching`
- **Files:** `Platform/MacOS/Source/Main/MacOSWindowManager.mm`

### #20: FramePacer Null Dereference
- **Symptom:** Potential SIGSEGV in `FramePacer::isActualFramesPerSecondLimitEnabled()`
- **Root Cause:** `TheScriptEngine` accessed without null check when `TheTacticalView != nullptr`. Also `TheGlobalData->m_useFpsLimit` accessed without null check.
- **Fix:** Added null safety for `TheScriptEngine` and `TheGlobalData`
- **Files:** `Core/GameEngine/Source/Common/FramePacer.cpp`

### #21: nextDrawable VSync Blocking
- **Symptom:** `[CAMetalLayer nextDrawable]` blocked indefinitely when window inactive (isActive=0)
- **Root Cause:** `displaySyncEnabled=YES` caused nextDrawable to wait for VSync. When window was not on screen, no VSync → infinite wait.
- **Fix:** Set `displaySyncEnabled = NO`. Frame rate controlled by `FramePacer`. Added nil guard on drawable.
- **Files:** `Platform/MacOS/Source/Metal/MetalDevice8.mm`

### #22: Signal Handlers for Crash Debugging
- **Feature:** Installed `sigaction`-based handlers for SIGSEGV, SIGBUS, SIGABRT with `backtrace()` output
- **Files:** `Platform/MacOS/Source/Main/MacOSWindowManager.mm`

---

## Resolved Runtime Issues (Phase 11) — 3D Object Loading ⭐

### #17: 3D Objects Not Spawning — Wrong GameLogic Factory
- **Symptom:** Terrain visible but no units, buildings, or objects in the game world.
- **Root Cause:** `createGameLogic()` returned `GameLogic` instead of `W3DGameLogic`. This caused `createTerrainLogic()` to return `TerrainLogic` instead of `W3DTerrainLogic`. `TerrainLogic::loadMapData()` skips the `ObjectsList` chunk from `.map` files → 0 objects parsed.
- **Fix:** Return `NEW W3DGameLogic()` from factory in `MacOSMain.mm`
- **Result:** 771 MapObjects loaded, units and buildings spawn in shell map and missions
- **Files:** `Platform/MacOS/Source/Main/MacOSMain.mm`

---

## Resolved Runtime Issues (Phase 10) — Terrain Textures ⭐

### #15: Terrain Textures Black (RESOLVED)
- **Symptom:** 3D terrain draw calls execute but terrain appeared entirely black
- **Root Cause:** `MetalSurface8::UnlockRect()` was not properly uploading texture data to the parent `MetalTexture8`'s GPU texture. Terrain tile textures were locked/edited on CPU but changes never reached the GPU.
- **Fix:** Proper texture upload pipeline through `MetalSurface8` → parent `MetalTexture8` → `replaceRegion`
- **Files:** `Platform/MacOS/Source/Metal/MetalSurface8.mm`, `MetalTexture8.mm`

### #16: Incorrect Pixel Format Mapping
- **Symptom:** Texture corruption, wrong colors for some texture formats
- **Fix:** Refined `MetalFormatFromD3D()` mapping for key D3D formats (A8R8G8B8, X8R8G8B8, A4R4G4B4, R5G6B5, A1R5G5B5, DXT1-5)
- **Files:** `Platform/MacOS/Source/Metal/MetalDevice8.mm`

---

## Resolved Runtime Issues (Phase 9) — Shell Map Loading ⭐ MAJOR FIX

### #14: Shell Map Not Loading — Movie State Machine Broken on macOS
- **Symptom:** Main menu displayed but with black background. No 3D shell map scene rendered.
- **Root Cause (3 issues):**
  1. **Intro/sizzle state machine broken:** `VideoPlayer::open()` returns `nullptr` on macOS (no video player).
  2. **`showShellMap(TRUE)` never called:** Because the state machine never reached the shell path code.
  3. **`m_breakTheMovie = TRUE` blocked ALL rendering:** `W3DDisplay::draw()` checks this flag before `WW3D::Begin_Render()`.
- **Fix (in `MacOSGameClient::update()`):** Bypass movie state machine, call `TheShell->showShellMap(TRUE)` directly.
- **Files:** `Platform/MacOS/Source/Main/MacOSGameClient.mm`

---

## Resolved Runtime Issues (Phase 8) — Keyboard Input ⭐

### #13: Keyboard Input Not Working — Two Root Causes
- **Root Cause 1:** Window's `contentView` lacked `acceptsFirstResponder` override
- **Root Cause 2:** `StdKeyboard::update()` was empty — never called `Keyboard::update()`
- **Fix:** `GameContentView` with full key handling + `Keyboard::update()` call
- **Files:** `MacOSWindowManager.mm`, `StdKeyboard.mm`

---

## Resolved Runtime Issues (Phase 7) — Full UI Rendering ⭐

### #12: Full UI Rendering — W3DGameWindowManager Integration
- Changed inheritance to `W3DGameWindowManager` for proper gadget rendering
- **Files:** `MacOSGameWindowManager.h`, `MacOSGameWindowManager.mm`

### #11: Invisible UI Text — Back-Face Culling ⭐
- Metal's default CW front-face + backface culling discarded all 2D triangles after Y-flip
- **Fix:** `setCullMode:MTLCullModeNone` for XYZRHW vertices
- **Files:** `Platform/MacOS/Source/Metal/MetalDevice8.mm`

---

## Resolved Runtime Issues (Phase 6) — UI Rendering

### #10: UI Buttons, TSS Pipeline, Fog/Lighting
- Implemented full TextureStageState evaluation in Metal fragment shader
- Per-PSO blend state caching, depth/fog/lighting uniforms

---

## Resolved Runtime Issues (Phase 5) — 10 Crashes Fixed

| # | Issue | Root Cause | Fix |
|---|---|---|---|
| 1 | SIGBUS in AudioManager::init() | AVAudioEngine exception | @try/@catch |
| 2 | SIGSEGV in parseModuleName | Wrong factory type | W3DModuleFactory |
| 3 | SIGBUS in GameClient::init() | Vtable mismatch (missing zi_always) | CMake deps |
| 4 | ERROR_INVALID_D3D | LoadLibrary returned nullptr | Metal marker |
| 5 | ERROR_OUT_OF_MEMORY | Missing memory pool entries | Pool table update |
| 6 | SIGSEGV GameResultsInterface | nullptr returned | Stub class |
| 7 | Shader/asset paths | Wrong search paths | Path resolution |
| 8 | SIGSEGV audio playback | Uninitialized AVAudioEngine | @try/@catch + guard |
| 9 | SIGABRT malloc/free | Global alloc override vs Metal | calloc/free on macOS |
| 10 | SIGSEGV W3DBridgeBuffer | Uninitialized m_numBridges | Init to 0 + calloc |

---

## Commit History

| Commit | Description |
|:---|:---|
| `426bd96c` | **fix(macos): Resolve game loop crash and stabilize gameplay** 🎉 ⭐ |
| (prev) | fix(macos): 3D object loading — W3DGameLogic factory |
| (prev) | fix(macos): Terrain texture pipeline — surface upload |
| `d8d58c12` | fix(macos): text rendering — disable back-face culling for 2D |
| `03100065` | fix(macos): UI buttons, Metal TSS/fog/lighting, audio playback |
| `838d93c7` | fix(macos): resolve 10 runtime crashes — stable game loop |
| `a2e7a7ba` | macOS: resolve all linker errors — successful build 🎉 |
| Earlier | CMake setup, compilation fixes, DX8→Metal stubs |

---

## Milestones

| Phase | Status | Description |
|:---|:---|:---|
| Phase 1: CMake Setup | ✅ Done | Build system, presets, dependency resolution |
| Phase 2: Compilation Fixes | ✅ Done | windows.h shims, type stubs, PCH config |
| Phase 3: DX8→Metal Stubs | ✅ Done | d3d8_stub.h, MetalDevice8, Metal shaders |
| Phase 4: Linker Resolution | ✅ Done | GameSpy stubs, Win32 stubs, 170+ functions |
| Phase 5: Runtime Debugging | ✅ Done | 10 init crashes fixed, stable runtime |
| Phase 6: UI Rendering | ✅ Done | Buttons visible, TSS evaluation, fog/depth/lighting |
| Phase 7: Full UI + Text | ✅ Done | W3DGameWindowManager, all UI widgets |
| Phase 8: Input System | ✅ Done | Keyboard + Mouse fully working |
| Phase 9: Shell Map + 3D | ✅ Done | Shell map loads, 3D draws confirmed |
| Phase 10: Terrain Textures | ✅ Done | Terrain rendering with proper textures |
| Phase 11: Object Loading | ✅ Done | W3DGameLogic → 771 objects loaded |
| Phase 12: Game Loop | ✅ Done | Stable loop, cutscenes, missions playable 🎉 |
| Phase 13: Audio | 🔲 Next | Fix AudioEventRTS lifetime, restore sound |
| Phase 14: Texture Polish | 🔲 Next | Fix white textures on 3D models |
