# Initialization and Rendering Lifecycle (macOS/Metal)

This document describes the call chain from application launch to pixel output on screen.
**Last Updated:** 2026-02-18

## 1. Startup Flow

1.  **`MacOSMain.mm :: main()`**: Entry point. Initializes memory manager and signals. Calls `MacOS_Main`.
2.  **`MacOSWindowManager.mm :: MacOS_Main()`**: 
    *   Configures `NSApplication` (Cocoa).
    *   Creates window (`MacOS_CreateWindow`).
    *   Initializes Metal renderer (`MacOS_InitRenderer` -> `MacOSRenderDevice::Initialize`).
    *   Calls **`GameMain()`**.
3.  **`GameMain.cpp :: GameMain()`**: Creates global objects `TheGameEngine` and `TheFramePacer`. Calls `TheGameEngine->init()`.
4.  **`GameEngine.cpp :: GameEngine::init()`**: Initializes all subsystems (File System, INI, Audio, AI).
    *   Creates `TheGameClient` (on macOS, this is a `MacOSGameClient` object).
    *   `MacOSGameClient` creates `TheDisplay` (a `MacOSDisplay` object).

## 2. Main Loop

The heart of the game is in **`GameEngine::execute()`** (`GameEngine.cpp`):

```cpp
while (!m_quitting) {
    MacOS_PumpEvents();    // Mouse/Keyboard event handling
    update();              // Update logic and TRIGGER RENDERING HIERARCHY
    TheFramePacer->update();
}
```

## 3. Rendering Hierarchy

Rendering in Generals is not a single call, but a chain of nested systems:

### Level 1: Engine Update (`GameEngine::update`)
Calls `TheGameClient->UPDATE()`.

### Level 2: Client Update (`GameClient::update`)
Depending on state (menu or in-game), calls:
*   `TheDisplay->UPDATE()`
*   `TheDisplay->DRAW()` (Where W3D magic begins).

### Level 3: W3D / Metal Lifecycle (`MacOSDisplay::draw`)
This method in `MacOSDisplay.mm` manages frame states:
1.  **`WW3D::Begin_Render()`** -> `MetalDevice8::BeginScene()`
2.  **`Display::draw()`** -> Renders 3D views (TacticalView).
3.  **`TheWindowManager->winRepaint()`** -> Renders 2D interface (SAGE UI).
4.  **`WW3D::End_Render()`** -> `MetalDevice8::EndScene()`

## 4. Metal Renderer (MetalDevice8.mm)

### 4.1. Frame Lifecycle

*   **`BeginScene()`**:
    *   Checks `m_InScene`.
    *   Creates `MTLCommandBuffer` from `m_CommandQueue`.
    *   Acquires `CAMetalDrawable` from `CAMetalLayer`.
*   **`Clear(count, rects, flags, color, z, stencil)`**:
    *   Ends current encoder (if any).
    *   Creates `MTLRenderPassDescriptor`:
      - `D3DCLEAR_TARGET` → `MTLLoadActionClear` + `clearColor`.
      - Without `D3DCLEAR_TARGET` → `MTLLoadActionLoad`.
    *   Creates new `MTLRenderCommandEncoder`.
    *   Sets `MTLViewport` from `m_Viewport`.
*   **`EndScene()`**: Resets `m_InScene`.
*   **`Present()`**:
    *   `endEncoding` on current encoder.
    *   `presentDrawable` + `commit` command buffer.
    *   Releases encoder, drawable, command buffer.

### 4.2. Pipeline State Objects (PSO)

**`GetPSO(DWORD fvf)`** — creates or retrieves from cache (`m_PsoCache`):

1.  **Vertex Function**: `vertex_main` (from `MacOSShaders.metal`).
2.  **Fragment Function**: `fragment_main`.
3.  **Color Attachment**: `MTLPixelFormatBGRA8Unorm`, blending = Standard Alpha (hardcoded).
4.  **Vertex Descriptor** — built based on FVF:
    - `D3DFVF_XYZ` → attr[0] `float3` (12B)
    - `D3DFVF_XYZRHW` → attr[0] `float4` (16B)
    - `D3DFVF_NORMAL` → skip 12B (not in shader)
    - `D3DFVF_DIFFUSE` → attr[1] `uchar4norm` (4B)
    - `D3DFVF_SPECULAR` → skip 4B
    - `D3DFVF_TEX1` → attr[2] `float2` (8B)
5.  Cached by `fvf` key: `std::map<uint32_t, void*>`.

**⚠️ Known PSO Gaps:**
- Blending is always Standard Alpha (no PSO re-creation based on `D3DRS_SRCBLEND`/`DESTBLEND`).
- No Depth/Stencil state object.
- No CullMode binding.
- Does not handle `D3DFVF_TEX2`, `D3DFVF_XYZBn` (bone weights).

### 4.3. Draw Calls

**`DrawPrimitive(pt, sv, pc)`** and **`DrawIndexedPrimitive(pt, mi, nv, si, pc)`**:

1.  Gets FVF from VB via `GetBufferFVF(m_StreamSource)`.
2.  Gets/Creates PSO via `GetPSO(fvf)`.
3.  Sets PSO in encoder.
4.  Binds Vertex Buffer: `setVertexBuffer:offset:atIndex:0`.
5.  Fills **`MetalUniforms`** and sends via `setVertexBytes` / `setFragmentBytes` to buffer index 1:
    - `world`, `view`, `projection` — from `m_Transforms[]`.
    - `screenSize` — window width/height.
    - `useProjection` — 1 (3D) or 2 (Screen Space, XYZRHW).
    - `shaderSettings` — bit field (texturing on/off).
