### 🚀 Handoff Prompt for macOS Port (ARM64)

**Context:** We are porting Command & Conquer Generals: Zero Hour to macOS (ARM64/Apple Silicon).
**Current Milestone:** Phase 5 (Runtime Debugging) — Game launches, runs stable 35+ seconds! Metal renders 400+ frames, zero crashes!

**Technical Status:**
- **Build Outcome:** Both `generalsv` (21MB, Generals) and `generalszh` (22MB, Zero Hour) are Mach-O 64-bit arm64 executables.
- **Build System:** CMake (`cmake --preset macos` + `cmake --build build/macos`), Ninja generator.
- **Branch:** `feature/macos-c_make`
- **Runtime:** Game initializes all subsystems, Metal device renders 400+ frames, stable game loop. No crashes during idle runtime.

**Resolved Runtime Issues (Phase 5) — 10 total:**

1. **SIGBUS in `MacOSAudioManager::init()`** — ✅ FIXED
   - AVAudioEngine threw exception when no audio output was available.
   - Wrapped in `@try/@catch`, engine continues with audio disabled.

2. **SIGSEGV in `ThingTemplate::parseModuleName`** — ✅ FIXED
   - `TheModuleFactory` was null because `createModuleFactory()` returned base `ModuleFactory` instead of `W3DModuleFactory`.
   - Fixed in `MacOSMain.mm`.

3. **SIGBUS in `GameClient::init()` at `setFrameRate()`** — ✅ FIXED (Root Cause: vtable mismatch)
   - **Root cause:** `macos_platform` CMake target was missing `zi_always` dependency, so `RTS_ZEROHOUR` was undefined when compiling `MacOSGameClient.mm`.
   - This caused `notifyTerrainObjectMoved()` and `createSnowManager()` to be excluded from the vtable (`#if RTS_ZEROHOUR` guard), creating a 2-slot vtable offset vs. the engine library.
   - `setFrameRate()` vtable dispatch jumped into `typeinfo` data → `EXC_BAD_ACCESS (code=2)`.
   - **Fix:** Added `zi_always` + `zi_gameengine_include` as PRIVATE deps to `macos_platform`, switched include paths from `Generals/` to `GeneralsMD/`.

4. **ERROR_INVALID_D3D in `DX8Wrapper::Init()`** — ✅ FIXED
   - `LoadLibrary`/`GetProcAddress` stubs in `windows.h` returned nullptr, preventing D3D8 interface creation.
   - **Fix:** `LoadLibrary` now returns a marker, `GetProcAddress("Direct3DCreate8")` routes to `CreateMetalInterface8`.

5. **ERROR_OUT_OF_MEMORY (0xDEAD0002)** — ✅ FIXED
   - Memory pools for `MetalSurface8` and `MetalTexture8` were missing from `GameMemoryInitPools_GeneralsMD.inl`.
   - The game's memory pool system creates pools with `init=0, overflow=0` and expects them to be listed in the pool size table.
   - **Fix:** Added pool entries under `#ifdef __APPLE__` in `GameMemoryInitPools_GeneralsMD.inl`.

6. **SIGSEGV in `GameResultsInterface`** — ✅ FIXED
   - `createNewGameResultsInterface()` returned nullptr → `initSubsystem` called `setName()` on null pointer.
   - **Fix:** Created `StubGameResultsInterface` with no-op methods in `GameSpyStubs.cpp`.

7. **Shader/Asset paths** — ✅ FIXED
   - `MacOSShaders.metal` not found at runtime → added more search paths in `MetalDevice8.mm`.
   - `.big` files not found → created symlinks in build output directory.

8. **SIGSEGV in audio playback (`scheduleFile`)** — ✅ FIXED
   - AVAudioEngine not running or audio format incompatible → crash in `AVAudioPlayerNode::scheduleFile`.
   - **Fix:** Added `@try/@catch` block and engine-running guard in `friend_forcePlayAudioEventRTS`.

9. **SIGABRT — Metal driver vs custom allocator conflict** — ✅ FIXED (CRITICAL)
   - Global `operator new/delete` override routed all allocations through `DynamicMemoryAllocator`, which prepends a `MemoryPoolSingleBlock` header. Metal/AppKit/libdispatch frameworks use system malloc but free through our overridden `delete` → `___BUG_IN_CLIENT_OF_LIBMALLOC_POINTER_BEING_FREED_WAS_NOT_ALLOCATED`.
   - **Fix:** On macOS (`#ifdef __APPLE__`), global `operator new` uses `calloc(1, size)` and `operator delete` uses `free()`. Pool-based allocations (`MEMORY_POOL_GLUE`) still use the game's allocator. `calloc` ensures zero-fill (the game relies on zeroed allocations).

10. **SIGSEGV in `W3DBridgeBuffer` / `Pathfinder` constructors** — ✅ FIXED
    - `m_numBridges` was uninitialized before `clearAllBridges()` was called in the constructor → garbage loop count → crash in `clearBridge()` on uninitialized objects.
    - Exposed by the switch from custom allocator (which zeroed memory) to system malloc.
    - **Fix:** Initialize `m_numBridges = 0` before `clearAllBridges()` call. Also fixed by using `calloc` to zero-fill all allocations.

**Current State:**
- **Stable idle runtime** — 400+ frames rendered, 800+ heartbeats, zero crashes over 35s.
- **Next focus:** Interactive features (mouse/keyboard input), full rendering (textures, 3D scene), audio playback.

**⚠️ IMPORTANT: Kill previous processes before testing!**
```bash
killall generalszh 2>/dev/null; sleep 1
```

**Reference Files:**
- `.agent/PORTING_STATUS.md` — Full status with build flow diagram
- `.agent/NEXT_STEPS_PROMPT.md` — Detailed next steps prompt
- `.agent/RENDERING_FLOW.md` — Metal rendering pipeline architecture
- `Platform/MacOS/Source/Stubs/GameSpyStubs.cpp` — Linker stubs (450+ lines)
- `Platform/MacOS/Source/Main/MacOSMain.mm` — Main entry point
- `Platform/MacOS/CMakeLists.txt` — Platform build config (with vtable fix)

**Instructions for AI:** Continue with **Phase 5: Runtime Debugging**. Game launches and runs stable. Focus on interactive features: input handling, full rendering, audio. **Russian** for communication, **English** for code/tech notes.
