# macOS Port — Build System & Status

**Goal:** Clean macOS build via CMake (`--preset macos`).
**Branch:** `feature/macos-c_make`
**Last Updated:** 2026-02-19 14:57
**Build Progress:** ✅ BUILD SUCCESSFUL — both `generalsv` (21MB) and `generalszh` (22MB) linked and built
**Runtime Progress:** 🟢 Phase 5 — STABLE RUNTIME! 10 crashes resolved. Game runs 35+ seconds, 400+ frames via Metal, zero crashes.

---

## 🏗 macOS Build Flow

```
cmake --preset macos          → configure (Ninja, Debug, ARM64)
cmake --build build/macos     → build
killall generalszh 2>/dev/null; sleep 1
build/macos/GeneralsMD/generalszh  → run

                        CMakeLists.txt (root)
                              │
  ┌───────────────────────────┼───────────────────────────────────┐
  │         CONFIGURE         │                                   │
  │                           │                                   │
  │  cmake/compilers.cmake    │  cmake/config.cmake               │
  │   └─ C++20, -g for Rel    │   ├─ config-build.cmake           │
  │                           │   │   └─ RTS_BUILD_ZEROHOUR=ON    │
  │  cmake/debug_strip.cmake  │   ├─ config-debug.cmake           │
  │   └─ MinGW only, skip     │   └─ config-memory.cmake          │
  │                           │                                   │
  ├───────────────────────────┼───────────────────────────────────┤
  │     DEPENDENCIES          │                                   │
  │                           │                                   │
  │  ┌─ DX8 (APPLE) ──────────┼──► d3d8_stub.h (pure C++ ifaces)  │
  │  │  NO FetchContent!      │    Platform/MacOS/Include/ only   │
  │  │  d3d8lib INTERFACE:    │    MetalDevice8 implements these  │
  │  │    include → MacOS/Inc │    BUILD_WITH_D3D8 define         │
  │  │  d3d8,d3dx8,dinput8,   │                                   │
  │  │  dxguid: empty targets │                                   │
  │  │                        │                                   │
  │  ├─ GameSpy (APPLE) ──────┼──► FetchContent → INTERFACE only  │
  │  │  include paths only    │    (real code in Platform stubs)  │
  │  │                        │                                   │
  │  ├─ Miles (APPLE) ────────┼──► milesstub INTERFACE            │
  │  │  Dependencies/miles/*  │    include path only              │
  │  │                        │                                   │
  │  ├─ Bink (APPLE) ─────────┼──► binkstub INTERFACE             │
  │  │  Dependencies/bink/*   │    include path only              │
  │  │                        │                                   │
  │  ├─ Win32 libs (APPLE) ───┼──► INTERFACE dummies              │
  │  │  comctl32,vfw32,       │    (no-op link targets)           │
  │  │  winmm,imm32           │                                   │
  │  │                        │                                   │
  │  ├─ lzhl ─────────────────┼──► STATIC lib (real, cross-plat)  │
  │  │                        │                                   │
  │  └─ stlport ──────────────┼──► INTERFACE (non-VC6)            │
  │                           │                                   │
  ├───────────────────────────┼───────────────────────────────────┤
  │     BUILD TARGETS         │                                   │
  │                           │                                   │
  │  Dependencies/Utility     → core_utility                      │
  │  resources/               → resources                         │
  │                           │                                   │
  │  Platform/MacOS/ ◄────── ✅ CMakeLists.txt DONE               │
  │   ├─ Include/             │  (d3d8_stub.h, shims, platform)   │
  │   ├─ Source/Metal/        │  (MetalDevice8, buffers, tex)     │
  │   ├─ Source/Main/         │  (MacOSMain, client, input)       │
  │   ├─ Source/Client/       │  (Display, DisplayString)         │
  │   ├─ Source/Audio/        │  (MacOSAudioManager)              │
  │   ├─ Source/Common/       │  (Std* file system)               │
  │   ├─ Source/Stubs/        │  (GameSpy, LZHL, WWDownload)      │
  │   └─ Source/Debug/        │  (Screenshot)                     │
  │                           │                                   │
  │  Core/                    │                                   │
  │   ├─ Libraries/           │                                   │
  │   │  ├─ Source/debug/     → core_debug ◄── ✅ debug_stack excl│
  │   │  ├─ Source/profile/   → core_profile ◄── ✅ win files excl│
  │   │  ├─ Source/WWVegas/   → core_ww* (d3d8lib, milesstub)     │
  │   │  ├─ Source/Compression→ compression lib                   │
  │   │  └─ Source/EABrowser* → EA browser libs                   │
  │   ├─ GameEngine/          → corei_gameengine_*                │
  │   └─ GameEngineDevice/    → corei_gameenginedevice_*          │
  │       └─ links: d3d8lib, binkstub, milesstub                  │
  │                           │                                   │
  │  Generals/Code/           │                                   │
  │   └─ GameEngine/          → g_gameengine  ◄── ✅ CLEAN        │
  │                           │                                   │
  │  GeneralsMD/Code/         │                                   │
  │   ├─ GameEngine/          → z_gameengine  ◄── ✅ CLEAN        │
  │   ├─ GameEngineDevice/    → z_gameenginedevice ◄── ✅ CLEAN   │
  │   │   └─ PCH: <windows.h> (uses shim)                         │
  │   │   └─ links: corei_gameenginedevice_*, z_gameengine        │
  │   └─ Main/                → z_generals (executable)           │
  │       ├─ WinMain.cpp      │  (WIN32 entry — skipped on mac)   │
  │       └─ links: d3d8,d3dx8,dinput8,dxguid,binkstub,           │
  │          milesstub, comctl32,vfw32,winmm,imm32,               │
  │          core_debug, core_profile, z_gameengine,              │
  │          z_gameenginedevice, zi_always                        │
  └───────────────────────────┴───────────────────────────────────┘
```

