# macOS Stubs Audit — Systematic Tracking Table

**Created:** 2026-02-20
**Last Updated:** 2026-02-20 22:28
**Purpose:** Audit every stub in `Platform/MacOS/` to find the wild branch (`EXC_BAD_INSTRUCTION` at `0x100000000`) culprit.
**Crash context:** PC jumps to `0x100000000` (Mach-O header), likely from nullptr vtable deref. Happens during `GameClient::update()` after `MetalDevice8::Clear` and 2D text drawing.

---

## ✅ RESOLVED: ODR Violations Fixed (2026-02-20 22:28)

### 1. AudioManager ODR — FIXED ✅
**`MacOSMain.mm` had stub implementations for `AudioManager` base class methods that duplicated `Core/GameEngine/Source/Common/Audio/GameAudio.cpp`.**

**Fix applied:** Removed ALL AudioManager stubs from `MacOSMain.mm`. The real implementations in `GameAudio.cpp` are now used exclusively.

### 2. GlobalData ODR — FIXED ✅
**`MacOSMain.mm` had a simplified `GlobalData::GlobalData()` constructor that duplicated the full 450-line constructor in `GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp`.**

**Fix applied:** Removed GlobalData stubs from `MacOSMain.mm`. Added `#ifdef __APPLE__` block in `GlobalData.cpp` for macOS-specific `m_userDataDir` (~/Library/Application Support/Generals Zero Hour).

### 3. Win32GameEngine ODR — FIXED ✅
**`MacOSMain.mm` defines `Win32GameEngine::init/reset/update/serviceWindowsOS` which were also in `GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32GameEngine.cpp` (which was being compiled).**

**Fix applied:** Added `if(APPLE) set_source_files_properties(Win32GameEngine.cpp PROPERTIES HEADER_FILE_ONLY TRUE)` in `GeneralsMD/Code/GameEngineDevice/CMakeLists.txt`.

### Previous crash analysis (for reference):

| Method | Real impl (GameAudio.cpp) | macOS stub (was) |
|:---|:---|:---|
| `allocateAudioRequest()` | Returns `newInstance(AudioRequest)` ✅ | Was **`nullptr`** 🔴 |
| `getListenerPosition()` | Returns `&m_listenerPosition` ✅ | Was **`nullptr`** 🔴 |
| `newAudioEventInfo()` | Creates + returns `AudioEventInfo*` ✅ | Was **`nullptr`** 🔴 |

---

## Legend

| Symbol | Meaning |
|:---|:---|
| ✅ | **Fully implemented** — real functionality, not a stub |
| ⚠️ | **Partial / Safe stub** — returns reasonable default, unlikely to cause crash |
| ❌ | **Dangerous stub** — returns `nullptr` or has empty implementation where the caller may crash |
| 🔴 | **CRITICAL** — most likely crash candidate (factory/create returning nullptr, or empty vtable) |

---

## 1. Graphics / Metal (DX8 Backend)

