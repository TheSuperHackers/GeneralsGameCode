# Generals Architecture: Pre-Port Analysis

> This directory contains architectural overviews of the **original** Generals engine,
> created during initial codebase research before the macOS port.
>
> For the **current** build system docs, see [`docs/BUILD_SYSTEM.md`](../../BUILD_SYSTEM.md).
> For the **current** rendering pipeline docs, see [`docs/RENDERING.md`](../../RENDERING.md).

## 🗺️ Subsystem Index

1.  **[Core Architecture: Logic/Client Separation](CORE_ARCHITECTURE.md)**
    The fundamental "State vs Presentation" split that drives the engine.
2.  **[Engine Initialization and Main Loop](ENGINE_MAIN_LOOP.md)**
    How the game starts and pulses (the heart of the process).
3.  **[Graphics Rendering Pipeline](GRAPHICS_PIPELINE.md)**
    The original DX8 rendering pipeline and W3D draw call chain.
4.  **[Object System Architecture](OBJECT_SYSTEM.md)**
    Composition-based units and behavioral modules.
5.  **[Configuration and Data Management](CONFIGURATION.md)**
    INI files, BIG archives, and data-driven design.

---

## 📂 Repository Structure

```text
/
├── Core/               # Shared engine code (Math, WWVegas, Tools)
├── Dependencies/       # 3rd party libs (GameSpy, STLPort, DX8 stubs)
├── GeneralsMD/         # Zero Hour specific code (Main, GameLogic)
├── Platform/           # Platform-specific folders (MacOS)
│   └── MacOS/          # Entry points, Metal renderer, Window management
├── resources/          # Build resources
├── cmake/              # CMake modules
└── scripts/            # Build utilities
```

## 🛠️ Key Philosophy

- **Modernization without Mutilation**: Keep the core legacy SAGE logic but replace the platform layer (DirectX 8 → Metal, Win32 → Cocoa).
- **Logic Determinism**: Protect the fixed 30 FPS update loop at all costs.
- **Data-Driven**: Prefer changing an INI property over changing code.
