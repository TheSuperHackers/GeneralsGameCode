# macOS Port — Changelog

## Current Status (2026-02-20)

🟢 **Full UI + Input Complete!** — All menu elements render with original W3D textures. Keyboard fully connected (Escape, arrows, F-keys, all letters/numbers). Mouse input working. 3D terrain geometry renders but textures not binding (black terrain). Next: fix terrain texture binding.

---

## Resolved Runtime Issues (Phase 8) — Keyboard Input ⭐

### #13: Keyboard Input Not Working — Two Root Causes
- **Symptom:** All keyboard presses ignored. NSEvents captured (`INPUT: KeyDown` in logs) but game didn't react
- **Root Cause 1:** Window's `contentView` was a plain `NSView` without `acceptsFirstResponder` override → macOS didn't deliver keyboard events reliably
- **Fix 1:** Created `GameContentView` (NSView subclass) with `acceptsFirstResponder = YES`, `keyDown:`/`keyUp:` overrides. Set as window's contentView and firstResponder. Added `keyDown:` override on `MacOSWindow` to prevent system beep.
- **Root Cause 2:** `StdKeyboard::update()` was **empty** — never called `Keyboard::update()`. The base class `update()` is what triggers `updateKeys()` → `getKey()` → reads from ring buffer → populates `m_keys[]`. Without it, `createStreamMessages()` produced zero `MSG_RAW_KEY_DOWN/UP` messages.
- **Fix 2:** Added `Keyboard::update()` call in `StdKeyboard::update()`
- **Bonus:** Extended key mapping with F1-F12, Delete, Home, End, PageUp/Down, Period, NumpadEnter
- **Files:** `MacOSWindowManager.mm`, `StdKeyboard.mm`

---

## Resolved Runtime Issues (Phase 7) — Full UI Rendering ⭐ MAJOR BREAKTHROUGH

### #12: Full UI Rendering — W3DGameWindowManager Integration
- **Symptom:** Dialog frames missing, dropdowns empty, static graphics absent, buttons had oversaturated colors
- **Root Cause:** `MacOSGameWindowManager` inherited from `GameWindowManager` and returned simplified `MacOSGadget*Draw` functions that only drew colored rectangles (`DrawBeveledRect`) without textures/images
- **Fix:** Changed inheritance to `W3DGameWindowManager` which provides the original `W3DGadget*Draw` functions (push buttons, combo boxes, list boxes, sliders, progress bars, etc.) that properly render through `TheWindowManager->winDrawImage()` → `Render2DClass` → `DX8Wrapper` → `MetalDevice8`
- **Additional Fix:** Created `MacOSGameWindow` subclass of `W3DGameWindow` to safely handle `nullptr fontData` (since macOS uses CoreText/`MacOSDisplayString` instead of Windows GDI/`FontCharsClass`)
- **Files:** `MacOSGameWindowManager.h`, `MacOSGameWindowManager.mm`, `GameMemoryInitPools_GeneralsMD.inl`

---


## Resolved Runtime Issues (Phase 7 earlier) — Text Rendering

### #11: Invisible UI Text — Back-Face Culling ⭐ KEY FIX
- **Symptom:** All button text (SOLO PLAY, MULTIPLAYER, etc.) was completely invisible despite textures being created and DrawPrimitiveUP being called
- **Root Cause:** The vertex shader flips Y for screen-space (XYZRHW) coordinates: `screenPos.y = 1.0 - y/screenH * 2.0`. This reverses triangle winding order from CW to CCW in NDC. With Metal's default CW front-face winding + back-face culling enabled, **all 2D triangles were silently discarded**
- **Fix:** Force `setCullMode:MTLCullModeNone` for XYZRHW vertices in `DrawPrimitiveUP`, applied after `ApplyPerDrawState()`
- **Files:** `Platform/MacOS/Source/Metal/MetalDevice8.mm`

## Resolved Runtime Issues (Phase 6) — UI Rendering

### #10.1: UI Buttons Not Visible
- **Symptom:** Menu buttons appeared as empty rectangles without visible borders
- **Root Cause:** Missing TSS evaluation in fragment shader, incorrect blend states
- **Fix:** Implemented full TextureStageState evaluation (SELECTARG1/2, MODULATE, ADD), per-PSO blend state caching, depth/fog/lighting uniforms

### #10.2: FPS Text Position (y=-1)
- **Symptom:** FPS counter text rendered at y=-1 (1 pixel off-screen top)
- **Status:** Cosmetic — text partially visible at top-left. Not blocking.

---

## Resolved Runtime Issues (Phase 5) — 10 total

### #1: SIGBUS in `MacOSAudioManager::init()`
- **Symptom:** Crash on startup when no audio output available
- **Root Cause:** `AVAudioEngine` threw an unhandled Objective-C exception
- **Fix:** Wrapped initialization in `@try/@catch`, engine continues with audio disabled

### #2: SIGSEGV in `ThingTemplate::parseModuleName`
- **Symptom:** Null pointer dereference during INI parsing
- **Root Cause:** `createModuleFactory()` returned base `ModuleFactory` instead of `W3DModuleFactory`
- **Fix:** Changed return type in `MacOSMain.mm`