---

## 📦 Platform/MacOS/Include/ — Shim Inventory

### DX8 Core (d3d8_stub.h architecture)
| File | Lines | Purpose |
|:---|:---|:---|
| `d3d8_stub.h` | 1114 | **Source of Truth** — pure C++ DX8 interfaces (no COM vtable). MetalDevice8 implements these. |
| `d3d8.h` | 3 | Thin redirect → `d3d8_stub.h` |
| `d3d8types.h` | 3 | Thin redirect → `d3d8_stub.h` |
| `d3d8caps.h` | 3 | Thin redirect → `d3d8_stub.h` |

### D3DX Extensions
| File | Lines | Purpose |
|:---|:---|:---|
| `d3dx8.h` | 5 | Umbrella → `d3dx8core.h` + `d3dx8math.h` + `d3dx8tex.h` |
| `d3dx8math.h` | ~270 | `D3DXVECTOR2/3/4`, `D3DXMATRIX`, `D3DXQUATERNION`, inline math, `D3DXMatrixInverse`, `operator*=`, transitive `d3dx8core.h` include |
| `d3dx8core.h` | ~110 | `ID3DXFont` stub, D3DX function declarations, `D3DXIMAGE_INFO`, filter flags |
| `d3dx8tex.h` | ~25 | Forwarding + `D3DXFilterTexture` inline stub |
| `d3dx8mesh.h` | 2 | Empty (mesh utils not used on macOS) |
| `d3dx8shape.h` | 2 | Empty (shape utils not used) |
| `d3dx8effect.h` | 2 | Empty (effect framework not used) |