**File:** `Metal/MetalDevice8.mm` (2408 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MetalDevice8::InitMetal()` | Real Metal device/layer/shaders init |
| ✅ | `MetalDevice8::BeginScene()` / `EndScene()` | Real Metal frame lifecycle |
| ✅ | `MetalDevice8::Clear()` | Real Metal clear |
| ✅ | `MetalDevice8::Present()` | Real Metal drawable present |
| ✅ | `MetalDevice8::DrawIndexedPrimitive()` | Real Metal encoded draw |
| ✅ | `MetalDevice8::DrawPrimitiveUP()` | Real Metal immediate draw |
| ✅ | `MetalDevice8::SetTexture()` | Real Metal texture binding |
| ✅ | `MetalDevice8::SetRenderState()` | State cache, real pipeline state |
| ✅ | `MetalDevice8::SetTransform()` | Matrix cache → uniforms |
| ✅ | `MetalDevice8::CreateTexture()` | Creates `MetalTexture8` |
| ✅ | `MetalDevice8::CreateVertexBuffer()` | Creates `MetalVertexBuffer8` |
| ✅ | `MetalDevice8::CreateIndexBuffer()` | Creates `MetalIndexBuffer8` |
| ⚠️ | `MetalDevice8::CreatePixelShader()` | Returns `S_OK` (no-op) — game uses FFP mostly |
| ⚠️ | `MetalDevice8::CreateVertexShader()` | Returns `S_OK` (no-op) |
| ⚠️ | `MetalDevice8::SetPixelShader()` | No-op |
| ⚠️ | `MetalDevice8::SetVertexShader()` | Stores FVF, no real VS |
| ⚠️ | `MetalDevice8::SetLight()` | Real light data storage |
| ⚠️ | `MetalDevice8::LightEnable()` | Real enable tracking |
| ⚠️ | `MetalDevice8::GetBackBuffer()` | Creates `MetalSurface8` |
| ⚠️ | `MetalDevice8::GetDepthStencilSurface()` | Creates `MetalSurface8` |

**File:** `Metal/MetalInterface8.mm` (184 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MetalInterface8::CreateDevice()` | Creates `MetalDevice8`, calls `InitMetal()` |
| ⚠️ | `MetalInterface8::GetDeviceCaps()` | Returns hardcoded high-end caps |
| ⚠️ | `MetalInterface8::EnumAdapterModes()` | Returns 800×600 only |
| ⚠️ | `MetalInterface8::GetAdapterMonitor()` | Returns `nullptr` — Windows: returns `HMONITOR` |

**File:** `Metal/MetalTexture8.mm` (386 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MetalTexture8` constructor | Creates real MTLTexture |
| ✅ | `MetalTexture8::LockRect()` / `UnlockRect()` | Real staging + upload |
| ✅ | `MetalTexture8::GetLevelDesc()` / `GetSurfaceLevel()` | Returns real data |

**File:** `Metal/MetalVertexBuffer8.mm` (133 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MetalVertexBuffer8::Lock()` / `Unlock()` / `GetMTLBuffer()` | Real sys-mem + lazy MTL buffer |

**File:** `Metal/MetalIndexBuffer8.mm` (125 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MetalIndexBuffer8::Lock()` / `Unlock()` / `GetMTLBuffer()` | Real sys-mem + lazy MTL buffer |

**File:** `Metal/MetalSurface8.mm` (153 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MetalSurface8::LockRect()` / `UnlockRect()` | Staging buffer alloc, but **no upload** to Metal texture on unlock |
| ⚠️ | `MetalSurface8::GetContainer()` | Returns `nullptr`, `E_NOTIMPL` |

---

## 2. W3D Shader Manager

**File:** `Stubs/MacOSW3DShaderManager.mm` (190 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ⚠️ | `W3DShaderManager::init()` | Printf only |
| ⚠️ | `W3DShaderManager::shutdown()` | Printf only |
| ⚠️ | `W3DShaderManager::getChipset()` | Returns `DC_GEFORCE4` (high-end) |
| ⚠️ | `W3DShaderManager::getShaderPasses()` | Returns `1` |
| ⚠️ | `W3DShaderManager::setShader()` | Stores shader type, returns `TRUE` |
| ⚠️ | `W3DShaderManager::setShroudTex()` | Returns `TRUE` |
| ⚠️ | `W3DShaderManager::LoadAndCreateD3DShader()` | Returns `S_OK` (no real shader) |
| ⚠️ | `W3DShaderManager::testMinimumRequirements()` | Reports high-end hardware |
| ⚠️ | `W3DShaderManager::getGPUPerformanceIndex()` | Returns `STATIC_GAME_LOD_VERY_HIGH` |
| ⚠️ | `W3DShaderManager::endRenderToTexture()` | Returns **`nullptr`** — **SAFE**: all callers check `if (!tex) return false;` |
| ⚠️ | `W3DShaderManager::getRenderTexture()` | Returns **`nullptr`** — **SAFE**: callers check nullptr |
| ⚠️ | `W3DShaderManager::startRenderToTexture()` | No-op |
| ⚠️ | `W3DShaderManager::drawViewport()` | No-op |
| ⚠️ | `W3DShaderManager::filterPreRender()` / `filterPostRender()` / `filterSetup()` | Returns `false` (no filtering) |
| ⚠️ | `ScreenBWFilter::*` | All no-ops |
| ⚠️ | `ScreenBWFilterDOT3::*` | All no-ops |
| ⚠️ | `ScreenMotionBlurFilter::*` | All no-ops |
| ⚠️ | `ScreenCrossFadeFilter::*` | All no-ops |

---

## 3. D3DX Helper Functions

**File:** `Main/D3DXStubs.mm` (538 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `D3DXCreateTextureFromFileExA()` | Real — loads TGA/DDS from .big archives |
| ✅ | `D3DXCreateTexture()` | Delegates to `MetalDevice8::CreateTexture()` |
| ✅ | `DecompressDXT1()` / `DecompressDXT5()` | Real CPU decompression |
| ✅ | `LoadFileData()` | Real — reads from filesystem + .big archives |
| ⚠️ | Texture cache (`s_TextureCache`) | HashMap-based, functional |

---

## 4. Display / Rendering

**File:** `Client/MacOSDisplay.mm` (109 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MacOSDisplay::init()` | Calls `W3DDisplay::init()` |
| ✅ | `MacOSDisplay::draw()` | Delegates directly to `W3DDisplay::draw()` — null-safety guards added to parent for TheGameLogic, TheScriptEngine, TheFramePacer, TheTacticalView, TheParticleSystemManager, TheWaterTransparency |
| ✅ | `MacOSDisplay::update()` | Delegates to `W3DDisplay::update()` → `Display::update()` (video playback) |
| ⚠️ | `MacOSDisplay::takeScreenShot()` | Empty |
| ⚠️ | `MacOSDisplay::toggleMovieCapture()` | Empty |

**File:** `Client/MacOSDisplayString.mm` (329 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MacOSDisplayString::draw()` | Real — CoreText render → texture → DX8 quad |
| ✅ | `MacOSDisplayString::updateTexture()` | Real — rasterizes text via NSBitmapImageRep |
| ✅ | `MacOSDisplayString::getSize()` | Real — returns text dimensions |
| ✅ | `MacOSDisplayStringManager::newDisplayString()` | Returns real `MacOSDisplayString` |
| ⚠️ | `MacOSDisplayString::appendChar()` / `clipToWidth()` | Returns `nullptr` (line range clamping) — safe as callers check |

---

## 5. Game Client (Factory Methods)

**File:** `Main/MacOSGameClient.mm` (238 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MacOSGameClient::createGameDisplay()` | Returns `MacOSDisplay` (W3DDisplay subclass) |
| ✅ | `MacOSGameClient::createDisplayStringManager()` | Returns `MacOSDisplayStringManager` |
| ✅ | `MacOSGameClient::createFontLibrary()` | Returns `MacOSFontLibrary` (CoreText) |
| ✅ | `MacOSGameClient::createInGameUI()` | Returns `W3DInGameUI` |
| ✅ | `MacOSGameClient::createTerrainVisual()` | Returns `W3DTerrainVisual` ← **changed from MacOSTerrainVisual** |
| ✅ | `MacOSGameClient::createWindowManager()` | Returns `MacOSGameWindowManager` |
| ✅ | `MacOSGameClient::createKeyboard()` | Returns `StdKeyboard` |
| ✅ | `MacOSGameClient::createMouse()` | Returns `StdMouse` |
| ✅ | `MacOSGameClient::createVideoPlayer()` | Returns `MacOSVideoPlayer` |
| ⚠️ | `MacOSGameClient::setFrameRate()` | Empty |
| ⚠️ | `MacOSGameClient::addScorch()` | No-op — needs `TheTerrainRenderObject` |
| ⚠️ | `MacOSGameClient::createRayEffectByTemplate()` | No-op — needs W3D scene |
| ⚠️ | `MacOSGameClient::setTeamColor()` / `setTextureLOD()` | No-op — needs terrain render object |
| ✅ | `MacOSGameClient::releaseShadows()` / `allocateShadows()` | **FIXED** — now delegates to `GameClient::` base (iterates drawables) |

**File:** `Main/MacOSGameClient.mm` — Helper Classes

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ⚠️ | `MacOSFontLibrary::loadFontData()` | Real — maps fonts via CoreText, sets `fontData=nullptr` |
| ⚠️ | `MacOSSnowManager`  | All no-ops (init/reset/update) |
| ⚠️ | `MacOSVideoPlayer` | Delegates to `VideoPlayer` base class |
| ⚠️ | `MacOSTerrainVisual` (UNUSED) | **Not used anymore** — `W3DTerrainVisual` is used instead |

---

## 6. Win32 Game Engine (Factory Methods)

**File:** `Main/MacOSMain.mm` (916 lines) — Factory Methods

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `Win32GameEngine::createGameClient()` | Returns `MacOSGameClient` |
| ✅ | `Win32GameEngine::createLocalFileSystem()` | Returns `StdLocalFileSystem` |
| ✅ | `Win32GameEngine::createArchiveFileSystem()` | Returns `StdBIGFileSystem` |
| ✅ | `Win32GameEngine::createModuleFactory()` | Returns `W3DModuleFactory` |
| ✅ | `Win32GameEngine::createThingFactory()` | Returns `ThingFactory` |
| ✅ | `Win32GameEngine::createFunctionLexicon()` | Returns `W3DFunctionLexicon` |
| ✅ | `Win32GameEngine::createAudioManager()` | Returns `MacOSAudioManager` |
| ✅ | `Win32GameEngine::createRadar()` | Returns `RadarDummy` |
| ✅ | `Win32GameEngine::createWebBrowser()` | Returns `StubWebBrowser` |
| ✅ | `Win32GameEngine::createParticleSystemManager()` | **FIXED** — now returns `W3DParticleSystemManager` (was `StubParticleSystemManager`) |
| ⚠️ | `Win32GameEngine::createNetwork()` | Returns `StubNetwork` |

---

## 7. Win32 Game Engine (Stub Subsystems)

**File:** `Main/MacOSMain.mm` — Stub Classes

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ⚠️ | `StubNetwork` | Full `NetworkInterface` no-op impl (45+ methods) |
| ⚠️ | ~~`StubParticleSystemManager`~~ | **REMOVED** — replaced by `W3DParticleSystemManager` |
| ⚠️ | `StubWebBrowser` | No-op `createBrowserWindow()` returns `false` |
| ⚠️ | `CDManagerStub` | Returns `nullptr` from `getDrive()`, `newDrive()`, `createDrive()` |

---

## 8. AudioManager — ✅ RESOLVED

**File:** `Main/MacOSMain.mm` — Base class stubs **REMOVED** (was lines 213-268)

All AudioManager base class stubs have been **removed**. The real implementations from `Core/GameEngine/Source/Common/Audio/GameAudio.cpp` are now used.

| Status | Note |
|:---|:---|
| ✅ | All 40+ AudioManager base methods now use real implementations from GameAudio.cpp |

**File:** `Audio/MacOSAudioManager.mm` (379 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MacOSAudioManager::friend_forcePlayAudioEventRTS()` | Real — extracts from .big, plays via AVAudioPlayer |
| ✅ | `MacOSAudioManager::update()` | Real — cleans up finished audio |
| ✅ | `MacOSAudioManager::processRequestList()` | Real — dispatches play/stop/pause |
| ⚠️ | `MacOSAudioManager::getDevice()` | Returns **`nullptr`** — Miles `HDIGDRIVER` equivalent |
| ⚠️ | `MacOSAudioManager::getHandleForBink()` | Returns **`nullptr`** — Bink audio handle |
| ⚠️ | `MacOSAudioManager::getFileLengthMS()` | Returns `0.0f` |

---

## 9. GameSpy / Network / WOL

**File:** `Stubs/GameSpyStubs.cpp` (449 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ⚠️ | 14 null singletons (`TheGameSpyConfig`, `TheLAN`, `TheNAT`, etc.) | All `nullptr` — safe if not dereffed during offline play |
| ⚠️ | 10 overlay functions (`GameSpyOpenOverlay`, etc.) | All no-ops |
| ⚠️ | 8 lobby/game list functions | Return `nullptr` / `NAMEKEY_INVALID` |
| ⚠️ | 8 network/patch functions | All no-ops |
| ⚠️ | 18 Transport/UDP methods | Return `FALSE` / `-1` / `0` |
| ⚠️ | 47 LANAPI methods | All no-ops, lookups return `nullptr` |
| ⚠️ | 12 GameSpyStagingRoom methods | All no-ops |
| ⚠️ | 13 NAT/User/Download methods | All no-ops |
| ⚠️ | RegistryClass (4 methods) | Returns default values |
| ⚠️ | DX8WebBrowser (4 methods) | All no-ops |
| ⚠️ | WorkerProcess (6 methods) | All no-ops, `isDone()` returns `true` |
| ✅ | `GameResultsInterface::createNewGameResultsInterface()` | Returns `StubGameResultsInterface` (**was** `nullptr`, fixed) |
| ⚠️ | `CreateIMEManagerInterface()` | Returns **`nullptr`** — **SAFE**: all callers check `if (TheIMEManager)` before use (verified in GameClient.cpp:352, Shell.cpp) |

---

## 10. Compression (LZHL) — ✅ RESOLVED

**File:** `Stubs/LZHLStubs.cpp` — **REMOVED**

LZHL stubs were an ODR violation — the real `liblzhl` library is fetched via FetchContent and linked through `core_compression`. The stubs returned `0` from `LZHLDecompress`/`LZHLCompress`, which would break all save/replay/network compression.

| Status | Note |
|:---|:---|
| ✅ | Real liblzhl now used exclusively — stubs removed from macOS build |

---

## 11. WWDownload / FTP

**File:** `Stubs/WWDownloadStubs.cpp` (65 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ⚠️ | `CDownload::PumpMessages()` / `Abort()` | Returns `S_OK` |
| ⚠️ | `CDownload::DownloadFile()` | Returns `E_FAIL` |
| ⚠️ | `Cftp::*` (15 methods) | All return `E_FAIL` / `-1` |

---

## 12. File System

**File:** `Common/StdLocalFile.cpp`, `Common/StdLocalFileSystem.cpp`, `Common/StdBIGFile.cpp`, `Common/StdBIGFileSystem.cpp`

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `StdLocalFile` | Full implementation using POSIX `fopen`/`fread`/`fwrite` |
| ✅ | `StdLocalFileSystem` | Full implementation using `opendir`/`readdir` |
| ✅ | `StdBIGFile` | Full implementation reading from .big archives |
| ✅ | `StdBIGFileSystem` | Full implementation mounting .big archives |

---

## 13. Input (Keyboard / Mouse)

**File:** `Main/StdKeyboard.mm` (252 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `StdKeyboard::update()` | Calls `Keyboard::update()` — ring buffer → `m_keys` |
| ✅ | `StdKeyboard::getKey()` | Real — reads from ring buffer |
| ✅ | `StdKeyboard::addEvent()` | Real — macOS keyCode → DIK mapping |
| ✅ | Full key mapping | A-Z, 0-9, F1-F12, arrows, modifiers, etc. |

**File:** `Main/StdMouse.mm` (229 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `StdMouse::update()` | Calls `Mouse::update()` |
| ✅ | `StdMouse::getMouseEvent()` | Real — reads from ring buffer |
| ✅ | `StdMouse::draw()` | Real — draws cursor image or green square fallback |
| ⚠️ | `StdMouse::setCursor()` | Maps to NSCursor (limited: arrow, crosshair, hand only) |
| ⚠️ | `StdMouse::capture()` / `releaseCapture()` | Empty — no SetCapture equivalent |
| ⚠️ | `StdMouse::regainFocus()` / `loseFocus()` | Empty |

---

## 14. Window Manager

**File:** `Main/MacOSWindowManager.mm` (355 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MacOS_Main()` | Real — creates NSWindow, inits renderer, calls GameMain |
| ✅ | `MacOS_CreateWindow()` | Real — creates NSWindow with GameContentView |
| ✅ | `MacOS_PumpEvents()` | Real — full NSEvent loop (keys, mouse, scroll) |
| ✅ | `MacOS_GetScreenSize()` | Real — reads NSScreen |

**File:** `Main/MacOSGameWindowManager.mm` (93 lines)

| Status | Stub / Class / Function | Notes |
|:---|:---|:---|
| ✅ | `MacOSGameWindowManager` | Inherits `W3DGameWindowManager` — all gadget draw funcs from W3D |
| ✅ | `allocateNewWindow()` | Returns `MacOSGameWindow` |
| ✅ | `winFormatText()` / `winGetTextSize()` | Uses `MacOSDisplayString` |

---

## 15. windows.h Shim (Key Returns)

**File:** `Include/windows.h` (~1590 lines)

| Status | Stub / Function | Notes |
|:---|:---|:---|
| ⚠️ | `LoadLibrary()` | Returns `(HMODULE)1` marker — **safe** |
| ✅ | `GetProcAddress("Direct3DCreate8")` | Returns `_CreateMetalInterface8_Wrapper` — **real** |
| ⚠️ | `CreateEvent()` / `CreateEventA()` | Returns **`nullptr`** — callers may check |
| ⚠️ | `SetCursor()` | Returns **`nullptr`** |
| ⚠️ | `LoadCursorFromFile()` | Returns **`nullptr`** |
| ⚠️ | `MonitorFromWindow()` | Returns **`nullptr`** |
| ⚠️ | `GetDesktopWindow()` | Returns **`nullptr`** |
| ⚠️ | `GetDC()` | Returns **`nullptr`** |
| ⚠️ | `GetProcessHeap()` | Returns **`nullptr`** — but `HeapAlloc` uses `calloc` directly |

---

## 16. Git / Build Info

**File:** `Stubs/GitInfoStubs.cpp` (12 lines)

| Status | Stub / Function | Notes |
|:---|:---|:---|
| ⚠️ | `GitSHA1`, `GitShortSHA1`, etc. | Hardcoded "MACOS_BUILD_STUB" |
| ⚠️ | `GitHaveInfo = true` | Prevents "no git info" errors |

---

## 17. Debug / Screenshot

**File:** `Debug/MacOSScreenshot.mm` (114 lines)

| Status | Stub / Function | Notes |
|:---|:---|:---|
| ✅ | `MacOS_SaveScreenshot()` | Real when `ENABLE_SCREENSHOTS` defined |
| ⚠️ | When `!ENABLE_SCREENSHOTS` | All no-ops |

---

## 18. Gadget Draw (Fallback)

**File:** `Main/MacOSGadgetDraw.mm` (188 lines)

| Status | Stub / Function | Notes |
|:---|:---|:---|
| ⚠️ | `MacOSGadget*Draw` (10 functions) | **NOT USED** — `MacOSGameWindowManager` inherits `W3DGameWindowManager` which provides real W3D draw functions. These are legacy fallbacks. |

---

# ✅ CRITICAL STUBS — All Resolved (2026-02-20)

All previously-critical stubs have been resolved:

| # | Issue | Resolution |
|:--|:--|:--|
| 1 | `CreateIMEManagerInterface() → nullptr` | ✅ **SAFE** — all callers check `if (TheIMEManager)` |
| 2 | `AudioManager::allocateAudioRequest() → nullptr` | ✅ **REMOVED** — real impl from GameAudio.cpp |
| 3 | `AudioManager::newAudioEventInfo() → nullptr` | ✅ **REMOVED** — real impl from GameAudio.cpp |
| 4 | `AudioManager::getListenerPosition() → nullptr` | ✅ **REMOVED** — real impl from GameAudio.cpp |
| 5 | `W3DShaderManager::endRenderToTexture() → nullptr` | ✅ **SAFE** — callers check `if (!tex) return false;` |
| 6 | `CDManagerStub::getDrive() → nullptr` | ✅ **SAFE** — `driveCount()` returns 0, never called |
| 7 | `StubParticleSystemManager::doParticles()` — empty | ✅ **SAFE** — no side effects expected |
| 8 | `MacOSDisplay::update()` — was empty | ✅ **FIXED** — now delegates to `W3DDisplay::update()` |

---

# Summary Statistics

| Category | Total Stubs | ✅ Implemented | ⚠️ Safe Stub | ❌ Dangerous | 🔴 Critical |
|:---|:---|:---|:---|:---|:---|
| Metal / DX8 | 40 | 26 | 14 | 0 | 0 |
| W3D Shader Manager | 18 | 0 | 18 | 0 | 0 |
| D3DX Helpers | 5 | 5 | 0 | 0 | 0 |
| Display | 5 | 4 | 1 | 0 | 0 |
| DisplayString | 5 | 4 | 1 | 0 | 0 |
| GameClient Factory | 14 | 9 | 5 | 0 | 0 |
| GameEngine Factory | 11 | 9 | 2 | 0 | 0 |
| AudioManager | 25 | 25 | 0 | 0 | 0 |
| GameSpy/Network | 170+ | 1 | 169 | 0 | 0 |
| FileSystem | 4 | 4 | 0 | 0 | 0 |
| Input | 12 | 10 | 2 | 0 | 0 |
| Window Manager | 6 | 6 | 0 | 0 | 0 |
| Compression | 5 | 5 | 0 | 0 | 0 |
| WWDownload | 17 | 0 | 17 | 0 | 0 |
| windows.h | 9 | 1 | 8 | 0 | 0 |
| Debug/Screenshot | 3 | 1 | 2 | 0 | 0 |
| Git Info | 2 | 0 | 2 | 0 | 0 |
| **TOTAL** | **~350** | **~110** | **~240** | **0** | **0** |
