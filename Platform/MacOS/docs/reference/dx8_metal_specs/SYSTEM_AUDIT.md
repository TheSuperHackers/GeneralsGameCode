# DX8 → Metal System Audit

> **Date:** 2026-02-21
> **Based on:** `dx8_spec_extracted.txt` (official DX8.1 spec) + `metal_spec_extracted.txt` (Metal Shading Language spec)
> **Scope:** All Metal porting layer files

---

## Legend

| Symbol | Meaning |
|:---:|:---|
| ✅ | Implemented & matches DX8 spec |
| ⚠️ | Implemented but has spec deviations |
| ❌ | Stubbed / Not implemented |
| 🔧 | Partially implemented, needs work |

---

## 1. MetalInterface8 (`MetalInterface8.mm` — 184 lines)

### IDirect3D8 Methods

| Method | Status | Notes |
|:---|:---:|:---|
| `QueryInterface` | ✅ | Returns E_NOINTERFACE (acceptable) |
| `AddRef/Release` | ✅ | Correct ref counting |
| `RegisterSoftwareDevice` | ❌ | Returns E_NOTIMPL — spec says OK for HAL devices |
| `GetAdapterCount` | ⚠️ | Hardcoded to 1 — OK for macOS single-GPU |
| `GetAdapterIdentifier` | ⚠️ | Returns "Apple Metal GPU" — should query actual device name |
| `GetAdapterModeCount` | ⚠️ | **Returns 1** — spec expects enumeration of all valid display modes. The game may try to enumerate modes for resolution selection |
| `EnumAdapterModes` | ⚠️ | **Hardcoded 800x600@60Hz** — should query actual display modes from CGDisplayCopyAllDisplayModes |
| `GetAdapterDisplayMode` | ⚠️ | **Hardcoded 800x600@60Hz** — should query actual current display mode |
| `CheckDeviceType` | ⚠️ | Always returns D3D_OK — OK as passthrough but no validation |
| `CheckDeviceFormat` | ⚠️ | Always returns D3D_OK — should reject unsupported formats (e.g. 16-bit on macOS) |
| `CheckDeviceMultiSampleType` | ⚠️ | Always D3D_OK — macOS Metal does support MSAA but should validate sample counts |
| `CheckDepthStencilMatch` | ⚠️ | Always D3D_OK — OK for now |
| `GetDeviceCaps` | 🔧 | See detailed caps audit below |
| `GetAdapterMonitor` | ⚠️ | Returns nullptr — should return monitor handle for multi-monitor |
| `CreateDevice` | ✅ | Creates MetalDevice8 properly |

### D3DCAPS8 Audit (GetDeviceCaps)

Per DX8 spec §D3DCAPS8 structure:

| Cap Field | Set Value | Spec Expectation | Status |
|:---|:---|:---|:---:|
| `DeviceType` | `D3DDEVTYPE_HAL` | Correct | ✅ |
| `DevCaps` | `HWTRANSFORMANDLIGHT` | Missing: `DRAWPRIMTLVERTEX`, `HWRASTERIZATION` — game may check these | ⚠️ |
| `MaxSimultaneousTextures` | 8 | Spec: actual GPU max. 8 is fine for Metal | ✅ |
| `MaxTextureBlendStages` | 8 | OK | ✅ |
| `VertexShaderVersion` | 0x0101 | VS 1.1 — OK but our shaders don't actually process DX8 VS bytecode | ⚠️ |
| `PixelShaderVersion` | 0x0101 | PS 1.1 — OK but same caveat | ⚠️ |
| `MaxPrimitiveCount` | 0xFFFFFF | OK | ✅ |
| `MaxVertexIndex` | 0xFFFFFF | OK | ✅ |
| `MaxStreams` | 8 | OK | ✅ |
| `MaxActiveLights` | 4 | Matches shader (4 lights array) | ✅ |
| `MaxTextureWidth/Height` | 4096 | Metal supports 16384+ on Apple Silicon, but 4096 is safe | ✅ |
| `RasterCaps` | `FOGRANGE \| 0x100 \| 0x200 \| ZBIAS` | **0x100 = FOGTABLE, 0x200 = FOGVERTEX** — should use named constants for clarity. Missing: `ZTEST`, `DITHER`, `MIPMAPLODBIAS` | ⚠️ |
| `TextureCaps` | `0x01 \| 0x02 \| 0x04` | **Magic numbers!** Should be: `ALPHA \| PERSPECTIVE \| POW2`. Missing: `MIPMAP`, `CUBEMAP`, `PROJECTED` | ⚠️ |
| `TextureOpCaps` | Named flags | Missing: `MODULATE4X`, `SUBTRACT`, `ADDSIGNED`, `DOTPRODUCT3` — we implement these in the shader but don't advertise them | ⚠️ |
| `SrcBlendCaps` | 0x1FFF | All blend caps — OK | ✅ |
| `DestBlendCaps` | 0x1FFF | All blend caps — OK | ✅ |
| `TextureFilterCaps` | 0 | **Missing!** Spec: should report MAGFPOINT, MAGFLINEAR, MINFPOINT, MINFLINEAR etc. Game may check these | ❌ |
| `TextureAddressCaps` | 0 | **Missing!** Spec: should report WRAP, CLAMP, MIRROR etc. | ❌ |
| `ShadeCaps` | 0 | **Missing!** Spec: should report COLORGOURAUDRGB, ALPHAGOURAUDBLEND, SPECULARGOURAUDRGB | ❌ |
| `PrimitiveMiscCaps` | `COLORWRITEENABLE` | Missing: `CULLCW`, `CULLCCW`, `CULLNONE`, `MASKZ`, `BLENDOP` | ⚠️ |
| `MaxTextureRepeat` | 0 | **Missing!** Should be >= 1. Game may use for tiled terrain textures | ❌ |
| `MaxAnisotropy` | 0 | **Missing!** Metal supports up to 16 | ❌ |
| `MaxVertexBlendMatrices` | 0 | OK — game uses CPU skinning | ✅ |
| `MaxPointSize` | 0 | **Missing!** Spec says >= 1.0 if point primitives supported | ❌ |

