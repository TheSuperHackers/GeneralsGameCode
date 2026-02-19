# macOS Port — Setup Guide

## Prerequisites

| Requirement | Version | Notes |
|:---|:---|:---|
| **macOS** | 13+ (Ventura) | Apple Silicon (ARM64) recommended |
| **Xcode Command Line Tools** | Latest | `xcode-select --install` |
| **CMake** | 3.25+ | `brew install cmake` |
| **Ninja** | Latest | `brew install ninja` |
| **Game Data** | Generals: Zero Hour | `.big` files from retail install |

## Build Instructions

### 1. Clone the Repository

```bash
git clone https://github.com/TheSuperHackers/GeneralsGameCode.git
cd GeneralsGameCode
git checkout feature/macos-c_make
```

### 2. Configure

```bash
cmake --preset macos
```

This configures with:
- **Generator:** Ninja
- **Build type:** Debug
- **Architecture:** ARM64 (native Apple Silicon)
- **C++ Standard:** C++20

### 3. Build

```bash
cmake --build build/macos
```

Produces two executables:
- `build/macos/Generals/generalsv` — Generals (~21MB)
- `build/macos/GeneralsMD/generalszh` — Zero Hour (~22MB)

### 4. Install Game Data

The game needs `.big` asset archives to run. Copy them from a retail install:

```bash
# From your Generals Zero Hour installation
cp /path/to/game/Data/*.big Data/
```

Required `.big` files include: `INIZH.big`, `W3DZH.big`, `TexturesZH.big`, `TerrainZH.big`, `WindowZH.big`, `ShadersZH.big`, `AudioZH.big`, and others.

### 5. Run

```bash
# Always kill previous instances first!
killall generalszh 2>/dev/null; sleep 1

# Run Zero Hour
build/macos/GeneralsMD/generalszh
```

## Logging

The game outputs debug logs to stderr. To capture:

```bash
./generalszh > /tmp/generals.log 2>&1
```

Useful log patterns:
- `initSubsystem:` — Subsystem initialization progress
- `Signal received:` — Crash signals (followed by stack trace)
- `DEBUG:` — Metal rendering and game loop heartbeats
- `MACOS AUDIO:` — Audio system status

## Troubleshooting

### Game doesn't start
- Ensure `.big` files are in the `Data/` directory
- Check for stale processes: `killall generalszh`
- Run with logging: `./generalszh 2>&1 | tee /tmp/debug.log`

### Window appears but screen is black
- Metal rendering is initializing. Check log for `BeginScene`/`Present` calls
- Verify Metal is supported: `system_profiler SPDisplaysDataType | grep Metal`

### Build errors after pulling
- Clean and rebuild: `rm -rf build && cmake --preset macos && cmake --build build/macos`