### #3: SIGBUS in `GameClient::init()` at `setFrameRate()` ⭐ KEY FIX
- **Symptom:** `EXC_BAD_ACCESS (code=2)` — reading typeinfo data as code
- **Root Cause:** **Vtable mismatch.** `macos_platform` CMake target was missing `zi_always` dependency → `RTS_ZEROHOUR` undefined → two virtual methods excluded from vtable → 2-slot offset in vtable dispatch
- **Fix:** Added `zi_always` + `zi_gameengine_include` as PRIVATE deps in `Platform/MacOS/CMakeLists.txt`, switched include paths from `Generals/` to `GeneralsMD/`

### #4: ERROR_INVALID_D3D in `DX8Wrapper::Init()`
- **Symptom:** D3D8 device creation failed — `LoadLibrary`/`GetProcAddress` returned nullptr
- **Fix:** `LoadLibrary` returns marker `(HMODULE)0x1`, `GetProcAddress("Direct3DCreate8")` routes to `CreateMetalInterface8`

### #5: ERROR_OUT_OF_MEMORY (0xDEAD0002)
- **Symptom:** Memory pool exhaustion for Metal resource classes
- **Root Cause:** Memory pools for `MetalSurface8` and `MetalTexture8` missing from pool size table
- **Fix:** Added pool entries under `#ifdef __APPLE__` in `GameMemoryInitPools_GeneralsMD.inl`

### #6: SIGSEGV in `GameResultsInterface`
- **Symptom:** Null pointer dereference in `initSubsystem`
- **Root Cause:** `createNewGameResultsInterface()` returned nullptr
- **Fix:** Created `StubGameResultsInterface` with no-op methods in `GameSpyStubs.cpp`

### #7: Shader/Asset path resolution
- **Symptom:** `MacOSShaders.metal` and `.big` files not found at runtime
- **Fix:** Added search paths in `MetalDevice8.mm`, created symlinks in build directory

### #8: SIGSEGV in audio playback (`scheduleFile`)
- **Symptom:** Crash when playing audio with uninitialized engine or incompatible format
- **Fix:** Added `@try/@catch` block and `isRunning` guard in `friend_forcePlayAudioEventRTS`

### #9: SIGABRT — Metal driver vs custom allocator ⭐ CRITICAL FIX
- **Symptom:** `BUG_IN_CLIENT_OF_LIBMALLOC_POINTER_BEING_FREED_WAS_NOT_ALLOCATED`
- **Root Cause:** Global `operator new/delete` override routed ALL allocations through `DynamicMemoryAllocator`. Metal/AppKit/libdispatch use system malloc but free through our overridden `delete` — crash
- **Fix:** On macOS, global `operator new` uses `calloc(1, size)` and `operator delete` uses `free()`
- **Files:** `Core/GameEngine/Source/Common/System/GameMemory.cpp`

### #10: SIGSEGV in `W3DBridgeBuffer` / `Pathfinder` constructors
- **Symptom:** Crash in `clearAllBridges()` — iterating over garbage data
- **Root Cause:** `m_numBridges` uninitialized before `clearAllBridges()` call in constructor
- **Fix:** Initialize `m_numBridges=0` before `clearAllBridges()`. Also fixed systemically by using `calloc` (issue #9)

---

## Commit History

| Commit | Description |
|:---|:---|
| `d8d58c12` | **fix(macos): text rendering — disable back-face culling for 2D/XYZRHW draws** ⭐ |
| `03100065` | fix(macos): UI buttons, Metal TSS/fog/lighting, audio playback |
| `838d93c7` | fix(macos): resolve 10 runtime crashes — stable 35s game loop |
| `a2e7a7ba` | **macOS: resolve all linker errors — successful build 🎉** (31 files, +1121/-81) |
| `ac60483f` | fix: resolve macos_platform compilation errors — Carbon compat, D3DX stubs, overrides |
| `3b3130e5` | fix(macos): W3DDisplay, W3DGameEngine, windows.h stubs — build fixes |
| `0edf1903` | fix(macos): Replace DX8 SDK with d3d8_stub.h + shim d3dx8 headers |
| `1f33c17a` | fix(macos): Resolve compilation errors (windows.h shims, PCH, exclusions) |
| `2dfbe0e4` | feat(macos): Clean restart — Platform/MacOS/ (78 files), CMake skeleton |

---

## Milestones

| Phase | Status | Description |
|:---|:---|:---|
| Phase 1: CMake Setup | ✅ Done | Build system, presets, dependency resolution |
| Phase 2: Compilation Fixes | ✅ Done | windows.h shims, type stubs, PCH config |
| Phase 3: DX8→Metal Stubs | ✅ Done | d3d8_stub.h, MetalDevice8, Metal shaders |
| Phase 4: Linker Resolution | ✅ Done | GameSpy stubs, Win32 stubs, 170+ functions |
| Phase 5: Runtime Debugging | ✅ Done | 10/10 init crashes fixed, stable runtime |
| Phase 6: UI Rendering | ✅ Done | Buttons visible, TSS evaluation, fog/depth/lighting |
| Phase 7: Full UI + Text | ✅ Done | W3DGameWindowManager integration, all UI widgets render with textures, 3D shell map |
| Phase 8: Gameplay Scene | 🔲 Next | Terrain rendering, units, buildings, camera, gameplay loop |