---

## 2. MetalDevice8 (`MetalDevice8.mm` — 2476 lines)

### 2.1 Lifecycle & Frame Management

| Method | Status | Notes |
|:---|:---:|:---|
| `InitMetal` | ✅ | Creates device, queue, layer, depth texture, default surfaces |
| `BeginScene` | ✅ | Creates command buffer, acquires drawable |
| `EndScene` | ⚠️ | **Only sets m_InScene=false** — DX8 spec says rendering between Begin/End. We create encoder in `Clear()` which is called before BeginScene — this works but is non-standard |
| `Present` | ✅ | Ends encoder, presents drawable, commits and waits for completion |
| `Clear` | ✅ | Handles D3DCLEAR_TARGET, ZBUFFER, STENCIL correctly |
| `TestCooperativeLevel` | ✅ | Returns D3D_OK (always cooperative on macOS) |
| `Reset` | ⚠️ | **Empty stub** — spec says recreates device resources. May cause issues on window resize |
| `GetAvailableTextureMem` | ⚠️ | Returns fixed 512MB — should query Metal device |

### 2.2 Resource Creation

| Method | Status | Notes |
|:---|:---:|:---|
| `CreateTexture` | ✅ | Creates MetalTexture8 properly |
| `CreateVolumeTexture` | ❌ | Returns E_NOTIMPL — game may not use |
| `CreateCubeTexture` | ❌ | Returns E_NOTIMPL — game uses for environment maps! |
| `CreateVertexBuffer` | ⚠️ | Casts count to `unsigned short` — **potential overflow** for large meshes (>65535 vertices). DX8 spec allows UINT32 vertex counts |
| `CreateIndexBuffer` | ✅ | Handles 16-bit and 32-bit correctly |
| `CreateImageSurface` | ✅ | Creates surface correctly |
| `CreateRenderTarget` | ❌ | **Not found** — needed for shadow maps |
| `CreateDepthStencilSurface` | ❌ | **Not found** — needed for custom depth surfaces |

### 2.3 Render States

