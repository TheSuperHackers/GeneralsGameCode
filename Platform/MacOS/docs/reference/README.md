# Reference Materials

Supplementary documentation, specifications, and research materials for the macOS port.

---

## DX8 → Metal Specifications

Detailed specifications for the DirectX 8 to Metal translation layer.

| Document | Description |
|:---|:---|
| [DX8_METAL_BACKEND.md](dx8_metal_specs/DX8_METAL_BACKEND.md) | Complete DX8→Metal backend specification |
| [MACOS_CMAKE_INTEGRATION.md](dx8_metal_specs/MACOS_CMAKE_INTEGRATION.md) | CMake integration details for macOS |
| [documentation.pdf](dx8_metal_specs/documentation.pdf) | DirectX 8 SDK reference documentation |
| [dx8_spec_extracted.txt](dx8_metal_specs/dx8_spec_extracted.txt) | Extracted DX8 API specification (~500KB) |

## Engine Architecture

Documentation of the original game engine architecture (pre-port analysis).

| Document | Description |
|:---|:---|
| [INDEX.md](architecture/INDEX.md) | Architecture overview index |
| [CORE_ARCHITECTURE.md](architecture/CORE_ARCHITECTURE.md) | Core engine components and subsystems |
| [ENGINE_MAIN_LOOP.md](architecture/ENGINE_MAIN_LOOP.md) | Main game loop lifecycle |
| [GRAPHICS_PIPELINE.md](architecture/GRAPHICS_PIPELINE.md) | Original graphics pipeline (DX8) |
| [OBJECT_SYSTEM.md](architecture/OBJECT_SYSTEM.md) | Game object/thing system |
| [BUILD_SYSTEM.md](architecture/BUILD_SYSTEM.md) | Original build system analysis |
| [CONFIGURATION.md](architecture/CONFIGURATION.md) | INI/Configuration system |

## Rendering Analysis

Detailed rendering call chain analysis created during debugging.

| Document | Description |
|:---|:---|
| [RENDERING_FLOW.md](RENDERING_FLOW.md) | Full rendering flow with mermaid diagrams — from `main()` to Metal API calls |
| [RENDERING_LIFECYCLE.md](RENDERING_LIFECYCLE.md) | Frame lifecycle, Metal PSO creation, shader details, known gaps |

## Porting Status (Detailed)

| Document | Description |
|:---|:---|
| [PORTING_STATUS.md](PORTING_STATUS.md) | Full porting status with build flow diagram, stub tables, backlog |