6.  Binds texture stage 0 (if present): `setFragmentTexture:atIndex:0`.
7.  Primitive mapping:
    - `D3DPT_TRIANGLELIST` → `MTLPrimitiveTypeTriangle`, `vertexCount = pc * 3`
    - `D3DPT_TRIANGLESTRIP` → `MTLPrimitiveTypeTriangleStrip`, `vertexCount = pc + 2`
    - `D3DPT_LINELIST` → `MTLPrimitiveTypeLine`, `vertexCount = pc * 2`
8.  `drawPrimitives` or `drawIndexedPrimitives`.

**DrawPrimitiveUP / DrawIndexedPrimitiveUP** — ❌ not implemented (stubs, return `D3D_OK`).

### 4.4. Resource Creation

| Resource | Method | Allocation |
|:---|:---|:---|
| `MetalTexture8` | `CreateTexture` | `W3DNEW MetalTexture8(this, w, h, l, u, f, p)` |
| `MetalVertexBuffer8` | `CreateVertexBuffer` | `W3DNEW MetalVertexBuffer8(FVF, count, vertexSize)` |
| `MetalIndexBuffer8` | `CreateIndexBuffer` | `W3DNEW MetalIndexBuffer8(count, is32bit)` |
| `Surface` | `CreateImageSurface` | ❌ Stub (`E_NOTIMPL`) |

### 4.5. Texture Lifecycle

1. Game calls `CreateTexture(w, h, levels, usage, format, pool)` → `MetalTexture8` creates `MTLTexture`.
2. `LockRect(level)` → allocates staging buffer (`malloc`), returns ptr + pitch.
3. Game writes pixels to staging buffer.
4. `UnlockRect(level)` → `[mtlTexture replaceRegion:withBytes:bytesPerRow:]`, `free()` staging.
5. `SetTexture(0, tex)` → stores in `m_Textures[0]`.
6. At draw call: `setFragmentTexture:mtlTex atIndex:0`.

D3DX texture loading (`D3DXCreateTextureFromFileExA`):
- Calls `GetRenderDevice()->CreateTextureFromFile()` → `MacOSTexture` (`ITexture`).
- Wraps result in `MetalTexture8(device, mtlTexture, format)`.

**Formats (`MetalFormatFromD3D`):**
- `ARGB8/XRGB8` → `MTLPixelFormatBGRA8Unorm`
- `DXT1` → `MTLPixelFormatBC1_RGBA`
- `DXT3` → `MTLPixelFormatBC2_RGBA`
- `DXT5` → `MTLPixelFormatBC3_RGBA`
- 16-bit (`R5G6B5`, `A1R5G5B5`, etc.) → fallback `BGRA8` (⚠️ no content conversion!)

## 5. Shaders (MacOSShaders.metal)

**Vertex Shader (`vertex_main`):**
- Inputs: `position` (attr 0), `color` (attr 1), `texCoord` (attr 2).
- Uniforms: `world`, `view`, `projection`, `screenSize`, `useProjection`, `shaderSettings`.
- If `useProjection == 1`: `pos = projection * view * world * pos` (3D).
- If `useProjection == 2`: screen space → NDC (`pos / screenSize * 2 - 1`).
- Fog factor: dummy `clamp(-viewPos.z / 500.0, 0, 1)`.

**Fragment Shader (`fragment_main`):**
- Sampled texture if `SHIFT_TEXTURING` bit set.
- `finalColor = texColor * in.color`.
- Alpha test discard if `SHIFT_ALPHATEST` bit set and alpha < 0.375.
- Primary gradient: `MODULATE` (default) or `ADD`.
- Fog: simple mix with `fogColor(0.5, 0.5, 0.5)`.

**⚠️ Vertex Mismatch Issue:**
- For `D3DFVF_XYZRHW`, vertex descriptor uses `MTLVertexFormatFloat4` for attr[0],
  but shader declares `VertexIn.position` as `float3`. This may cause issues.
  Needs a separate vertex function or `packed_float4` for `XYZRHW`.

## 6. Cheatsheet: Not Implemented

| Functionality | Plan Phase | Status |
|:---|:---|:---|
| Depth/Stencil texture + state | Phase 10 | ❌ |
| Dynamic blending (per-RS) | Phase 6 | ❌ |
| CullMode binding | Phase 6 | ❌ |
| Multi-texturing (stage 1+) | Phase 7 | ❌ |
| TSS formulas in shader | Phase 7 | ❌ |
| Sampler states | Phase 7 | ❌ |
| Per-vertex lighting | Phase 8 | ❌ |
| Real fog params | Phase 9 | ❌ |
| Render targets | Phase 10 | ❌ |
| DrawPrimitiveUP | Phase 3 | ❌ |
| Surface (`GetSurfaceLevel`) | Phase 5 | ❌ |
| 16-bit format conversion | Phase 5 | ❌ |
| `TriangleFan` → `TriangleList` | Phase 3 | ❌ |
| `MTLDevice` shared (`VB/IB`) | Phase 2 | ⚠️ Uses `MTLCreateSystemDefaultDevice()` |