| Render State | Status | Notes |
|:---|:---:|:---|
| `D3DRS_ZENABLE` | ✅ | Mapped to MTLDepthStencilDescriptor |
| `D3DRS_ZWRITEENABLE` | ✅ | Correctly toggles depth writes |
| `D3DRS_ZFUNC` | ✅ | All D3DCMP values mapped correctly |
| `D3DRS_ALPHABLENDENABLE` | ✅ | Baked into PSO key |
| `D3DRS_SRCBLEND` | ✅ | All D3DBLEND values mapped correctly |
| `D3DRS_DESTBLEND` | ✅ | All D3DBLEND values mapped correctly |
| `D3DRS_BLENDOP` | ❌ | **Not handled!** Spec defines ADD, SUBTRACT, REVSUBTRACT, MIN, MAX. We hardcode `MTLBlendOperationAdd`. If game sets D3DBLENDOP_REVSUBTRACT, blending will be wrong |
| `D3DRS_CULLMODE` | ✅ | Mapped with correct DX8↔Metal winding flip |
| `D3DRS_FILLMODE` | ❌ | **Not handled!** D3DFILL_WIREFRAME should map to `setTriangleFillMode:MTLTriangleFillModeLines` |
| `D3DRS_LIGHTING` | ✅ | Passed to shader as `lightingEnabled` |
| `D3DRS_AMBIENT` | ✅ | Passed as globalAmbient |
| `D3DRS_DIFFUSEMATERIALSOURCE` | ✅ | Passed to shader |
| `D3DRS_AMBIENTMATERIALSOURCE` | ✅ | Passed to shader |
| `D3DRS_SPECULARMATERIALSOURCE` | ✅ | Passed to shader |
| `D3DRS_EMISSIVEMATERIALSOURCE` | ✅ | Passed to shader |
| `D3DRS_SPECULARENABLE` | ⚠️ | Default set to FALSE. **Shader always adds specular unconditionally** (line 575 MacOSShaders.metal). Per DX8 spec, specular should only be added when D3DRS_SPECULARENABLE=TRUE |
| `D3DRS_ALPHATESTENABLE` | ✅ | Passed to fragment shader |
| `D3DRS_ALPHAFUNC` | ✅ | All D3DCMP values implemented |
| `D3DRS_ALPHAREF` | ✅ | Normalized to 0..1 correctly |
| `D3DRS_FOGENABLE` | ✅ | Controls fog computation |
| `D3DRS_FOGCOLOR` | ✅ | Converted from ARGB DWORD correctly |
| `D3DRS_FOGTABLEMODE` | ✅ | Mapped to fog formulas |
| `D3DRS_FOGVERTEXMODE` | ✅ | Fallback if table mode is NONE |
| `D3DRS_FOGSTART/END/DENSITY` | ✅ | Bit-cast from DWORD correctly |
| `D3DRS_STENCILENABLE` | ✅ | Full stencil state implemented |
| `D3DRS_STENCILFUNC/REF/MASK/WRITEMASK` | ✅ | Correctly mapped |
| `D3DRS_STENCILFAIL/ZFAIL/PASS` | ✅ | All stencil ops mapped |
| `D3DRS_COLORWRITEENABLE` | ✅ | Mapped to MTLColorWriteMask |
| `D3DRS_TEXTUREFACTOR` | ✅ | Converted from ARGB to float4 |
| `D3DRS_WRAP0..7` | ❌ | Not handled — controls UV wrapping at vertex level |
| `D3DRS_POINTSIZE` | ❌ | Not handled |
| `D3DRS_POINTSIZE_MIN/MAX` | ❌ | Not handled |
| `D3DRS_POINTSPRITEENABLE` | ❌ | Not handled — game may use for particle effects |
| `D3DRS_POINTSCALEENABLE` | ❌ | Not handled |
| `D3DRS_MULTISAMPLEANTIALIAS` | ❌ | Not handled |
| `D3DRS_NORMALIZENORMALS` | ⚠️ | Not passed to shader. Shader always normalizes (line 245) which is OK but not spec-correct for un-normalized meshes |
| `D3DRS_DITHERENABLE` | ❌ | Not handled — typically no-op on modern HW |
| `D3DRS_SHADEMODE` | ❌ | D3DSHADE_FLAT not supported — would need `[[flat]]` interpolation in Metal |
| `D3DRS_LASTPIXEL` | ❌ | Not handled — affects line drawing |
| `D3DRS_CLIPPING` | ❌ | Not handled — Metal always clips |
| `D3DRS_ZBIAS` | ❌ | Not handled — should map to `setDepthBias:slopeScale:clamp:` |
| `D3DRS_RANGEFOGENABLE` | ❌ | Not handled — shader uses distance-based fog (which IS range fog), but doesn't check the RS |
| `D3DRS_LOCALVIEWER` | ❌ | Shader uses non-local viewer (V=(0,0,1)) which is DX8 default — OK but not controllable |

### 2.4 Drawing

| Method | Status | Notes |
|:---|:---:|:---|
| `DrawPrimitive` | ✅ | TRIANGLELIST, TRIANGLESTRIP, LINELIST handled |
| `DrawIndexedPrimitive` | ✅ | TRIANGLELIST, TRIANGLESTRIP handled with baseVertex |
| `DrawPrimitiveUP` | ✅ | User pointer draw with inline data or temp buffer |
| `DrawIndexedPrimitiveUP` | ❌ | **Empty stub** — returns D3D_OK silently |
| D3DPT_TRIANGLEFAN | ❌ | **Not handled in any draw call!** Metal doesn't support triangle fans natively. Spec says it should be converted to triangle list. DX8 spec explicitly lists TRIANGLEFAN as a primitive type. If the game uses it, geometry will be missing |
| D3DPT_POINTLIST | ✅ | Only in DrawPrimitiveUP |
| D3DPT_LINESTRIP | ✅ | Only in DrawPrimitiveUP |

### 2.5 Transforms

| Method | Status | Notes |
|:---|:---:|:---|
| `SetTransform` | ✅ | Stores matrices for indices 0-259 |
| `GetTransform` | ✅ | Retrieves stored matrices |
| `D3DTS_WORLD` (256) | ✅ | Passed to vertex shader |
| `D3DTS_VIEW` (2) | ✅ | Passed to vertex shader |
| `D3DTS_PROJECTION` (3) | ✅ | Passed to vertex shader |
| `D3DTS_TEXTURE0..7` (16-23) | ❌ | **Not passed to shader!** Texture coordinate transforms are ignored. Some effects use `D3DTSS_TEXTURETRANSFORMFLAGS` with texture matrix |
| `D3DTS_WORLD1..3` (257-259) | ❌ | Not used for vertex blending — OK if game doesn't use |
| `MultiplyTransform` | ❌ | **Not found** — DX8 spec lists this as a method |

