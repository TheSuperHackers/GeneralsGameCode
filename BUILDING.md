# Building Generals Game Code

This document provides comprehensive instructions for building Command & Conquer: Generals and Zero Hour from source on Windows and Linux.

## Table of Contents

- [Overview](#overview)
- [Prerequisites](#prerequisites)
- [Building on Windows](#building-on-windows)
  - [Visual Studio 2022 Build](#visual-studio-2022-build)
  - [VC6 Build (Retail Compatible)](#vc6-build-retail-compatible)
- [Building on Linux](#building-on-linux)
  - [Docker Build (Recommended)](#docker-build-recommended)
  - [Running the Game on Linux](#running-the-game-on-linux)
- [Build Presets](#build-presets)
- [Build Targets](#build-targets)
- [Troubleshooting](#troubleshooting)

## Overview

The Generals Game Code can be built using different compilers and configurations:

| Build Type | Platform | Compiler | Notes |
|------------|----------|----------|-------|
| VC6 | Windows | Visual Studio 6 | Retail-compatible binaries |
| Win32 | Windows | Visual Studio 2022 | Modern build with C++20 |
| Docker | Linux | Wine + VC6 | Cross-compiles Windows binaries |

**Important**: The game engine uses DirectX 8, Miles Sound System, and Bink Video, which are Windows-only technologies. All builds produce Windows executables (.exe files). Linux users run these via Wine.

## Prerequisites

### Windows

- **CMake 3.25+**: Download from [cmake.org](https://cmake.org/download/)
- **Ninja**: Download from [ninja-build.org](https://ninja-build.org/) or install via package manager
- **Git**: For cloning the repository and fetching dependencies

For **Visual Studio 2022** builds:
- Visual Studio 2022 with "Desktop development with C++" workload
- Windows SDK

For **VC6** builds (retail-compatible):
- Visual Studio 6.0 (MSVC 6) - see CI configuration for setup

### Linux

- **Docker**: Install from [docker.com](https://docs.docker.com/engine/install/)
- **Wine** (optional): For running the built executables

```bash
# Debian/Ubuntu
sudo apt-get install docker.io wine

# Fedora
sudo dnf install docker wine

# Arch Linux
sudo pacman -S docker wine
```

## Building on Windows

### Visual Studio 2022 Build

This is the recommended build for development on Windows.

```powershell
# Configure
cmake --preset win32

# Build (Release)
cmake --build build/win32 --config Release

# Build (Debug)
cmake --build build/win32 --config Debug
```

Executables are placed in `build/win32/GeneralsMD/` and `build/win32/Generals/`.

### VC6 Build (Retail Compatible)

The VC6 build produces binaries compatible with the retail game, allowing replay compatibility testing.

```powershell
# Configure
cmake --preset vc6

# Build
cmake --build build/vc6
```

**Note**: VC6 builds require the Visual Studio 6 toolchain. The CI uses [itsmattkc/MSVC600](https://github.com/itsmattkc/MSVC600) portable installation.

## Building on Linux

### Docker Build (Recommended)

The Docker build compiles Windows executables using Wine and VC6 inside a container. This is the recommended way to build on Linux.

#### Quick Start

```bash
# Full build (both games)
./scripts/build-linux.sh

# Build Zero Hour only
./scripts/build-linux.sh --game zh

# Build Generals only
./scripts/build-linux.sh --game generals
```

#### Using the Docker Script Directly

```bash
# Build with the original script
./scripts/dockerbuild.sh

# Force CMake reconfiguration
./scripts/dockerbuild.sh --cmake

# Enter interactive shell in container
./scripts/dockerbuild.sh --bash
```

#### Build Output

After a successful build, executables are located in:

```
build/docker/
    GeneralsMD/             # Zero Hour
        generalszh.exe          # Main game
        WorldBuilderZH.exe      # Map editor
        W3DViewZH.exe           # 3D model viewer
        guiedit.exe             # GUI editor
        ParticleEditor.dll      # Particle editor
        ...
    Generals/               # Original Generals
        generalsv.exe           # Main game
        WorldBuilderV.exe       # Map editor
        W3DViewV.exe            # 3D model viewer
        ...
```

### Installing to Existing Game

The build produces Windows executables that need the original game data files (textures, sounds, maps, etc.) to run. You must have a legal copy of Command & Conquer: Generals Zero Hour installed.

#### Using the Installation Script

```bash
# Auto-detect game installation and install built files
./scripts/install-to-game.sh --detect

# Or specify the game directory manually
./scripts/install-to-game.sh "/path/to/Generals Zero Hour"

# Restore original files
./scripts/install-to-game.sh --restore "/path/to/Generals Zero Hour"

# Dry run (show what would be installed)
./scripts/install-to-game.sh --dry-run "/path/to/Generals Zero Hour"
```

#### Manual Installation

If you prefer manual installation:

1. Locate your game installation directory (e.g., `C:\Program Files\EA Games\Command and Conquer Generals Zero Hour\Data\`)
2. Backup the original executables (e.g., `generalszh.exe` → `generalszh.exe.backup`)
3. Copy the built executables from `build/docker/GeneralsMD/` to the game's `Data/` directory:
   - `generalszh.exe` - Main game
   - `WorldBuilderZH.exe` - Map editor
   - `W3DViewZH.exe` - Model viewer
   - `DebugWindow.dll` - Debug tools
   - `ParticleEditor.dll` - Particle editor

**Important**: Do NOT copy `mss32.dll` or `binkw32.dll` from the build directory - these are stub libraries for compilation only. Keep the original DLLs from your game installation.

### Running the Game on Linux

After installing the built executables to your game directory:

```bash
# Set up Wine prefix (first time only)
export WINEPREFIX=~/.wine-generals
export WINEARCH=win32
wineboot

# Run the game
cd "/path/to/Generals Zero Hour/Data"
wine generalszh.exe

# With quickstart (skip intro videos)
wine generalszh.exe -quickstart
```

**Requirements**:
- Original game data files installed (from retail, Origin, or other legal source)
- Wine 5.0+ recommended (Wine 10.0+ for best compatibility)
- DirectX 8 compatible graphics (Wine handles translation)

#### Wine Configuration Tips

```bash
# Use winetricks for better DirectX support
winetricks d3dx9 vcrun6

# Use 32-bit Wine prefix (required for this game)
WINEPREFIX=~/.wine32 WINEARCH=win32 wine generalszh.exe

# Enable DXVK for better performance (optional)
# Install DXVK to your Wine prefix for DirectX to Vulkan translation
```

## Build Presets

Available CMake presets:

| Preset | Description |
|--------|-------------|
| `vc6` | VC6 release build (retail compatible) |
| `vc6-debug` | VC6 debug build |
| `vc6-profile` | VC6 with profiling enabled |
| `win32` | VS2022 release build |
| `win32-debug` | VS2022 debug build |
| `win32-vcpkg` | VS2022 with vcpkg dependencies |
| `unix` | Unix experimental build (limited) |

List all presets:
```bash
cmake --list-presets
```

## Build Targets

### Game Executables

| Target | Game | Description |
|--------|------|-------------|
| `generalszh` | Zero Hour | Main game executable |
| `generalsv` | Generals | Main game executable |

### Tools

| Target | Description |
|--------|-------------|
| `generalszh_tools` | All Zero Hour tools |
| `generalsv_tools` | All Generals tools |
| `WorldBuilderZH` | Zero Hour map editor |
| `WorldBuilderV` | Generals map editor |
| `W3DViewZH` | Zero Hour 3D model viewer |
| `W3DViewV` | Generals 3D model viewer |

Build specific target:
```bash
cmake --build build/vc6 --target generalszh
```

## Build Options

CMake options that can be set during configuration:

| Option | Default | Description |
|--------|---------|-------------|
| `RTS_BUILD_ZEROHOUR` | ON | Build Zero Hour code |
| `RTS_BUILD_GENERALS` | ON | Build Generals code |
| `RTS_BUILD_OPTION_DEBUG` | OFF | Debug build (breaks retail compatibility) |
| `RTS_BUILD_OPTION_PROFILE` | OFF | Enable profiling |
| `RTS_BUILD_OPTION_FFMPEG` | OFF | Enable FFmpeg support |

Example:
```bash
cmake --preset vc6 -DRTS_BUILD_GENERALS=OFF  # Zero Hour only
```

## Troubleshooting

### Docker Build Issues

**"Permission denied" when running Docker**
```bash
# Add user to docker group
sudo usermod -aG docker $USER
# Log out and back in, or run:
newgrp docker
```

**Build fails with "out of memory"**
- Increase Docker memory limit in Docker Desktop settings
- Or build specific targets instead of all:
  ```bash
  ./scripts/build-linux.sh --target generalszh
  ```

### Wine Runtime Issues

**"d3d8.dll not found"**
- Install Wine's DirectX components:
  ```bash
  winetricks d3dx9
  ```

**Game crashes on startup**
- Try using a 32-bit Wine prefix
- Check game data files are properly installed
- Verify registry paths point to correct game directory

### Windows Build Issues

**"CMake Error: CMake was unable to find a build program"**
- Ensure Ninja is installed and in PATH
- Or use Visual Studio generator: `cmake -G "Visual Studio 17 2022" -A Win32 ..`

**"Cannot find compiler"**
- Open "Developer Command Prompt for VS 2022" before running CMake
- Or ensure Visual Studio C++ tools are installed

### General Issues

**Replay compatibility test failures**
- Use VC6 optimized build with `RTS_BUILD_OPTION_DEBUG=OFF`
- Don't mix debug and release builds

**Missing dependencies**
- Dependencies are fetched automatically via CMake FetchContent
- Ensure internet connection during first build

## Additional Resources

- [CLAUDE.md](CLAUDE.md) - AI assistant instructions for this codebase
- [.github/workflows/](.github/workflows/) - CI build configurations
- [cmake/](cmake/) - CMake configuration modules
