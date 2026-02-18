# Graphics Architecture (Metal Bridge)
**Source:** `GeneralsX:old-multiplatform-attempt` (`docs/WORKDIR/support/CRITICAL_VFS_DISCOVERY.md`, `PHASE37/TEXTURE_LOADING_SUCCESS.md`)
**Time Context:** Oct - Nov 2025

## The Bridge Strategy
The project implemented a bridge between the engine's `W3D` (Westwood 3D) system and Apple's **Metal** API. Instead of a full refactor, it used "Interface Hijacking".

### 1. IRenderDevice Redirection
The `MacOSRenderer.mm` implements `IRenderDevice`. A key discovery was adding a `DrawIndexed` overload that takes `VertexBufferClass*` and `IndexBufferClass*` directly, bypassing temporary CPU copies.

### 2. Buffer Hijacking (Critical Technique)
On Windows, `DX8VertexBufferClass` wraps a `IDirect3DVertexBuffer8`. On macOS, this class was modified to hold a Metal `id<MTLBuffer>` pointer:
- **Lock/Unlock**: These methods map/unmap the Metal buffer for CPU writes.
- **Creation**: The constructor detects the platform and allocates a Metal buffer with `MTLResourceStorageModeShared`.

### 3. Texture Loading Pipeline
The most successful part of the old port was the texture pipeline.
- **VFS Flow**: The FileSystem extracts `.tga`/`.dds` from `.big`.
- **Interception**: In `texture.cpp`, the `Apply_New_Surface()` method was hooked.
- **Metal Upload**: A new class `MetalWrapper` (or `MacOSRenderer`) provided `CreateTextureFromMemory()`, which converted CPU pixel data into `id<MTLTexture>`.
- **Result**: Confirmed 10/10 textures (including menu elements like `MainMenu.wnd` assets) loaded successfully to the GPU.

### 4. Shaders
- **Uber-Shader**: A single Metal Shading Language (MSL) shader (`basic.metal`) was used to emulate the Fixed Function Pipeline of DirectX 8.
- **Features**: Supported alpha-testing, fog, and basic lighting via bitmasks.