### 2.6 Textures & Samplers

| Method | Status | Notes |
|:---|:---:|:---|
| `SetTexture` | ✅ | Stores texture pointer, binds in draw calls |
| `GetTexture` | ✅ | Returns stored pointer with AddRef |
| `SetTextureStageState` | ✅ | Stores in m_TextureStageStates[stage][type] |
| `GetTextureStageState` | ✅ | Retrieves from cache |
| TSS stages supported | ⚠️ | **Only 2 stages** (stages[2]). DX8 spec supports 8. Generals mostly uses 2, but some effects may use more |
| `D3DTSS_COLOROP/ARG1/ARG2` | ✅ | Passed to fragment shader |
| `D3DTSS_ALPHAOP/ARG1/ARG2` | ✅ | Passed to fragment shader |
| `D3DTSS_ADDRESSU/V` | ✅ | Mapped to MTLSamplerAddressMode |
| `D3DTSS_MAGFILTER` | ✅ | Mapped correctly |
| `D3DTSS_MINFILTER` | ✅ | Mapped correctly |
| `D3DTSS_MIPFILTER` | ✅ | Mapped correctly |
| `D3DTSS_TEXCOORDINDEX` | ❌ | **Not handled!** Specifies which UV set to use for a texture stage. Default is stage-index match, but game can redirect |
| `D3DTSS_TEXTURETRANSFORMFLAGS` | ❌ | **Not handled!** Controls texture coordinate generation modes |
| `D3DTSS_BUMPENVMAT00/01/10/11` | ❌ | Not handled — bump mapping matrices |
| `D3DTSS_BORDERCOLOR` | ❌ | Not passed to sampler |
| `D3DTSS_MAXMIPLEVEL` | ❌ | Not handled |
| `D3DTSS_MAXANISOTROPY` | ❌ | Not handled — should set sampler maxAnisotropy |
| `D3DTSS_RESULTARG` | ❌ | Not handled — DX8 extension for temp register |

### 2.7 Lighting

| Method | Status | Notes |
|:---|:---:|:---|
| `SetLight` | ✅ | Stores D3DLIGHT8 data |
| `GetLight` | ✅ | Returns stored data |
| `LightEnable` | ✅ | Toggles light in uniform |
| `GetLightEnable` | ✅ | Returns stored flag |
| `SetMaterial` | ✅ | Stores D3DMATERIAL8 |
| `GetMaterial` | ✅ | Returns stored material |
| Max lights | ⚠️ | **4 lights hardcoded**. DX8 spec allows more, game may use up to 8 |
| Light types | ✅ | DIRECTIONAL, POINT, SPOT all implemented |
| Attenuation formula | ✅ | Matches DX8 spec: 1/(a0 + a1*d + a2*d²) |
| Spotlight formula | ✅ | Inner/outer cone with falloff power |

### 2.8 Other Methods

| Method | Status | Notes |
|:---|:---:|:---|
| `SetViewport` | ✅ | Applied to encoder |
| `GetViewport` | ✅ | Returns stored viewport |
| `SetRenderTarget` | ❌ | **Empty stub** — critical for render-to-texture (shadows, mirrors) |
| `GetRenderTarget` | ✅ | Returns default RT surface |
| `GetDepthStencilSurface` | ✅ | Returns default depth surface |
| `SetGammaRamp` | ❌ | Stub — no gamma correction |
| `GetGammaRamp` | ❌ | Stub |
| `ShowCursor` | ❌ | Stub (uses NSCursor) |
| `CopyRects` | ❌ | **Stub** — needed for surface-to-surface copies (screenshots, render targets) |
| `UpdateTexture` | ❌ | **Stub** — needed for managed texture updates |
| `GetFrontBuffer` | ❌ | Stub — screenshots |
| `SetClipPlane` | ❌ | Stub |
| `ValidateDevice` | ✅ | Returns 1 pass — OK |
| `CreateVertexShader` | ⚠️ | Returns handle=0 — DX8 vertex shaders not supported. OK if game only uses FVF pipeline |
| `SetVertexShader` | ⚠️ | Stores FVF value — DX8 uses SetVertexShader for BOTH shader handles AND FVF codes. The implementation treats it as FVF only, which is correct for Generals |
| `CreatePixelShader` | ⚠️ | Returns handle=0 — DX8 pixel shaders not supported |
| `SetVertexShaderConstant` | ❌ | Stub |
| `SetPixelShaderConstant` | ❌ | Stub |
| `GetDirect3D` | ⚠️ | Returns nullptr — should return the IDirect3D8 that created this device |
| `GetCreationParameters` | ❌ | **Not found** — DX8 spec method |
| `GetInfo` | ❌ | Not found |
| `ApplyStateBlock` / `CreateStateBlock` | ❌ | Not found — DX8 state blocks for state snapshots |
| `GetClipStatus` / `SetClipStatus` | ❌ | Not found |
| `ProcessVertices` | ❌ | Not found — software vertex processing |
| `GetRasterStatus` | ❌ | Not found |
| `SetPaletteEntries` / `GetPaletteEntries` | ❌ | Not found — needed for P8 format textures |
| `SetCurrentTexturePalette` | ❌ | Not found |

