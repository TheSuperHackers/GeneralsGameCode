# Local Build Setup Guide

This guide provides detailed instructions for building the Generals Game Code project locally on Windows and Linux.

## Prerequisites

### Windows (Visual Studio 2022)

- **Visual Studio 2022** with the "Desktop development with C++" workload installed
  - Must include x86 target support
  - Community Edition is sufficient
- **CMake** >= 3.25 (available from [cmake.org](https://cmake.org))
- **Ninja** build system
  - Install via: `winget install Ninja-build.Ninja`

### Linux / Unix (via vcpkg)

- **CMake** >= 3.25
- **vcpkg** (set `VCPKG_ROOT` environment variable to your vcpkg installation)
- Standard C++ build tools (g++/clang)

### Dependencies

The project uses **CMake FetchContent** to automatically download required dependencies:
- Open stub libraries replace proprietary SDKs (Miles, Bink, DirectX 8, GameSpy)
- No manual dependency installation required for the `win32` preset
- vcpkg is only needed for `win32-vcpkg`, `unix`, and cross-compilation presets

## Quick Start

### Windows (Visual Studio 2022)

Run this command in Git Bash or PowerShell from the repository root:

```bash
cmd.exe /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x86 && cd /d C:\openSAGE-zh && cmake --preset win32 && cmake --build build/win32 --config Release --target z_generals'
```

This command:
1. Sets up the Visual Studio environment for x86 compilation
2. Configures the CMake build with the `win32` preset
3. Builds the game in Release mode to `build/win32/Release/z_generals`

### Linux (via Docker)

For Linux builds, use the provided Docker setup:

```bash
./scripts/docker-build.sh              # Build using Docker
./scripts/docker-install.sh --detect   # Install to your game directory
```

The Docker build compiles the project in a Wine environment with VC6 toolchain. This is maintained as an alternative for CI and reproducible builds.

## CMake Preset Reference

The project provides several CMake presets for different build configurations. Use the preset that matches your needs:

| Preset Name | Compiler | Target | When to Use | Notes |
|---|---|---|---|---|
| `win32` | MSVC 2022 | Win32 (x86) Release | **Recommended for local development** | Uses CMake FetchContent; no vcpkg required |
| `win32-debug` | MSVC 2022 | Win32 (x86) Debug | Local debugging with symbols | Enables debug output and logging |
| `win32-profile` | MSVC 2022 | Win32 (x86) Release | Performance profiling with Tracy | Requires Tracy profiler (see below) |
| `win32-vcpkg` | MSVC 2022 | Win32 (x86) Release | Alternative with vcpkg | Requires `VCPKG_ROOT` environment variable |
| `win32-vcpkg-debug` | MSVC 2022 | Win32 (x86) Debug | Debug builds with vcpkg | Requires `VCPKG_ROOT` |
| `win32-vcpkg-profile` | MSVC 2022 | Win32 (x86) Release | Profile builds with vcpkg | Requires `VCPKG_ROOT` |
| `unix` | GCC/Clang | Unix (32-bit) Release | Linux builds | Requires `VCPKG_ROOT` and 32-bit libc |
| `mingw-w64-i686` | MinGW-w64 | Win32 (x86) Release | Cross-compile or MinGW setup | Uses Unix Makefiles; requires MinGW toolchain |
| `mingw-w64-i686-debug` | MinGW-w64 | Win32 (x86) Debug | MinGW debug builds | Debug variant of MinGW preset |
| `mingw-w64-i686-profile` | MinGW-w64 | Win32 (x86) Release | MinGW profiling | Profile variant of MinGW preset |
| `vc6`, `vc6-debug`, `vc6-profile`, `vc6-releaselog`, `vc6-weekly` | VC6 | Win32 (x86) | CI only | Requires legacy VC6 toolchain; not available on modern machines |

### Choosing a Preset

- **Local development**: `win32` (simplest, no setup needed)
- **Debugging**: `win32-debug`
- **Performance testing**: `win32-profile` (requires Tracy)
- **CI/reproducible builds**: Docker via `./scripts/docker-build.sh`

## Performance Profiling with Tracy

The `win32-profile` preset enables Tracy profiler integration. To use it:

1. Download [Tracy v0.13.1](https://github.com/wolfpld/tracy/releases/tag/v0.13.1)
2. Build with: `cmake --preset win32-profile && cmake --build build/win32-profile --config Release`
3. Connect Tracy profiler (`tracy-profiler.exe`) to the running game

**Troubleshooting**: If you encounter errors when using Tracy, remove `dbghelp.dll` from the game's binary directory.

## CMake 4.x Compatibility

If you're using CMake 4.x, you may encounter policy errors when fetching dependencies:

```
CMake Error: Policy CMP00XX ... is not known to this version of CMake
```

**Workaround**: Add `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` to your cmake configure command:

```bash
cmake --preset win32 -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

This allows CMake 4.x to accept the older dependency configurations.

## Build Output

After a successful build, the compiled binary will be located at:

- `build/win32/Release/z_generals.exe` (Windows Release)
- `build/win32-debug/Debug/z_generals.exe` (Windows Debug)
- `build/unix/Release/z_generals` (Linux)

## Testing

For replay-based testing and validation of your build, see [TESTING.md](../TESTING.md).

## Troubleshooting

### Visual Studio environment not found

Ensure Visual Studio 2022 is installed with the C++ development workload. Verify the path in the build command matches your installation:
```
C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat
```

### Ninja not found

Install Ninja via:
```bash
winget install Ninja-build.Ninja
```

Then ensure it's in your PATH or restart your terminal.

### CMake preset errors

Clear the build directory and reconfigure:
```bash
rm -rf build/
cmake --preset win32
```

### VCPKG_ROOT not set

If using a `*-vcpkg` or `unix` preset, ensure your environment variable points to a valid vcpkg installation:
```bash
export VCPKG_ROOT=/path/to/vcpkg  # Linux/macOS
set VCPKG_ROOT=C:\path\to\vcpkg   # Windows cmd
```

## Contributing Builds

When contributing back to the project, all builds should pass the standard `win32` preset without manual vcpkg configuration. Use CI workflows for alternative presets.
