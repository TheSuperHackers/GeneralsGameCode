# macOS Port — Documentation

> **Command & Conquer: Generals — Zero Hour** on Apple Silicon (ARM64)

<div align="center">
  
![](demo_generals_arm64.mp4)

</div>

This is the official documentation hub for the macOS/Metal port of Generals Zero Hour. The port translates the original DirectX 8 rendering pipeline to Apple Metal, replaces Win32 subsystems with Cocoa/AVFoundation equivalents, and builds natively for ARM64.

## 📖 Documents

| Document | Description |
|:---|:---|
| **[Setup Guide](SETUP.md)** | Prerequisites, build instructions, and how to run the game |
| **[Changelog](CHANGELOG.md)** | Resolved issues, milestones, and commit history |
| **[Development Guide](DEVELOPMENT.md)** | Architecture, conventions, gotchas, and golden rules for contributors |
| **[Rendering Pipeline](RENDERING.md)** | Metal backend architecture, DX8→Metal translation, shader details |
| **[Build System](BUILD_SYSTEM.md)** | CMake structure, dependency graph, platform targets |
| **[Reference Materials](reference/README.md)** | DX8 specs, engine architecture analysis, rendering flow diagrams |

## 🚀 Quick Start

```bash
# Build & Run (recommended)
sh build_run_mac.sh

# Or manually:
cmake --preset macos
cmake --build build/macos
GENERALS_INSTALL_PATH="/path/to/game/" GENERALS_FPS_LIMIT=60 build/macos/GeneralsMD/generalszh -quick
```

## 📊 Current Status

| Metric | Value |
|:---|:---|
| **Build** | ✅ Successful — `generalsv` + `generalszh` |
| **Runtime** | 🟢 **Stable** — 5500+ loop iterations, no crashes |
| **Game Loop** | ✅ Shell map → cutscenes → missions all work |
| **Rendering** | 🟡 Terrain + UI working, some 3D textures white |
| **Audio** | ❌ Stubbed (SIGSEGV workaround — needs fix) |
| **Input** | ✅ Keyboard + Mouse fully working |
| **Crashes Resolved** | 22 |

## 🏗 Architecture Overview

```
Platform/MacOS/
├── CMakeLists.txt              # Platform build config
├── Include/                    # Headers (d3d8_stub.h, win_compat.h)
├── Source/
│   ├── Main/                   # Entry point, window, input, game client
│   ├── Metal/                  # MetalDevice8 — DX8→Metal backend (95KB+)
│   ├── Audio/                  # MacOSAudioManager (partially stubbed)
│   ├── Client/                 # Display, text rendering (CoreText)
│   └── Stubs/                  # GameSpy, Win32, network stubs
└── docs/                       # ← You are here
```

## 📝 Branch

All macOS work lives on `feature/macos-c_make`.