### 2.9 PSO Cache & State Management

| Aspect | Status | Notes |
|:---|:---:|:---|
| PSO key includes FVF | ✅ | 20-bit FVF in key |
| PSO key includes blend state | ✅ | blendEn + src/dst blend + color write |
| PSO key includes depth state | ❌ | **Depth state is separate** — this is correct! Metal separates PSO from DSS |
| Sampler state cache | ✅ | Key from address + filter modes |
| Depth stencil state cache | ✅ | Key from Z enable + Z write + Z func + stencil states |
| Separate alpha blend factors | ❌ | **Uses same as RGB** — DX8 doesn't have separate alpha blend (that's DX9), so this is correct |
| PSO invalidation on blend change | ⚠️ | **New PSO for each unique blend state** — can cause PSO explosion. Should be manageable |

---

## 3. MetalTexture8 (`MetalTexture8.mm` — 395 lines)

### Format Support

| D3DFORMAT | Metal Format | Status | Notes |
|:---|:---|:---:|:---|
| `A8R8G8B8` | BGRA8Unorm | ✅ | Direct mapping |
| `X8R8G8B8` | BGRA8Unorm | ✅ | Alpha ignored |
| `R5G6B5` | BGRA8Unorm | ⚠️ | **Format mismatch!** Creates 32-bit Metal texture but LockRect/UnlockRect writes 16-bit data (bpp=2). `replaceRegion` receives 2 bytes/pixel data into a 4 bytes/pixel texture — **corrupted textures** |
| `A1R5G5B5` | BGRA8Unorm | ⚠️ | Same issue as R5G6B5. MetalSurface8 has A1R5G5B5→BGRA8 conversion but MetalTexture8 does NOT |
| `X1R5G5B5` | BGRA8Unorm | ⚠️ | Same issue |
| `A4R4G4B4` | BGRA8Unorm | ⚠️ | Same issue — no conversion code |
| `R8G8B8` | BGRA8Unorm | ⚠️ | **3 bytes per pixel to 4 bytes/pixel texture** — corrupted |
| `A8` | BGRA8Unorm | ⚠️ | **1 byte to 4 bytes** — corrupted |
| `L8` | BGRA8Unorm | ⚠️ | **1 byte to 4 bytes** — should use MTLPixelFormatR8Unorm |
| `P8` (palettized) | BGRA8Unorm | ⚠️ | **1 byte to 4 bytes** — needs palette lookup conversion |
| `DXT1` | BC1_RGBA | ✅ | Correct mapping |
| `DXT2` | — | ❌ | **Not mapped!** Falls through to default BGRA8. Should be BC2_RGBA (premult alpha variant of DXT3) |
| `DXT3` | BC2_RGBA | ✅ | Correct mapping |
| `DXT4` | — | ❌ | **Not mapped!** Falls through to default BGRA8. Should be BC3_RGBA (premult alpha variant of DXT5) |
| `DXT5` | BC3_RGBA | ✅ | Correct mapping |

### Texture Operations

| Operation | Status | Notes |
|:---|:---:|:---|
| `LockRect` | ✅ | Allocates staging buffer, reads back existing data |
| `UnlockRect` | ✅ | Uploads data, frees staging buffer |
| `GetLevelDesc` | ✅ | Returns correct mip dimensions |
| `GetSurfaceLevel` | ✅ | Creates MetalSurface8 linked to mip |
| `AddDirtyRect` | ❌ | Stub — no dirty rect tracking |
| Mipmap auto-generation | ❌ | `m_Levels=0` → clamped to 1. Per DX8 spec, levels=0 means auto-generate all mipmaps. Should calculate `floor(log2(max(w,h))) + 1` |
| Read-back on lock | ⚠️ | Only for uncompressed formats that were previously written — correct per DX8 spec |
| Re-allocation on single-level unlock | ✅ | Creates new texture to avoid GPU sync issues — good workaround |
| **16-bit format conversion in UnlockRect** | ❌ | **Missing!** The data written by the game is in D3D format (16-bit R5G6B5, etc.) but the Metal texture is BGRA8Unorm (32-bit). MetalTexture8::UnlockRect does raw `replaceRegion` without any conversion. This means **all 16-bit textures are corrupted** |

---

## 4. MetalSurface8 (`MetalSurface8.mm` — 270 lines)

