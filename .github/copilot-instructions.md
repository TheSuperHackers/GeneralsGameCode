# AI Coding Agent Instructions

## Project Overview

This is the **GeneralsGameCode** project - a community-driven effort focused on improving *Command & Conquer: Generals Zero Hour*. The supported build is GeneralsMD with Clang/LLVM, C++23, and Windows x64.

## Architecture

### Game Structure
- **GeneralsMD/**: Zero Hour expansion (v1.04) codebase - **supported target**
- **Core/**: Shared game engine and libraries used by both games

### Key Components
- **Core/GameEngine/**: Base game engine with GameClient/GameLogic separation
- **Core/Libraries/**: Internal libraries including WWVegas graphics framework
- **Core/GameEngineDevice/**: Platform-specific rendering (DirectX 8)
- **Core/Tools/**: Development tools (W3DView, texture compression, etc.)
- **Dependencies/**: External dependencies and utilities

## Build System

### CMake Preset (Critical)
- **clang-windows-x64**: GeneralsMD, Clang/LLVM, Windows x64, C++23
- Use `cmake --preset clang-windows-x64` followed by `cmake --build --preset clang-windows-x64`

### Build Commands
```bash
# Configure with the supported toolchain
cmake --preset clang-windows-x64

# Build (from project root)
cmake --build --preset clang-windows-x64
```

### Retail Compatibility
- Release builds are used for replay compatibility testing
- Use RTS_BUILD_OPTION_DEBUG=OFF for compatibility testing

## Development Workflow

### Code Change Documentation
**Every user-facing change requires TheSuperHackers comment format:**
```cpp
// TheSuperHackers @keyword author DD/MM/YYYY Description
```

Common keywords: `@bugfix`, `@feature`, `@performance`, `@refactor`, `@tweak`, `@build`

### Pull Request Guidelines
- Title format: `type: Description starting with action verb`
- Types: `bugfix:`, `feat:`, `fix:`, `refactor:`, `perf:`, `build:`
- Zero Hour changes take precedence over Generals
- Changes must be identical between both games when applicable

### Code Style
- Maintain consistency with surrounding legacy code
- Prefer C++98 style unless modern features add significant value
- No big refactors mixed with logical changes
- Use present tense in documentation ("Fixes" not "Fixed")

## Testing

### Replay Compatibility Testing
Located in `GeneralsReplays/` - critical for ensuring retail compatibility:
```bash
generalszh.exe -jobs 4 -headless -replay subfolder/*.rep
```
- Requires an optimized Clang/LLVM build with RTS_BUILD_OPTION_DEBUG=OFF
- Copies replays to `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Replays`
- CI automatically tests GeneralsMD builds against known replays

### Build Validation
- CI tests the Clang/LLVM Windows x64 preset
- Path-based change detection triggers relevant builds
- Tools and extras are built with `+t+e` flags

## Common Patterns

### Memory Management
- Manual memory management (delete/delete[]) - this is legacy C++98 code
- The legacy STLPort/VC6 compatibility layer is no longer part of the build

### Game Engine Separation
- **GameLogic**: Game state, rules, simulation
- **GameClient**: Rendering, UI, platform-specific code
- Clean separation maintained for potential future networking

### Module Structure
```
Core/
├── GameEngine/Include/Common/     # Shared interfaces
├── GameEngine/Include/GameLogic/  # Game simulation
├── GameEngine/Include/GameClient/ # Rendering/UI
├── Libraries/Include/rts/         # RTS-specific utilities
└── Libraries/Source/WWVegas/      # Graphics framework
```

## External Dependencies

### Required for Building
- **LLVM/Clang**: `clang`, `clang++`, `lld-link`, `llvm-rc`, and LLVM archiving tools
- **Windows SDK**: headers/libraries and `midl.exe`
- **vcpkg**: manifest dependencies, using the `x64-windows` triplet

### Platform-Specific
- **Windows x64**: DirectX 8, Miles Sound System, Bink Video
- **Registry detection**: Automatic game install path detection from EA registry keys

## Tools and Utilities

### Development Scripts (`scripts/cpp/`)
- `fixInludesCase.sh`: Fix include case sensitivity
- `refactor_*.py`: Code refactoring utilities
- `remove_trailing_whitespace.py`: Code cleanup

### Build Tools
- W3DView: 3D model viewer
- TextureCompress: Asset optimization
- MapCacheBuilder: Map preprocessing

## Key Files to Understand
- `CMakePresets.json`: Supported build configuration
- `cmake/toolchains/clang-windows-x64.cmake`: Environment-safe LLVM tool discovery
- `cmake/config-build.cmake`: Build options and feature flags
- `Core/GameEngine/Include/`: Core engine interfaces
- `**/Code/Main/WinMain.cpp`: Application entry points
- `GeneralsReplays/`: Compatibility test data
