# Generals Architecture: Source of Truth

This directory contains verified architectural overviews of the Generals engine, modernized for cross-platform development (macOS/Linux).

## 🗺️ Subsystem Index

1.  **[Core Architecture: Logic/Client Separation](CORE_ARCHITECTURE.md)**
    The fundamental "State vs Presentation" split that drives the engine.
2.  **[Engine Initialization and Main Loop](ENGINE_MAIN_LOOP.md)**
    How the game starts and pulses (the heart of the process).
3.  **[Graphics Rendering Pipeline](GRAPHICS_PIPELINE.md)**
    From W3D draw calls to Metal/Vulkan backends.
4.  **[Object System Architecture](OBJECT_SYSTEM.md)**
    Composition-based units and behavioral modules.
5.  **[Configuration and Data Management](CONFIGURATION.md)**
    INI files, BIG archives, and data-driven design.
6.  **[Build System Architecture](BUILD_SYSTEM.md)**
    CMake structure and modular library management.

---

## 📂 Repository Structure (Verified)

```text
/
├── Core/               # Shared engine code (Math, WWVegas, Tools)
├── Dependencies/       # 3rd party libs (GameSpy, STLPort, Mock DX8)
├── GeneralsMD/         # Zero Hour specific code (Main, GameLogic)
├── Platform/           # Platform-specific folders (MacOS, Win32)
│   └── MacOS/          # Entry points, Metal renderer, Window management
├── GameResources/      # Shaders, Windows, UI layouts
├── scripts/            # Build and deployment utilities
```

## 🛠️ Key Philosophy

- **Modernization without Mutilation**: Keep the core legacy SAGE logic (which works perfectly for RTS) but replace the platform layer (DirectX 8 -> Metal/Vulkan, Win32 -> SDL2).
- **Logic Determinism**: Protect the fixed 30 FPS update loop at all costs.
- **Data-Driven**: Prefer changing an INI property over changing code.
