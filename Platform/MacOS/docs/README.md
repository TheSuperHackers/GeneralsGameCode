# macOS Port — Documentation

> **Command & Conquer: Generals — Zero Hour** on Apple Silicon (ARM64)

This is the official documentation hub for the macOS/Metal port of Generals Zero Hour. The port translates the original DirectX 8 rendering pipeline to Apple Metal, replaces Win32 subsystems with Cocoa/AVFoundation equivalents, and builds natively for ARM64.

## 📖 Documents

| Document | Description |
|:---|:---|
| **[Setup Guide](SETUP.md)** | Prerequisites, build instructions, and how to run the game |
| **[Changelog](CHANGELOG.md)** | Resolved issues, milestones, and commit history |
| **[Development Guide](DEVELOPMENT.md)** | Architecture, conventions, gotchas, and golden rules for contributors |
| **[Rendering Pipeline](RENDERING.md)** | Metal backend architecture, DX8→Metal translation, shader details |
| **[Build System](BUILD_SYSTEM.md)** | CMake structure, dependency graph, platform targets |
| **[Reference Materials](reference/README.md)** | DX8 specs, engine architecture analysis, rendering flow diagrams, screenshots |

## 🚀 Quick Start

```bash
# Configure
cmake --preset macos

# Build
cmake --build build/macos

# Run (kill previous instances first!)
killall generalszh 2>/dev/null; sleep 1
build/macos/GeneralsMD/generalszh
```

## 📊 Current Status

| Metric | Value |
|:---|:---|
| **Build** | ✅ Successful — `generalsv` (21MB) + `generalszh` (22MB) |
| **Runtime** | 🟢 Stable — 35+ seconds, 400+ frames, zero crashes |
| **Crashes Resolved** | 10 |
| **Rendering** | Metal Clear/BeginScene/Present working |
| **Audio** | AVAudioEngine initialized, graceful fallback |
| **Input** | Cocoa events → game engine (needs testing) |

## 🏗 Architecture Overview

```
Platform/MacOS/
├── CMakeLists.txt              # Platform build config
├── Include/                    # Headers (d3d8_stub.h, win_compat.h)
├── Source/
│   ├── Main/                   # Entry point, window, input, game client
│   ├── Metal/                  # MetalDevice8 — DX8→Metal backend (85KB+)
│   ├── Audio/                  # AVAudioEngine audio manager
│   ├── Client/                 # Display, text rendering (CoreText)
│   └── Stubs/                  # GameSpy, Win32, network stubs
└── docs/                       # ← You are here
```

## 📝 Branch

All macOS work lives on `feature/macos-c_make`.