| Aspect | Status | Notes |
|:---|:---:|:---|
| `LockRect` | ✅ | Allocates based on format-aware bpp |
| `UnlockRect` | 🔧 | Has A1R5G5B5→BGRA8 conversion. Missing: R5G6B5, A4R4G4B4, X1R5G5B5 conversions |
| `GetDesc` | ⚠️ | Always reports D3DPOOL_DEFAULT — should use actual pool |
| Memory management | ⚠️ | **Does NOT free m_LockedData on UnlockRect** — by design (W3DShroud pattern). Data freed in destructor. This is a spec deviation but necessary for the game |
| Parent texture upload | ✅ | Uploads to parent MetalTexture8's MTL texture at correct mip level |
| Compressed texture upload | ✅ | Handles BC1/BC2/BC3 with correct bytes-per-row calculation |

---

## 5. MetalVertexBuffer8 (`MetalVertexBuffer8.mm` — 133 lines)

| Aspect | Status | Notes |
|:---|:---:|:---|
| Construction | ⚠️ | Uses system memory copy. DX8 spec has D3DPOOL (DEFAULT, MANAGED, SYSTEMMEM) — all treated as shared memory |
| `Lock` | ⚠️ | Returns pointer to system memory directly. **No flags handling**: D3DLOCK_DISCARD, D3DLOCK_NOOVERWRITE, D3DLOCK_NOSYSLOCK are ignored |
| `Unlock` | ✅ | Copies to Metal buffer or marks dirty |
| `GetMTLBuffer` (lazy creation) | ✅ | Creates on first use, updates on dirty |
| `GetDesc` | ✅ | Returns correct FVF, size, type |
| `Release` ref counting | ⚠️ | **Never deletes self!** Sets ref count to 0 but relies on external destructor. Comment says "lifetime managed by DX8VertexBufferClass" — this works but could leak if used differently |
| `D3DUSAGE_DYNAMIC` | ❌ | Not handled — all buffers are effectively dynamic (shared storage) |
| Buffer invalidation | ⚠️ | Copies ENTIRE buffer on unlock even if only partial data changed |

---

## 6. MetalIndexBuffer8 (`MetalIndexBuffer8.mm` — 125 lines)

| Aspect | Status | Notes |
|:---|:---:|:---|
| Construction | ✅ | Supports 16-bit and 32-bit indices |
| `Lock/Unlock` | ✅ | Same pattern as vertex buffer |
| `GetMTLBuffer` | ✅ | Lazy creation with dirty tracking |
| `GetDesc` | ✅ | Returns correct format and size |
| Release ref counting | ⚠️ | Same non-deletion pattern as vertex buffer |

---

## 7. MacOSShaders.metal (`MacOSShaders.metal` — 579 lines)

### Vertex Shader

| Feature | Status | Notes |
|:---|:---:|:---|
| 3D transform (WVP) | ✅ | `projection * view * world * pos` |
| 2D screen space (XYZRHW) | ✅ | Screen→NDC conversion correct |
| Passthrough mode (useProjection=0) | ⚠️ | Not clear when this is used |
| Vertex fog (LINEAR) | ✅ | Formula matches DX8 spec: `(end - d) / (end - start)` |
| Vertex fog (EXP) | ✅ | Formula: `exp(-density * d)` matches spec |
| Vertex fog (EXP2) | ✅ | Formula: `exp(-(density * d)²)` matches spec |
| Fog skip for 2D | ✅ | XYZRHW vertices get fogFactor=1.0 |
| Per-vertex lighting | ✅ | Full Gouraud lighting pipeline |
| Normal transform | ⚠️ | Uses worldView matrix directly. **DX8 spec says use inverse-transpose** for non-uniform scale. Comment acknowledges this but doesn't implement |
| Material source resolution | ✅ | D3DMCS_MATERIAL, COLOR1, COLOR2 all handled |
| Directional light | ✅ | Transform direction to view space, negate |
| Point light attenuation | ✅ | 1/(a0 + a1*d + a2*d²) matches spec |
| Spot light cone | ✅ | Inner/outer cone with falloff matches spec |
| Specular (Blinn-Phong) | ✅ | Uses halfway vector with non-local viewer (0,0,1) — matches DX8 default |
| Final color formula | ✅ | emissive + ambient·matAmbient + diffuse·matDiffuse; alpha=matDiffuse.a |
| Texture coordinate output | ✅ | Passes UV0 and UV1 through |

### Fragment Shader

