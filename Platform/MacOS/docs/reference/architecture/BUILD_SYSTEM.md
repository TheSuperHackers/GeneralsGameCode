# Build System Architecture

The project employs a highly modular build system based on **CMake**, designed to handle hundreds of source files and dozens of internal libraries while supporting modern toolchains (macOS/Clang, Windows/MSVC).

## Strategic Goals

1.  **Modularity**: Each major engine component (AI, Physics, Network, W3D rendering) is built as a separate static library.
2.  **Cross-Platform Portability**: Using CMake to generate platform-native projects (Project files for Xcode/MacOS, Ninja for development, MSVC for Windows).
3.  **Modern Standards**: Targeting C++20 across all platforms.
4.  **Legacy Support**: Maintaining the ability to link with legacy BINK, Miles, and SXP libraries where source code is unavailable.

## Directory Hierarchy & Libraries

The build system follows the physical directory structure:

### 1. Core Engine Libraries
- **`GameEngine`**: The heart of the simulation (Logic, Common types).
- **`GameEngineDevice`**: Platform abstraction layers (Input, Windows, FileSystem).
- **`WW3D2` (Vegas Legacy)**: The core 3D rendering engine and math libraries.

### 2. Dependencies
- Located in `Dependencies/`.
- Includes custom versions of **STLPort**, **GameSpy**, and mock headers for **DirectX 8** (on non-Windows platforms).
- External dependencies like **SDL2** and **OpenAL** are managed via system packages or git submodules.

### 3. Main Targets
- **`z_generals` (Generals Zero Hour)**: The primary game executable, linking all libraries.
- **Tools**: Numerous utility targets like `MapCacheBuilder`, `ImagePacker`, and `GUIEdit`.

## Build Process (macOS)

1.  **Generation**: `cmake . -B build` creates the build files.
2.  **Compilation**: `cmake --build build --target GeneralsXZH` compiles and links the executable.
3.  **Resources**: Assets are typically not handled by CMake but deployed via external scripts or manual copy to the application bundle.

## Key CMake Files

- **Root Entry**: `./GeneralsMD/Code/Main/CMakeLists.txt`
- **Engine Logic**: `./GeneralsMD/Code/GameEngine/CMakeLists.txt`
- **W3D Engine**: `./GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt`
- **Platform Specifics**: `./Platform/MacOS/CMakeLists.txt` (referenced by main targets)
