# Research Summary: Heritage of the macOS Port
**Date:** 2026-02-14
**Source:** Historical branches from `GeneralsX` (specifically `old-multiplatform-attempt` and `feature/dxvk-native-migration`)

## Executive Overview
Our research into the `GeneralsX` repository has uncovered a wealth of technical documentation from the "Metal Era" (Oct 2025 - Jan 2026). This port reached a high level of maturity, successfully rendering UI and 3D meshes using a native Metal bridge before the project pivoted towards a Vulkan/MoltenVK-based "DXVK Native" approach.

## Key Reference Files
1. **[Runtime_Commands.md](./Runtime_Commands.md)**: How to launch the game, environment variables, and startup switches.
2. **[Graphics_Architecture.md](./Graphics_Architecture.md)**: Details on the Metal bridge, texture loading pipeline, and the "Buffer Hijacking" technique.
3. **[Input_and_UI_Patterns.md](./Input_and_UI_Patterns.md)**: Insights into NSEvent routing and coordinate transformation.
4. **[Known_Issues_and_Bugs.md](./Known_Issues_and_Bugs.md)**: Documented limitations like the 1024px+ texture artifact and BC3 decompression.

## Core Breakthroughs Found
- **Direct Texture Upload**: Confirmed that `TextureCache` can load textures directly from `.big` archives into Metal textures (`id<MTLTexture>`).
- **Buffer Redirection**: The legacy `DX8VertexBufferClass` was modified to store Metal buffers on macOS, allowing the original `W3D` code to "think" it's talking to DirectX.
- **VFS Integration**: The Virtual File System (VFS) handles case-insensitivity and archive extraction correctly on macOS, provided the data path is mapped.

## Strategic Recommendation
For our current `Phase 6: UI Rendering`, we should implement `drawFillRect` by mimicking the `DrawIndexed` redirection found in the `old-multiplatform-attempt` branch. This avoids rewriting the high-level UI logic and focuses on the low-level graphics bridge.
