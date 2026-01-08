# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

GeneralsGameCode is a community project to fix and improve *Command & Conquer: Generals* and *Zero Hour*. The codebase was modernized from Visual Studio 6/C++98 to Visual Studio 2022/C++20 while maintaining retail compatibility.

## Build Commands

### Windows
```bash
# Configure (choose a preset)
cmake --preset vc6          # VC6 release build (retail compatible)
cmake --preset win32        # Modern VS2022 build
cmake --preset vc6-debug    # Debug build (breaks retail compatibility)

# Build
cmake --build build/vc6
cmake --build build/win32 --config Release

# Build with tools
cmake --build build/vc6 --target generalszh_tools generalszh_extras
```

### Linux (via Docker)
```bash
# Build using Docker (produces Windows executables)
./scripts/build-linux.sh              # Full build
./scripts/build-linux.sh --game zh    # Zero Hour only

# Install to existing game
./scripts/install-to-game.sh --detect

# See BUILDING.md for detailed instructions
```

Key presets: `vc6`, `vc6-debug`, `vc6-profile`, `win32`, `win32-debug`, `win32-vcpkg`

## Replay Compatibility Testing

```bash
# Run replay tests (requires VC6 optimized build with RTS_BUILD_OPTION_DEBUG=OFF)
generalszh.exe -jobs 4 -headless -replay subfolder/*.rep
```

Replays in `GeneralsReplays/` are used for CI compatibility testing.

## Clang-Tidy

```bash
# Generate compile commands database
cmake -B build/clang-tidy -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -G Ninja

# Run custom checks
clang-tidy -p build/clang-tidy --checks='-*,generals-use-is-empty,generals-use-this-instead-of-singleton' file.cpp
```

## Architecture

### Dual Game Structure
- **GeneralsMD/**: Zero Hour (v1.04) - **primary development focus**
- **Generals/**: Original Generals (v1.08)
- **Core/**: Shared engine and libraries used by both

Changes to Zero Hour take precedence. When code is not shared, implement for Zero Hour first, then replicate to Generals with identical implementation.

### Engine Separation (Core/GameEngine/)
- **GameLogic** (`Include/GameLogic/`): Game state, rules, simulation - deterministic for multiplayer/replays
- **GameClient** (`Include/GameClient/`): Rendering, UI, platform-specific code
- **Common** (`Include/Common/`): Shared utilities (AsciiString, UnicodeString, file systems, audio)

### Library Stack (Core/Libraries/)
- **WWVegas**: Graphics framework (WW3D2, WWAudio, WWLib, WWMath)
- **rts/**: RTS-specific utilities

### Key Singletons
Global singletons like `TheGameLogic`, `TheGlobalData` are used throughout. Inside member functions of the same class, use member access directly instead of the singleton reference.

## Code Change Requirements

### Comment Format (Mandatory for User-Facing Changes)
```cpp
// TheSuperHackers @keyword author DD/MM/YYYY Description
```

Keywords: `@bugfix`, `@feature`, `@performance`, `@refactor`, `@tweak`, `@build`, `@fix`, `@info`, `@todo`

### Pull Request Titles
Use conventional commits format:
```
bugfix(scope): Fix description starting with action verb
feat(scope): Add new feature
refactor(scope): Improve code structure
perf(scope): Optimize performance
```

### Code Style
- Match surrounding legacy code style
- Prefer C++98 patterns unless modern features add significant value
- Separate refactors from logical changes into different commits
- Use present tense in documentation ("Fixes" not "Fixed")
- Prefix unit names with faction (e.g., "USA Missile Defender" not "MD")

## Code Quality Principles

### SOLID Principles
When adding new code, follow SOLID principles where practical:

- **Single Responsibility (SRP)**: Each class/function should have one reason to change. Split large classes into focused components.
- **Open/Closed (OCP)**: Design for extension over modification. Use virtual functions and inheritance for varying behavior.
- **Liskov Substitution (LSP)**: Derived classes must be substitutable for their base classes without breaking behavior.
- **Interface Segregation (ISP)**: Prefer focused interfaces. Don't force classes to implement methods they don't need.
- **Dependency Inversion (DIP)**: High-level modules shouldn't depend on low-level modules. Both should depend on abstractions.

### Clean Code Practices
- **Meaningful names**: Use descriptive variable and function names that reveal intent
- **Small functions**: Functions should do one thing and do it well (aim for <20 lines)
- **DRY (Don't Repeat Yourself)**: Extract common code into reusable functions
- **Fail fast**: Validate inputs early and return/throw on invalid state
- **Minimize nesting**: Use early returns to reduce indentation depth
- **Comments**: Code should be self-documenting; comments explain *why*, not *what*

### Legacy Code Considerations
This is a legacy codebase (C++98 era). When modifying existing code:
- Maintain consistency with surrounding code style
- Avoid introducing modern patterns that clash with existing architecture
- Make incremental improvements rather than large refactors
- Keep retail binary compatibility in mind for VC6 builds

## Build Options

Key CMake options in `cmake/config-build.cmake`:
- `RTS_BUILD_ZEROHOUR` / `RTS_BUILD_GENERALS`: Select which game to build
- `RTS_BUILD_OPTION_DEBUG`: Debug build (breaks retail compatibility)
- `RTS_BUILD_OPTION_PROFILE`: Profiling build
- `RTS_BUILD_ZEROHOUR_TOOLS`: Build mod tools

## Dependencies

- **VC6 builds**: Require MSVC 6.0 toolchain (CI uses itsmattkc/MSVC600)
- **Modern builds**: Visual Studio 2022, Ninja
- **vcpkg**: Optional (zlib, ffmpeg for enhanced builds)
- **Platform libs**: DirectX 8, Miles Sound System, Bink Video (Windows 32-bit only)
