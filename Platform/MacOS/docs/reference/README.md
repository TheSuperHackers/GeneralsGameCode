# Reference Materials

Supplementary documentation, specifications, and research materials for the macOS port.

> **Note:** For the main working documentation, see the [docs root](../README.md).
> This directory contains **reference-only** materials: original engine analysis, DX8 specs, and implementation plans.

---

## DX8 → Metal Specifications

Detailed specifications for the DirectX 8 to Metal translation layer.

| Document | Description |
|:---|:---|
| [DX8_METAL_BACKEND.md](dx8_metal_specs/DX8_METAL_BACKEND.md) | Complete DX8→Metal backend specification (phased implementation plan) |
| [MACOS_CMAKE_INTEGRATION.md](dx8_metal_specs/MACOS_CMAKE_INTEGRATION.md) | CMake integration reference (actual build system state) |
| [documentation.pdf](dx8_metal_specs/documentation.pdf) | DirectX 8 SDK reference documentation (549 pages) |
| [dx8_spec_extracted.txt](dx8_metal_specs/dx8_spec_extracted.txt) | Extracted DX8 API specification (~500KB searchable text) |
| [Metal-Shading-Language-Specification.pdf](dx8_metal_specs/Metal-Shading-Language-Specification.pdf) | Apple Metal Shading Language Specification (12MB PDF) |
| [metal_spec_extracted.txt](dx8_metal_specs/metal_spec_extracted.txt) | Extracted Metal spec (~670KB searchable text) |

## Engine Architecture (Pre-Port Analysis)

Documentation of the **original** game engine architecture, created during initial codebase research.

| Document | Description |
|:---|:---|
| [INDEX.md](architecture/INDEX.md) | Architecture overview index |
| [CORE_ARCHITECTURE.md](architecture/CORE_ARCHITECTURE.md) | Core engine: Logic/Client separation |
| [ENGINE_MAIN_LOOP.md](architecture/ENGINE_MAIN_LOOP.md) | Main game loop lifecycle |
| [GRAPHICS_PIPELINE.md](architecture/GRAPHICS_PIPELINE.md) | Original DX8 graphics pipeline |
| [OBJECT_SYSTEM.md](architecture/OBJECT_SYSTEM.md) | Game object/thing system |
| [CONFIGURATION.md](architecture/CONFIGURATION.md) | INI/Configuration system |

## Document Responsibility Map

To avoid duplication, each topic has a **single source of truth**:

| Topic | Source of Truth | Location |
|:---|:---|:---|
| How to build & run | **SETUP.md** | `docs/SETUP.md` |
| CMake structure & targets | **BUILD_SYSTEM.md** | `docs/BUILD_SYSTEM.md` |
| CMake reference (actual code) | **MACOS_CMAKE_INTEGRATION.md** | `docs/reference/dx8_metal_specs/` |
| Metal rendering pipeline | **RENDERING.md** | `docs/RENDERING.md` |
| Architecture, gotchas, rules | **DEVELOPMENT.md** | `docs/DEVELOPMENT.md` |
| Bug history & milestones | **CHANGELOG.md** | `docs/CHANGELOG.md` |
| Stub audit | **STUBS_AUDIT.md** | `docs/STUBS_AUDIT.md` |
| DX8→Metal impl plan | **DX8_METAL_BACKEND.md** | `docs/reference/dx8_metal_specs/` |
| Original engine architecture | **architecture/*.md** | `docs/reference/architecture/` |