| Feature | Status | Notes |
|:---|:---:|:---|
| Texture sampling | ✅ | Samples tex0 and tex1 with presence checks |
| TSS Stage 0 processing | ✅ | Full colorOp/alphaOp pipeline |
| TSS Stage 1 processing | ✅ | Full colorOp/alphaOp pipeline |
| `D3DTOP_DISABLE` | ✅ | Stage skipped when disabled |
| `D3DTOP_SELECTARG1` | ✅ | Returns arg1 |
| `D3DTOP_SELECTARG2` | ✅ | Returns arg2 |
| `D3DTOP_MODULATE` | ✅ | arg1 * arg2 |
| `D3DTOP_MODULATE2X` | ✅ | arg1 * arg2 * 2, clamped |
| `D3DTOP_MODULATE4X` | ✅ | arg1 * arg2 * 4, clamped |
| `D3DTOP_ADD` | ✅ | arg1 + arg2, clamped |
| `D3DTOP_ADDSIGNED` | ✅ | arg1 + arg2 - 0.5 |
| `D3DTOP_ADDSIGNED2X` | ✅ | (arg1 + arg2 - 0.5) * 2 |
| `D3DTOP_SUBTRACT` | ✅ | arg1 - arg2 |
| `D3DTOP_ADDSMOOTH` | ✅ | arg1 + arg2 - arg1*arg2 |
| `D3DTOP_BLENDDIFFUSEALPHA` | ✅ | mix(arg2, arg1, diffuse.a) via evaluateBlendOp |
| `D3DTOP_BLENDTEXTUREALPHA` | ✅ | mix(arg2, arg1, texColor0.a) |
| `D3DTOP_BLENDFACTORALPHA` | ✅ | mix(arg2, arg1, tFactor.a) |
| `D3DTOP_BLENDCURRENTALPHA` | ✅ | mix(arg2, arg1, current.a) |
| `D3DTOP_MODULATEALPHA_ADDCOLOR` | ⚠️ | Formula: `arg1.rgb + arg1.a * arg2.rgb` — **DX8 spec says `Arg1.RGB + Arg1.A × Arg2.RGB`** — matches! But should it be `float4(result.rgb, arg1.a)` not `float4(val.rgb, arg1.a)`? Actually the implementation returns the full float4 from the switch, so color and alpha are both set |
| `D3DTOP_MODULATECOLOR_ADDALPHA` | ⚠️ | Formula: `arg1.rgb * arg2.rgb + arg1.a` — DX8 spec is `Arg1.RGB × Arg2.RGB + Arg1.A`. The `.a` is being added to `.rgb` which looks wrong — should only affect color channels |
| `D3DTOP_MODULATEINVALPHA_ADDCOLOR` | ✅ | `(1-arg1.a) * arg2.rgb + arg1.rgb` |
| `D3DTOP_MODULATEINVCOLOR_ADDALPHA` | ❌ | **Missing!** Not in evaluateOp switch — will fall through to default modulate |
| `D3DTOP_DOTPRODUCT3` | ✅ | Correct bias-and-scale then dot product |
| `D3DTOP_MULTIPLYADD` | ❌ | **Missing!** — `Arg0 + Arg1 × Arg2` (3-operand) |
| `D3DTOP_LERP` | ❌ | **Missing!** — `Arg0 × Arg1 + (1-Arg0) × Arg2` (3-operand) |
| `D3DTOP_PREMODULATE` (15) | ❌ | Missing |
| `D3DTOP_BLENDCURRENTALPHA` (16) | ✅ | Implemented |
| `D3DTOP_BLENDFACTORALPHA` (14) | ✅ | Implemented |
| D3DTA_COMPLEMENT modifier | ✅ | `1 - val` applied correctly |
| D3DTA_ALPHAREPLICATE modifier | ✅ | `float4(val.a)` applied correctly |
| D3DTA_TEMP | ❌ | DX8 temporary register not supported |
| Alpha test | ✅ | All D3DCMP functions using `discard_fragment()` |
| Fog application | ✅ | `mix(fogColor, current, fogFactor)` matches DX8 spec |
| **Specular addition** | ⚠️ | **Always adds specular!** Line 575: `current.rgb += specular.rgb`. Per DX8 spec, this should only happen when `D3DRS_SPECULARENABLE = TRUE`. Currently, the render state is default FALSE but the shader always adds. This should be conditional |
| Texture for stage 1 blend ops | ⚠️ | `evaluateBlendOp` always uses `texColor0.a` for `BLENDTEXTUREALPHA` — should use `texColor1.a` when called for stage 1 |

### Shader ↔ CPU Struct Alignment

| Struct | Status | Notes |
|:---|:---:|:---|
| `Uniforms` | ✅ | Layout matches between .metal and .mm |
| `FragmentUniforms` | ✅ | Layout matches |
| `LightingUniforms` | ⚠️ | **Potential alignment issue**: `materialPower` (float) followed by `globalAmbient` (float4). In Metal, float4 must be 16-byte aligned. On CPU side (C++ struct), the padding may differ. Current code uses `memset` so it works, but fragile |
| `LightData` | ✅ | Layout matches between .metal and .mm |

---

## 8. Massive Code Duplication

