# macOS Port — Build System

This document explains the CMake build system structure for the macOS port.

---

## Build Commands

```bash
cmake --preset macos            # Configure (Ninja, Debug, ARM64)
cmake --build build/macos       # Build both targets
killall generalszh 2>/dev/null; sleep 1
build/macos/GeneralsMD/generalszh   # Run Zero Hour
```

---

## Build Flow Diagram

```
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
  │  ┌─ DX8 (APPLE) ──────────┼──► d3d8_stub.h (pure C++ ifaces) │
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
  │  └─ zlib (APPLE) ─────────┼──► System zlib (find_package)     │
  │                           │                                   │
  ├───────────────────────────┼───────────────────────────────────┤
  │     TARGETS               │                                   │
  │                           │                                   │
  │  macos_platform ──────────┼──► Platform/MacOS/CMakeLists.txt  │
  │    STATIC library         │    MetalDevice8, MacOSMain,       │
  │    PRIVATE: zi_always     │    Stubs, Audio, Display          │
  │    Links: Metal, AppKit,  │    ⚠️  zi_always = PRIVATE!       │
  │      AVFoundation,        │                                   │
  │      QuartzCore           │                                   │
  │                           │                                   │
  │  generalsv ───────────────┼──► Generals (21MB)                │
  │    Links: g_gameengine,   │                                   │
  │      g_gameenginedevice,  │                                   │
  │      macos_platform       │                                   │
  │                           │                                   │
  │  generalszh ──────────────┼──► Zero Hour (22MB)               │
  │    Links: z_gameengine,   │                                   │
  │      z_gameenginedevice,  │                                   │
  │      macos_platform       │                                   │
  │                           │                                   │
  └───────────────────────────┴───────────────────────────────────┘
```

---

## Key CMake Targets

| Target | Type | Output | Description |
|:---|:---|:---|:---|
| `macos_platform` | STATIC | `libmacos_platform.a` | All macOS-specific code |
| `generalsv` | EXECUTABLE | `Generals/generalsv` | Generals (original) |
| `generalszh` | EXECUTABLE | `GeneralsMD/generalszh` | Zero Hour |
| `g_gameengine` | STATIC | `libg_gameengine.a` | Generals engine library |
| `z_gameengine` | STATIC | `libz_gameengine.a` | Zero Hour engine library |
| `g_gameenginedevice` | STATIC | `libg_gameenginedevice.a` | Generals device library |
| `z_gameenginedevice` | STATIC | `libz_gameenginedevice.a` | Zero Hour device library |

---

## macOS Framework Dependencies

| Framework | Purpose |
|:---|:---|
| `Metal` | GPU rendering |
| `MetalKit` | Metal utilities |
| `AppKit` | Window management, events |
| `QuartzCore` | `CAMetalLayer` |
| `AVFoundation` | Audio playback |
| `CoreText` | Text rendering |

---

## ⚠️ Critical: `zi_always` Must Be PRIVATE

`macos_platform` is a STATIC library linked by **both** `generalsv` and `generalszh`.

`zi_always` provides `RTS_ZEROHOUR=1`. If it leaks as PUBLIC:
- Generals target gets `RTS_ZEROHOUR` → wrong vtable layout
- Compilation may succeed but runtime crashes from vtable mismatch

**Rule:** Always use `target_link_libraries(macos_platform PRIVATE zi_always)`.

---

## Preset Configuration

`CMakePresets.json` defines the `macos` preset:

```json
{
  "name": "macos",
  "generator": "Ninja",
  "binaryDir": "build/macos",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Debug",
    "CMAKE_OSX_ARCHITECTURES": "arm64"
  }
}
```