### Win32 Compatibility
| File | Lines | Purpose |
|:---|:---|:---|
| `windows.h` | ~1590 | Base types, string shims, memory macros, struct defs, CriticalSection, exception/debug types, file ops, locale/date, MessageBox, process stubs, cursor/window stubs, message loop, execution state, FPU control, GetAsyncKeyState, Heap API (GetProcessHeap/HeapAlloc/HeapFree) |
| `objbase.h` | ~80 | COM shim: `IUnknown`, `GUID`, `STDMETHOD`, `DEFINE_GUID` |
| `ddraw.h` | 7 | `DDSCAPS2_CUBEMAP`, `DDSCAPS2_VOLUME` constants |
| `mmsystem.h` | 2 | Empty (dx8caps.cpp includes but doesn't use) |
| `windowsx.h` | 2 | Empty (UI helpers not needed) |
| `malloc.h` | 2 | Redirect → `<stdlib.h>` |
| `tchar.h` | ~48 | TCHAR → char mappings |
| `winerror.h` | ~25 | Windows error code constants |
| `winreg.h` | ~47 | Registry API stubs (always fail) |
| `wininet.h` | ~10 | WinInet stubs |
| `shlobj.h` | ~20 | `SHGetSpecialFolderPath` + CSIDL defines |
| `shlguid.h` | 1 | Empty |
| `snmp.h` | 1 | Empty |
| `mapicode.h` | 4 | Empty |
| `dinput.h` | ~120 | `DIK_` key codes + `IDirectInput` stubs |
| `oleauto.h` | ~10 | `BSTR`/`OLE` stubs |
| `atlbase.h` | ~10 | `CComModule` stub |
| `atlcom.h` | 4 | Includes `atlbase.h` |
| `EABrowserDispatch/BrowserDispatch.h` | 1 | Empty |

---

## ✅ z_gameengine — CLEAN BUILD (0 errors)

All 10 categories of issues resolved:
- `StackDump.cpp`, `MiniDumper.cpp`, `WorkerProcess.cpp` — excluded from CMake.
- All GameSpy/WinSock-dependent files excluded (`LANAPI*`, `Transport`, `udp`, `IPEnumeration`, `DownloadManager`, `GameSpy/*`).
- Pointer-to-int casts fixed (`INI.cpp`, `LocalFile.cpp`, `GUIUtil.cpp`, `FirewallHelper.cpp`).
- `ShowWindow`, `MB_*` constants added to `windows.h`.
- `endian_compat.h` — standard `uint*_t` types.
- `GetModuleFileNameW` stub added.
- `FARPROC` → function pointer type + explicit casts in `ScriptEngine.cpp`.
- `QueryPerformanceFrequency`/`Counter` + FPU control stubs added.

## ✅ g_gameengine — CLEAN BUILD (0 errors)

- WOL Browser code guarded with `#ifndef __APPLE__`.
- 40+ pointer-to-int casts fixed across GUI callbacks, GameLogic, GameNetwork.
- GameSpy/Network files excluded from CMake.
- `GadgetTextEntry` `WindowMsgData` cast via `uintptr_t`.

---

## ✅ z_gameenginedevice — CLEAN BUILD (0 errors)

All compilation errors resolved:
- `D3DXVECTOR4` duplicate typedef removed (`d3d8_stub.h`).
- `D3DXVECTOR4::operator const void*()` added for `SetPixelShaderConstant`.
- `D3DXMATRIX::operator*=` added for compound matrix ops.
- `D3DXMatrixInverse` implemented (Gauss-Jordan).
- `d3dx8math.h` → `d3dx8core.h` transitive include (fixes `D3DXAssembleShader` undeclared in `W3DWater.cpp`).
- `GetAsyncKeyState` stub added to `windows.h`.
- `HeapAlloc`/`HeapFree`/`GetProcessHeap`/`HEAP_ZERO_MEMORY` stubs added (`malloc`/`free` backend).
- Missing `#endif` for `_FPCONTROL_DEFINED` fixed.

---

## ✅ What's Already Working

| Component | Status | Details |
|:---|:---|:---|
| **CMakePresets.json** | ✅ | `macos` and `macos-release` presets |
| **Root CMakeLists.txt** | ✅ | Apple branching, DX8 via `d3d8_stub.h` |
| **DX8 → d3d8_stub.h** | ✅ | **No FetchContent!** Pure C++ interfaces |
| **d3dx8 shims** | ✅ | math/core/tex — types + functions declared |
| **GameSpy** | ✅ | INTERFACE-only + stubs |
| **Miles/Bink** | ✅ | INTERFACE stubs |
| **Win32 dummy libs** | ✅ | comctl32, vfw32, winmm, imm32 |
| **LZHL** | ✅ | Real STATIC lib |
| **stlport** | ✅ | INTERFACE (non-VC6) |
| **config.cmake** | ✅ | build/debug/memory config |
| **Platform/MacOS/CMakeLists** | ✅ | Metal, Cocoa, stubs, shaders |
| **Win32 shims** | ✅ | `windows.h` (~1470 lines), `objbase.h`, `tchar.h`, `winreg.h` etc. |
| **PCH ordering** | ✅ | `windows.h` first in all PCH lists |
| **debug exclusions** | ✅ | `debug_stack.cpp`, `DbgHelp*`, `debug_io_*` excluded |
| **profile exclusions** | ✅ | Windows-specific profile .cpp excluded |
| **osdep.h** | ✅ | `_UNIX` mechanism, Utility include path |
| **core_debug** | ✅ | Compiles |
| **core_profile** | ✅ | Compiles |
| **core_wwlib** | ✅ | Compiles |
| **core_wwmath** | ✅ | Compiles |
| **core_compression** | ✅ | Compiles |
| **macos_platform** | ✅ | **CLEAN BUILD** — all Carbon conflicts resolved, D3DX stubs fixed |
| **z_gameengine** | ✅ | **CLEAN BUILD — 0 errors** |
| **g_gameengine** | ✅ | **CLEAN BUILD — 0 errors** |
| **Carbon compat layer** | ✅ | `macos_carbon_compat.h` force-include blocks conflicting Carbon headers |
| **RGBColor conflict** | ✅ | Guarded with `#if !defined(__QUICKDRAW__)` in BaseType.h |
| **D3DX stubs** | ✅ | Fixed signature mismatch, removed duplicate `extern C` defs |
| **Override mismatches** | ✅ | Removed non-virtual methods from derived classes |
| **Memory pool macros** | ✅ | `allocateBlockImplementation(msg)` → `allocateBlock(msg)` for release builds |
| **WOL Browser code** | ✅ | Guarded with `#ifndef __APPLE__` |
| **LANMessage size** | ✅ | `MAX_LANAPI_PACKET_SIZE *= 2` on macOS |
| **_strlwr/_strupr linkage** | ✅ | `extern "C"` wrapper |
| **GadgetTextEntry pointer cast** | ✅ | `WindowMsgData` via `uintptr_t` |
| **GameSpy/Network exclusions** | ✅ | 20+ WinSock-dependent files excluded |
| **Pointer-to-int 64-bit fixes** | ✅ | Multiple files fixed |
| **windows.h stubs** | ✅ | `IsIconic`, `SetCursor`, `GetCursorPos`, `ScreenToClient`, `MSG`, `PeekMessage`, `SetErrorMode`, `SetThreadExecutionState`, `MessageBox` constants, threading (`CreateEvent`, `SetEvent`, `WaitForSingleObject`, `_beginthread`), `BITMAPINFO` pointer types, `GetAsyncKeyState`, `HeapAlloc`/`HeapFree`/`GetProcessHeap`, FPU control |
| **Win32GameEngine.h** | ✅ | WebBrowser include guarded, `CComObject` guarded |
| **W3DDisplay.cpp** | ✅ | `IsIconic` guarded, `CreateBMPFile` guarded |
| **WWAudio PCH** | ✅ | `windows.h` added via target_precompile_headers |
| **BINKEXPORT macro** | ✅ | Uses `__attribute__((visibility("default")))` on macOS |
| **D3DX math extensions** | ✅ | `D3DXMatrixInverse`, `operator*=`, `D3DXVECTOR4` void* cast, transitive `d3dx8core.h` include |
| **z_gameenginedevice** | ✅ | **CLEAN BUILD — 0 errors** |
| **g_generals (compile)** | ✅ | **ALL COMPILATION PASSED** — factory methods guarded with `#ifndef __APPLE__` |
| **z_generals (compile)** | ✅ | **ALL COMPILATION PASSED** — ZH-only virtuals (`notifyTerrainObjectMoved`, `createSnowManager`) guarded with `#if RTS_ZEROHOUR` |
| **MacOSMain.mm** | ✅ | SDK conflicts resolved (`__INTLRESOURCES__`, `__FINDER__`, `__AIFF__`), `toggleNetworkOn` guarded with `RTS_DEBUG` |
| **Win32GameEngine.h** | ✅ | Both Generals and GeneralsMD factory methods guarded with `#ifndef __APPLE__` |

---

## ✅ LINKER STAGE — RESOLVED (0 undefined symbols)

All linker errors resolved via `GameSpyStubs.cpp` (430+ lines).
Stub categories:

| Category | Count | Examples |
|:---|:---|:---|
| Null singletons | 14 | `TheGameSpyConfig`, `TheLAN`, `TheNAT`, `TheGameSpyGame` |
| GameSpy overlay | 10 | `GameSpyOpenOverlay`, `GameSpyCloseAllOverlays` |
| Lobby / Game list | 10 | `GetGameInfoListBox`, `RefreshGameListBoxes` |
| Network / Patch | 8 | `HTTPThinkWrapper`, `SetUpGameSpy`, `StartPatchCheck` |
| Transport / UDP | 18 | `Transport::init`, `UDP::Bind`, `UDP::Read` |
| LANAPI (vtable) | 47 | All virtual methods — init, reset, update, Request*, On* |
| GameSpyStagingRoom | 12 | Constructor, vtable, `reset`, `init`, `amIHost` |
| NAT / User / Download | 13 | `NAT::update`, `User::setName`, `DownloadManager::update` |
| Win32 compat | 25 | `RegistryClass`, `WorkerProcess`, `DX8WebBrowser`, `DbgHelpGuard` |
| Misc | 15 | `getQR2HostingStatus`, `ghttp*`, `LadderList`, IME, StackDump |

---

## 🔧 Phase 5: Runtime Debugging — Progress

| Problem | Status | Details |
|:---|:---|:---|
| **SIGBUS in MacOSAudioManager::init()** | ✅ FIXED | `AVAudioEngine` exception → wrapped in `@try`/`@catch` |
| **SIGSEGV in ThingTemplate::parseModuleName** | ✅ FIXED | `createModuleFactory()` returned base `ModuleFactory` → fixed to `W3DModuleFactory` |
| **SIGBUS in GameClient::init() / setFrameRate()** | ✅ FIXED | **Vtable mismatch** — `macos_platform` lacked `zi_always` (PRIVATE), `RTS_ZEROHOUR` not defined during compilation. 2 vtable slot shift → typeinfo as code → `EXC_BAD_ACCESS` |
| **ERROR_INVALID_D3D in DX8Wrapper::Init()** | ✅ FIXED | `LoadLibrary`/`GetProcAddress` stubs returned `nullptr` → fixed: `LoadLibrary` returns marker, `GetProcAddress("Direct3DCreate8")` → `CreateMetalInterface8` |
| **ERROR_OUT_OF_MEMORY (0xDEAD0002)** | ✅ FIXED | Memory pools for `MetalSurface8`/`MetalTexture8` missing in `GameMemoryInitPools_GeneralsMD.inl` → added under `#ifdef __APPLE__` |
| **SIGSEGV in GameResultsInterface** | ✅ FIXED | `createNewGameResultsInterface()` returned `nullptr` → created `StubGameResultsInterface` with no-op methods |
| **SIGSEGV in audio playback (scheduleFile)** | ✅ FIXED | `AVAudioEngine` not running / incompatible format → `@try`/`@catch` + engine guard in `friend_forcePlayAudioEventRTS` |
| **SIGABRT — Metal vs custom allocator** | ✅ FIXED | Global `operator new`/`delete` conflicted with Metal/AppKit → macOS uses `calloc`/`free` instead of `DynamicMemoryAllocator` |
| **SIGSEGV in W3DBridgeBuffer constructor** | ✅ FIXED | `m_numBridges` not initialized before `clearAllBridges()` → garbage loop. Initialization + `calloc` |
| **SIGSEGV in Pathfinder constructor** | ✅ FIXED | Uninitialized fields when switching to system `malloc` → resolved via `calloc` (zeroing) |

### Key Lesson: CMake Vtable Mismatch

`Platform/MacOS/CMakeLists.txt` — `macos_platform` is compiled once as a STATIC library.
- `zi_always` (provides `RTS_ZEROHOUR=1`) must be **PRIVATE** to avoid leaking into the Generals target.
- Include paths must point to `GeneralsMD/` (not `Generals/`), otherwise the wrong `GameClient.h` is used.
- Zero Hour `GameClient.h` has 2 additional pure-virtual methods (`notifyTerrainObjectMoved`, `createSnowManager`) which shift the vtable.

---

## 📋 BACKLOG

| Task | Priority | Notes |
|:---|:---|:---|
| Input handling | **High** | Keyboard/Mouse via Cocoa events — verify event routing |
| Full Metal rendering | **High** | Textures, shaders, 3D scene — verify with real rendering |
| UI / Menu rendering | **High** | Interface elements, buttons, text — verify drawing |
| Audio playback | **Medium** | Wrapped in `@try`/`@catch`, needs `.big` file loading verification |
| .big archives mounting | **Medium** | Asset loading works (INI files load), completeness unverified |
| WOL authorization | Low | Browser excluded. Possibly REST API. |
| Cross-platform LAN wire format | Low | `wchar_t` is 4B on macOS vs 2B on Windows |

---

## 📜 Commit History

| Commit | Description |
|:---|:---|
| `2dfbe0e4` | feat(macos): Clean restart — Platform/MacOS/ (78 files), CMake skeleton |
| `1f33c17a` | fix(macos): Resolve compilation errors (windows.h shims, PCH, exclusions) |
| `0edf1903` | fix(macos): Replace DX8 SDK with d3d8_stub.h + shim d3dx8 headers |
| `ac60483f` | fix: resolve macos_platform compilation errors — Carbon compat, D3DX stubs, override mismatches, memory pool macros |
| `3b3130e5` | fix(macos): W3DDisplay, W3DGameEngine, windows.h stubs — iterative build fixes (z_gameenginedevice progress) |
| `a2e7a7ba` | **macOS: resolve all linker errors — successful build 🎉** (31 files, +1121/-81) |

---

## 📚 External References
| Project | GitHub Link | Local Paths | Description |
|:---|:---|:---|:---|
| **TheSuperHackers** | [TheSuperHackers/Generals](https://github.com/TheSuperHackers/GeneralsGameCode) | `/Users/okji/dev/games/GeneralsX` | Upstream, modernized C++20 |
| **Fighter19 (Linux)** | [Fighter19/CnC_Generals](https://github.com/Fighter19/CnC_Generals_Zero_Hour) | `/Users/okji/dev/games/CnC_Generals_Zero_Hour` | Native Linux port reference |
| **GeneralsGamePatch** | [TheSuperHackers/GeneralsGamePatch](https://github.com/TheSuperHackers/GeneralsGamePatch/) | `/Users/okji/dev/games/GeneralsGamePatch` | Game data & assets (INI, maps, textures, audio, localization) |
| **Aspyr Port (macOS)** | N/A | `/Applications/Command & Conquer™ Generals Zero Hour.app` | Binary reference only |

---

## 📜 Golden Rules
1. **Don't touch `Core/`**: Platform code lives in `Platform/MacOS/Source/`. Minimal edits in `Core/`.
2. **Minimal `windows.h`**: Shim contains only what is actually needed. Add on build error.
3. **Exclude, don't patch**: Problematic Windows files are excluded from the build via CMake.
4. **`d3d8_stub.h` is the source of truth**: All DX8 interfaces on macOS only via stub, not the original SDK.
5. **Unified Pipeline**: All rendering through `MetalDevice8`. No side-channels.