| Issue | Location | Notes |
|:---|:---|:---|
| Fragment uniform setup | `DrawPrimitive`, `DrawIndexedPrimitive`, `DrawPrimitiveUP` | **TRIPLICATED** ~50 lines each. Should be extracted to a helper like `BuildFragmentUniforms()` |
| Lighting uniform setup | Same 3 methods | **TRIPLICATED** ~40 lines each |
| Vertex uniform setup | Same 3 methods | **TRIPLICATED** ~10 lines each |
| Texture/sampler binding | Same 3 methods | **TRIPLICATED** ~15 lines each |
| Fog parameter extraction | **6 places** (3 in fu, 3 in lu) | Copy-paste of the same fogMode/fogStart/fogEnd/fogDensity code |

---

## 9. Critical Bugs / Spec Violations

### 🔴 P0 — Will cause visual corruption

1. **16-bit texture format mismatch** (`MetalTexture8.mm` lines 41-66)
   - R5G6B5, A1R5G5B5, A4R4G4B4, X1R5G5B5 all create BGRA8Unorm textures (4 bytes/pixel) but LockRect/UnlockRect write 2 bytes/pixel data. The `replaceRegion:withBytes:bytesPerRow:` call will misinterpret the data, creating corrupted textures.
   - **Fix:** Add CPU conversion in `UnlockRect` (like MetalSurface8 does for A1R5G5B5) OR use native 16-bit Metal formats where available.

2. **D3DRS_SPECULARENABLE ignored in shader** (MacOSShaders.metal line 575)
   - Specular color is always added to fragment output regardless of the render state.
   - **Fix:** Pass `specularEnable` flag in FragmentUniforms, check before adding.

3. **D3DRS_BLENDOP not handled** (MetalDevice8.mm)
   - Always uses `MTLBlendOperationAdd`. If game uses `D3DBLENDOP_REVSUBTRACT` (common for subtractive blending effects), the result will be wrong.
   - **Fix:** Map D3DBLENDOP enum to MTLBlendOperation and include in PSO key.

### 🟡 P1 — May cause visual artifacts

4. **D3DPT_TRIANGLEFAN not converted** — Metal has no fan support. If game draws fans, those primitives are silently dropped.

5. **evaluateBlendOp uses texColor0 for stage 1** — When `D3DTOP_BLENDTEXTUREALPHA` is used on stage 1, it should use `texColor1.a` not `texColor0.a`.

6. **D3DTSS_TEXCOORDINDEX not handled** — If game redirects UVs between stages, wrong coordinates will be used.

7. **Missing DXT2/DXT4 format mapping** — Falls through to BGRA8Unorm which is completely wrong for compressed data.

8. **`CreateVertexBuffer` casts count to `unsigned short`** — Overflow for meshes with >65535 vertices.

### 🟢 P2 — Correctness improvements

9. **D3DRS_FILLMODE not handled** — Wireframe debug mode won't work.
10. **Texture coordinate transforms (D3DTS_TEXTURE*)** — Not passed to shader.
11. **DrawIndexedPrimitiveUP is empty** — If called, geometry will be missing.
12. **EnumAdapterModes hardcoded** — Resolution selection in options will be broken.
13. **CheckDeviceFormat always returns OK** — Game may create textures in unsupported formats.
14. **Missing D3DCAPS: TextureFilterCaps, TextureAddressCaps, ShadeCaps** — Game may disable features if caps aren't reported.

---

## 10. Summary Statistics

| Category | Count |
|:---|:---:|
| **IDirect3DDevice8 methods total** | ~90 |
| ✅ Fully implemented & spec-correct | ~35 |
| ⚠️ Implemented with deviations | ~25 |
| ❌ Stubbed/missing | ~30 |
| 🔧 Partially implemented | ~5 |

| Component | Lines of Code | Status |
|:---|:---:|:---|
| MetalDevice8.mm | 2476 | Core rendering engine, functional |
| MetalTexture8.mm | 395 | Working but 16-bit format bug |
| MetalSurface8.mm | 270 | Working for most formats |
| MetalVertexBuffer8.mm | 133 | Functional |
| MetalIndexBuffer8.mm | 125 | Functional |
| MetalInterface8.mm | 184 | Minimal but functional |
| MacOSShaders.metal | 579 | Full TSS + lighting pipeline |
| **Total** | **4162** | |

---

## 11. Prioritized Fix List

1. **Fix 16-bit texture format conversion** in MetalTexture8::UnlockRect
2. **Add D3DRS_BLENDOP support** to PSO creation
3. **Conditionally add specular** based on D3DRS_SPECULARENABLE
4. **Fix BLENDTEXTUREALPHA for stage 1** to use correct texture alpha  
5. **Add DXT2/DXT4 to format mapping** (map to BC2/BC3)
6. **Convert D3DPT_TRIANGLEFAN** to triangle list in draw calls
7. **Remove CreateVertexBuffer `unsigned short` cast** — use UINT
8. **Extract duplicated uniform setup** into helper methods
9. **Add D3DTSS_TEXCOORDINDEX** handling
10. **Report missing D3DCAPS** (TextureFilterCaps, etc.)
