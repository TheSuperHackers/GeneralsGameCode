# DX8 → Metal Graphics Backend — Implementation Plan

> **Цель:** Чистая, по спецификации DX8.1, реализация графического backend-а на Metal.
> Заменяет все ad-hoc заглушки одним архитектурно-чистым адаптером.

## Архитектура

```
┌──────────────────────────────────────────────────┐
│            Игровой код (W3D Engine)               │
│  DX8Wrapper::Set_Texture, Draw_Triangles, etc.   │
└────────────────────┬─────────────────────────────┘
                     │ вызывает
                     ▼
┌──────────────────────────────────────────────────┐
│         IDirect3DDevice8 (COM интерфейс)          │
│    d3d8_stub.h — виртуальные методы               │
└────────────────────┬─────────────────────────────┘
                     │ реализует
                     ▼
┌──────────────────────────────────────────────────┐
│     MetalDevice8 : IDirect3DDevice8    [НОВЫЙ]    │
│                                                   │
│  ┌─────────────┐ ┌──────────────┐ ┌────────────┐ │
│  │ State Cache  │ │ Buffer Mgr   │ │ Texture Mgr│ │
│  │ RenderStates │ │ VB/IB Pool   │ │ DDS/TGA    │ │
│  │ TSS Cache    │ │ MTLBuffer    │ │ MTLTexture │ │
│  └─────────────┘ └──────────────┘ └────────────┘ │
│  ┌─────────────────────────────────────────────┐  │
│  │        MetalPipelineManager                  │  │
│  │  - FVF → MTLVertexDescriptor                 │  │
│  │  - Blend/Depth/Stencil state cache           │  │
│  │  - Pipeline State Objects (PSO) cache         │  │
│  └─────────────────────────────────────────────┘  │
│  ┌─────────────────────────────────────────────┐  │
│  │     Metal Shaders (fixed_function.metal)      │  │
│  │  - Vertex: transform + per-vertex lighting    │  │
│  │  - Fragment: texture stage blending + fog     │  │
│  └─────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────┘
                     │
                     ▼
              Metal Framework (GPU)
```

## Что удаляем / заменяем

| Файл | Действие |
|:---|:---|
| `MacOSRenderer.mm` (858 строк) | **УДАЛЯЕМ** — заменяется MetalDevice8 |
| `MacOSShaders.metal` (104 строки) | **УДАЛЯЕМ** — заменяется `fixed_function.metal` |
| `Shaders.metal` | **УДАЛЯЕМ** — дубликат |
| `MacOSRenderDevice_Internal.h` | **УДАЛЯЕМ** — заменяется MetalDevice8.h |
| `IRenderDevice.h` | **УДАЛЯЕМ** — не нужен, используем IDirect3DDevice8 |
| `D3DXStubs.cpp` | **ОБНОВЛЯЕМ** — дополняем недостающее |

## Что оставляем

- `d3d8_stub.h` — все типы, enums, интерфейсы (уже корректны)
- `dx8wrapper.h/cpp` — W3D обёртка, вызывает наш MetalDevice8
- `d3dx8math.h`, `d3dx8tex.h` — helper функции
- Весь остальной движок — НЕ ТРОГАЕМ

---

## Источники истины

1. **`documentation.pdf`** (549 стр.) — DX8.1 SDK, формулы освещения, TSS, blend states
2. **`d3d8_stub.h`** (1115 строк) — все enums/types/interfaces
3. **`dx8wrapper.h`** (1546 строк) — какие методы реально вызываются
4. **`.agent/dx8_spec_extracted.txt`** (497 KB) — извлечённый текст спецификации

---

## Этапы реализации

### Этап 0: Каркас (фундамент)
**Цель:** MetalDevice8 компилируется и движок запускается через него

**Файлы:**
- `Platform/MacOS/Source/Metal/MetalDevice8.h` — объявления
- `Platform/MacOS/Source/Metal/MetalDevice8.mm` — реализация
- `Platform/MacOS/Source/Metal/MetalInterface8.h` — IDirect3D8
- `Platform/MacOS/Source/Metal/MetalInterface8.mm` — реализация

**Задачи:**
1. Создать класс `MetalDevice8 : IDirect3DDevice8` с ВСЕМИ методами (заглушки, return D3D_OK)
2. Создать класс `MetalInterface8 : IDirect3D8` (CreateDevice → создаёт MetalDevice8)
3. В `dx8wrapper.cpp`, функции `CreateMacOSD3D8()` и `CreateMacOSD3DDevice8()` — вернуть наши классы
4. Metal device, command queue, CAMetalLayer — инициализация в конструкторе
5. **Проверка:** Игра запускается, логирование показывает вызовы всех методов

**Критерий готовности:** Сборка проходит, игра запускается, пустой экран

---

### Этап 1: Scene Management + Clear
**Цель:** Чёрный экран с правильным clear color

**Спецификация (documentation.pdf p.258):**
- `BeginScene` — начало рендеринга кадра
- `EndScene` — конец рендеринга кадра  
- `Present` — показ backbuffer на экране
- `Clear` — заполнение render target цветом/depth/stencil

**Реализация:**
```
BeginScene   → [m_CommandQueue commandBuffer], nextDrawable, beginEncoder
EndScene     → endEncoder
Present      → presentDrawable, commitCommandBuffer
Clear(flags) → MTLRenderPassDescriptor с clearColor/clearDepth
```

**Задачи:**
1. `BeginScene()` — создать MTLCommandBuffer, получить drawable, начать render pass
2. `EndScene()` — закончить render pass encoder
3. `Present()` — present drawable, commit command buffer
4. `Clear(count, rects, flags, color, z, stencil)`:
   - `D3DCLEAR_TARGET` → `loadAction = MTLLoadActionClear`, clearColor из D3DCOLOR
   - `D3DCLEAR_ZBUFFER` → `clearDepth`
   - `D3DCLEAR_STENCIL` → `clearStencil`
5. `SetViewport(D3DVIEWPORT8)` → `[encoder setViewport:]`

**Критерий:** Экран заливается цветом, указанным игрой

---

### Этап 2: Vertex/Index Buffers
**Цель:** Геометрия создаётся и хранится в GPU памяти

**Спецификация (d3d8_stub.h, строки 883-895):**
- `IDirect3DVertexBuffer8` — Lock/Unlock/GetDesc
- `IDirect3DIndexBuffer8` — Lock/Unlock/GetDesc

**Файлы:**
- `Platform/MacOS/Source/Metal/MetalVertexBuffer8.h/mm`
- `Platform/MacOS/Source/Metal/MetalIndexBuffer8.h/mm`

**Реализация:**
```
CreateVertexBuffer(length, usage, fvf, pool) → MetalVertexBuffer8
  - MTLBuffer с MTLResourceStorageModeShared
  - Хранит FVF, размер
  
Lock(offset, size, &ptr, flags) → contents() + offset
Unlock() → no-op (shared memory), или didModifyRange
```

**FVF (Flexible Vertex Format) парсер** — критически важная часть:
```
FVF bits → stride и layout:
  D3DFVF_XYZ      → float3 position (12 bytes)
  D3DFVF_XYZRHW   → float4 position (16 bytes, pre-transformed)
  D3DFVF_NORMAL   → float3 normal (12 bytes)
  D3DFVF_DIFFUSE  → DWORD color (4 bytes)
  D3DFVF_SPECULAR → DWORD color (4 bytes)
  D3DFVF_TEX1     → float2 uv (8 bytes)
  D3DFVF_TEX2     → + float2 uv2 (8 bytes)
  D3DFVF_XYZBn    → bone weights
```

**Задачи:**
1. Реализовать `MetalVertexBuffer8` с MTLBuffer backing
2. Реализовать `MetalIndexBuffer8` (16-bit indices)
3. FVF parser: `DWORD fvf` → `{stride, offsets для position, normal, diffuse, specular, texN}`
4. `CreateVertexBuffer` / `CreateIndexBuffer` в MetalDevice8
5. `SetStreamSource(stream, vb, stride)` — запомнить текущий VB
6. `SetIndices(ib, baseVertex)` — запомнить текущий IB

**Критерий:** Буферы создаются без крашей, Lock/Unlock работают

---

### Этап 3: Pipeline State + Draw Calls
**Цель:** Треугольники рисуются на экране (белые, без текстур)

**Спецификация:**
- `DrawPrimitive(type, startVertex, primCount)` 
- `DrawIndexedPrimitive(type, minIndex, numVertices, startIndex, primCount)`
- Primitive types: TRIANGLELIST, TRIANGLESTRIP, TRIANGLEFAN, LINELIST, LINESTRIP

**Pipeline State Object (PSO) кэш:**
```
PSO key = hash(FVF, blendEnable, srcBlend, dstBlend, 
               depthEnable, depthWrite, depthFunc,
               cullMode, colorWriteMask)
```

**Файлы:**
- `Platform/MacOS/Source/Metal/MetalPipelineManager.h/mm`

**Задачи:**
1. FVF → `MTLVertexDescriptor` маппинг
2. PSO кэш: `std::unordered_map<uint64_t, id<MTLRenderPipelineState>>`
3. Depth/Stencil state кэш: `std::unordered_map<uint32_t, id<MTLDepthStencilState>>`
4. `DrawPrimitive` → `[encoder drawPrimitives:vertexStart:vertexCount:]`
5. `DrawIndexedPrimitive` → `[encoder drawIndexedPrimitives:indexCount:indexType:indexBuffer:indexBufferOffset:]`
6. Primitive type маппинг:
   - `D3DPT_TRIANGLELIST` → `MTLPrimitiveTypeTriangle`
   - `D3DPT_TRIANGLESTRIP` → `MTLPrimitiveTypeTriangleStrip`
   - `D3DPT_LINELIST` → `MTLPrimitiveTypeLine`
   - `D3DPT_LINESTRIP` → `MTLPrimitiveTypeLineStrip`
   - `D3DPT_TRIANGLEFAN` → конвертировать в triangle list (Metal не поддерживает)

**Критерий:** Треугольники видны на экране

---

### Этап 4: Transforms (матрицы)
**Цель:** 3D трансформации работают — объекты в правильных позициях

**Спецификация (d3d8_stub.h строки 633-645):**
```
D3DTS_WORLD      = 256  → Model matrix
D3DTS_VIEW       = 2    → Camera matrix  
D3DTS_PROJECTION = 3    → Projection matrix
D3DTS_TEXTUREn   = 16+n → Texture coordinate transform
```

**Формула (стандартная):**
```
clipPos = Projection × View × World × localPos
```

**Для pre-transformed вершин (D3DFVF_XYZRHW):**
```
clipPos.x = (x / screenWidth) * 2 - 1
clipPos.y = 1 - (y / screenHeight) * 2
clipPos.z = z
clipPos.w = 1/rhw
```

**Uniform buffer layout:**
```metal
struct Uniforms {
    float4x4 world;
    float4x4 view;  
    float4x4 projection;
    float4x4 texTransform[2];
    float2 screenSize;
    float2 _padding;
};
```

**Задачи:**
1. `SetTransform(state, matrix)` — сохранить в массив `m_Transforms[state]`
2. `GetTransform(state, matrix)` — вернуть из массива
3. Перед draw call: заполнить uniform buffer и привязать к encoder
4. Vertex shader: умножить позицию на WVP
5. Обработка XYZRHW (2D режим, UI) — screen space → clip space

**Критерий:** UI элементы в правильных позициях, 3D объекты с перспективой

---

### Этап 5: Textures
**Цель:** Текстуры загружаются и отображаются на геометрии

**Спецификация (d3d8_stub.h строки 917-926):**
```
IDirect3DTexture8 — GetLevelDesc, GetSurfaceLevel, LockRect, UnlockRect
```

**Файлы:**
- `Platform/MacOS/Source/Metal/MetalTexture8.h/mm`
- `Platform/MacOS/Source/Metal/MetalSurface8.h/mm`

**Форматы текстур (маппинг DX8 → Metal):**
| D3DFORMAT | Metal | Bytes/pixel |
|:---|:---|:---|
| D3DFMT_A8R8G8B8 | MTLPixelFormatBGRA8Unorm | 4 |
| D3DFMT_X8R8G8B8 | MTLPixelFormatBGRA8Unorm | 4 |
| D3DFMT_R5G6B5 | MTLPixelFormatB5G6R5Unorm* | 2 |
| D3DFMT_A1R5G5B5 | MTLPixelFormatA1BGR5Unorm* | 2 |
| D3DFMT_A4R4G4B4 | MTLPixelFormatABGR4Unorm* | 2 |
| D3DFMT_A8 | MTLPixelFormatA8Unorm | 1 |
| D3DFMT_DXT1 | MTLPixelFormatBC1_RGBA | 0.5 |
| D3DFMT_DXT2/3 | MTLPixelFormatBC2_RGBA | 1 |
| D3DFMT_DXT4/5 | MTLPixelFormatBC3_RGBA | 1 |
| D3DFMT_L8 | MTLPixelFormatR8Unorm | 1 |
| D3DFMT_P8 | → convert to A8R8G8B8 | 4 |

\* Некоторые 16-bit форматы недоступны на macOS Metal — конвертируем в BGRA8

**Задачи:**
1. `CreateTexture(w, h, levels, usage, format)` → MetalTexture8
2. `LockRect(level, &rect, rect, flags)` → staging buffer
3. `UnlockRect(level)` → blit staging → MTLTexture
4. `SetTexture(stage, texture)` → `[encoder setFragmentTexture:atIndex:]`
5. DDS loader integration (уже частично есть)
6. Swizzle ARGB → BGRA при необходимости

**Критерий:** Текстуры видны на геометрии

---

### Этап 6: Render States
**Цель:** Прозрачность, z-buffer, отсечение граней работают правильно

**Спецификация (d3d8_stub.h строки 404-482):**

**Группа 1: Depth/Stencil → `MTLDepthStencilDescriptor`**
| D3DRS | Metal |
|:---|:---|
| D3DRS_ZENABLE | depthCompareFunction != Never |
| D3DRS_ZWRITEENABLE | isDepthWriteEnabled |
| D3DRS_ZFUNC | depthCompareFunction (D3DCMP → MTLCompareFunction) |
| D3DRS_STENCILENABLE | stencil config |
| D3DRS_STENCILFUNC/REF/MASK | frontFaceStencil |

**Маппинг D3DCMP → MTLCompareFunction:**
```
D3DCMP_NEVER        → MTLCompareFunctionNever
D3DCMP_LESS         → MTLCompareFunctionLess  
D3DCMP_EQUAL        → MTLCompareFunctionEqual
D3DCMP_LESSEQUAL    → MTLCompareFunctionLessEqual
D3DCMP_GREATER      → MTLCompareFunctionGreater
D3DCMP_NOTEQUAL     → MTLCompareFunctionNotEqual
D3DCMP_GREATEREQUAL → MTLCompareFunctionGreaterEqual
D3DCMP_ALWAYS       → MTLCompareFunctionAlways
```

**Группа 2: Alpha Blending → `MTLRenderPipelineColorAttachmentDescriptor`**
| D3DRS | Metal |
|:---|:---|
| D3DRS_ALPHABLENDENABLE | blendingEnabled |
| D3DRS_SRCBLEND | sourceRGBBlendFactor |
| D3DRS_DESTBLEND | destinationRGBBlendFactor |
| D3DRS_BLENDOP | rgbBlendOperation |

**Маппинг D3DBLEND → MTLBlendFactor:**
```
D3DBLEND_ZERO         → MTLBlendFactorZero
D3DBLEND_ONE          → MTLBlendFactorOne
D3DBLEND_SRCCOLOR     → MTLBlendFactorSourceColor
D3DBLEND_INVSRCCOLOR  → MTLBlendFactorOneMinusSourceColor
D3DBLEND_SRCALPHA     → MTLBlendFactorSourceAlpha
D3DBLEND_INVSRCALPHA  → MTLBlendFactorOneMinusSourceAlpha
D3DBLEND_DESTALPHA    → MTLBlendFactorDestinationAlpha
D3DBLEND_INVDESTALPHA → MTLBlendFactorOneMinusDestinationAlpha
D3DBLEND_DESTCOLOR    → MTLBlendFactorDestinationColor
D3DBLEND_INVDESTCOLOR → MTLBlendFactorOneMinusDestinationColor
D3DBLEND_SRCALPHASAT  → MTLBlendFactorSourceAlphaSaturated
```

**Группа 3: Miscellaneous → encoder calls**
| D3DRS | Metal |
|:---|:---|
| D3DRS_CULLMODE | [encoder setCullMode:] |
| D3DRS_FILLMODE | [encoder setTriangleFillMode:] |
| D3DRS_ALPHATESTENABLE + D3DRS_ALPHAREF | fragment shader discard |
| D3DRS_COLORWRITEENABLE | colorWriteMask на PSO |

**Маппинг D3DCULL → MTLCullMode:**
```
D3DCULL_NONE → MTLCullModeNone
D3DCULL_CW   → MTLCullModeFront (DX8 CW = Metal Front)
D3DCULL_CCW  → MTLCullModeBack
```
*Внимание: DX8 и Metal используют разные winding orders по умолчанию!*

**Задачи:**
1. `SetRenderState(state, value)` — кэш в массив `m_RenderStates[256]`
2. Dirty tracking: при изменении RS, пометить соответствующий Metal state как dirty
3. Перед draw: пересоздать необходимые Metal state objects
4. PSO кэш с включением blend state в ключ
5. Depth/Stencil state кэш

**Критерий:** Прозрачные объекты рисуются корректно, z-fighting отсутствует

---

### Этап 7: Texture Stage States (самое сложное!)
**Цель:** Multi-texturing и texture blending работают как в DX8

**Спецификация (d3d8_stub.h строки 681-709, 768-796, 815-823):**

DX8 имеет до 8 texture stages. Каждый stage:
- Входы: `D3DTA_TEXTURE`, `D3DTA_CURRENT`, `D3DTA_DIFFUSE`, `D3DTA_TFACTOR`, `D3DTA_SPECULAR`
- Операция: `D3DTOP_*` (цвет и альфа отдельно)
- Модификаторы: `D3DTA_COMPLEMENT` (1-x), `D3DTA_ALPHAREPLICATE` (a,a,a,a)

**Формулы D3DTOP (из спецификации):**
```
D3DTOP_DISABLE            = этот и последующие stages отключены
D3DTOP_SELECTARG1         = Arg1
D3DTOP_SELECTARG2         = Arg2
D3DTOP_MODULATE           = Arg1 × Arg2
D3DTOP_MODULATE2X         = Arg1 × Arg2 × 2
D3DTOP_MODULATE4X         = Arg1 × Arg2 × 4
D3DTOP_ADD                = Arg1 + Arg2
D3DTOP_ADDSIGNED          = Arg1 + Arg2 - 0.5
D3DTOP_ADDSIGNED2X        = (Arg1 + Arg2 - 0.5) × 2
D3DTOP_SUBTRACT           = Arg1 - Arg2
D3DTOP_ADDSMOOTH          = Arg1 + Arg2 - Arg1 × Arg2
D3DTOP_BLENDDIFFUSEALPHA  = Arg1 × Alpha(Diffuse) + Arg2 × (1 - Alpha(Diffuse))
D3DTOP_BLENDTEXTUREALPHA  = Arg1 × Alpha(Texture) + Arg2 × (1 - Alpha(Texture))
D3DTOP_BLENDFACTORALPHA   = Arg1 × Alpha(Factor) + Arg2 × (1 - Alpha(Factor))
D3DTOP_BLENDCURRENTALPHA  = Arg1 × Alpha(Current) + Arg2 × (1 - Alpha(Current))
D3DTOP_MODULATEALPHA_ADDCOLOR   = Arg1.RGB + Arg1.A × Arg2.RGB
D3DTOP_MODULATECOLOR_ADDALPHA   = Arg1.RGB × Arg2.RGB + Arg1.A
D3DTOP_MODULATEINVALPHA_ADDCOLOR = (1-Arg1.A) × Arg2.RGB + Arg1.RGB
D3DTOP_MODULATEINVCOLOR_ADDALPHA = (1-Arg1.RGB) × Arg2.RGB + Arg1.A
D3DTOP_DOTPRODUCT3        = dot(Arg1, Arg2) × 4 (в range [-1,1])
D3DTOP_MULTIPLYADD        = Arg0 + Arg1 × Arg2
D3DTOP_LERP               = Arg0 × Arg1 + (1-Arg0) × Arg2
```

**Sampler States из TSS:**
| D3DTSS | Metal |
|:---|:---|
| D3DTSS_ADDRESSU | MTLSamplerAddressMode |
| D3DTSS_ADDRESSV | MTLSamplerAddressMode |
| D3DTSS_MINFILTER | MTLSamplerMinMagFilter |
| D3DTSS_MAGFILTER | MTLSamplerMinMagFilter |
| D3DTSS_MIPFILTER | MTLSamplerMipFilter |

**Подход к реализации:**

Generals использует максимум 2 texture stages. Вместо полной эмуляции 8 stages,
реализуем через uniform buffer, передающий операцию и аргументы в fragment shader:

```metal
struct TextureStageConfig {
    uint colorOp;     // D3DTOP enum
    uint colorArg1;   // D3DTA enum
    uint colorArg2;   // D3DTA enum
    uint alphaOp;     // D3DTOP enum
    uint alphaArg1;   // D3DTA enum
    uint alphaArg2;   // D3DTA enum
};

struct FragmentUniforms {
    TextureStageConfig stages[2];
    float4 textureFactor;  // D3DRS_TEXTUREFACTOR
    float4 fogColor;
    float fogStart;
    float fogEnd;
    float fogDensity;
    uint fogMode;
    uint alphaTestEnable;
    uint alphaFunc;
    float alphaRef;
};
```

**Задачи:**
1. `SetTextureStageState(stage, type, value)` — кэш в `m_TSS[stage][type]`
2. Fragment uniform buffer с TextureStageConfig
3. Fragment shader: вычисление по формулам D3DTOP
4. Sampler state кэш: `std::map<uint32_t, id<MTLSamplerState>>`
5. Texture coordinate generation (D3DTSS_TEXCOORDINDEX flags)

**Критерий:** Мультитекстурирование работает, terrain с двумя текстурами виден

---

### Этап 8: Lighting (per-vertex)
**Цель:** Per-vertex освещение по формулам DX8

**Спецификация (documentation.pdf p.118-119):**

**Формула Global Illumination:**
```
VertexColor = Emissive + Ambient_global + Σ(Ambient_i + Diffuse_i + Specular_i)
```

**Формула Diffuse:**
```
Diffuse_i = Cd × Ld × max(0, N · Ldir) × Atten × Spot
```
- `Cd` = vertex diffuse color или material diffuse (по D3DRS_DIFFUSEMATERIALSOURCE)
- `Ld` = light diffuse color
- `N` = normalized vertex normal
- `Ldir` = normalized direction to light
- `Atten` = 1/(att0 + att1×d + att2×d²) для point/spot, 1.0 для directional

**Формула Specular:**
```
Specular_i = Cs × Ls × max(0, N · H)^P × Atten × Spot
```
- `H` = normalize(Ldir + Vdir) — halfway vector
- `P` = material power (glossiness)

**Формула Spotlight:**
```
Spot = (cos(angle) - cos(Phi/2))^Falloff / (cos(Theta/2) - cos(Phi/2))
```

**Формула Ambient per-light:**
```
Ambient_i = Ca × La × Atten × Spot
```
- `Ca` = vertex ambient color или material ambient
- `La` = light ambient color

**Uniform buffer для освещения:**
```metal
struct LightData {
    float4 diffuse;
    float4 ambient;
    float4 specular;
    float3 position;
    float range;
    float3 direction;
    float falloff;
    float attenuation0;
    float attenuation1;
    float attenuation2;
    float theta;       // inner cone
    float phi;         // outer cone
    uint type;         // 1=point, 2=spot, 3=directional
    uint enabled;
    float2 _padding;
};

struct LightingUniforms {
    LightData lights[4];
    float4 materialDiffuse;
    float4 materialAmbient;
    float4 materialSpecular;
    float4 materialEmissive;
    float materialPower;
    float4 globalAmbient;
    uint lightingEnabled;
    uint diffuseSource;   // 0=material, 1=color1, 2=color2
    uint ambientSource;
    uint specularSource;
    uint emissiveSource;
};
```

**ВАЖНО:** Generals в основном использует:
- 4 directional lights (солнце + 3 fill)
- Vertex colors для подсветки ландшафта
- D3DRS_LIGHTING = FALSE для UI и некоторых эффектов

**Задачи:**
1. `SetLight(index, D3DLIGHT8)` — сохранить в uniform
2. `LightEnable(index, bool)` — вкл/выкл
3. `SetMaterial(D3DMATERIAL8)` — сохранить в uniform
4. `SetRenderState(D3DRS_LIGHTING, v)` — переключатель
5. `SetRenderState(D3DRS_AMBIENT, color)` — global ambient
6. `SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, v)` — источник цвета
7. Vertex shader: полный расчёт per-vertex lighting по формулам

**Критерий:** Освещённые объекты, тени от направленного света

---

### Этап 9: Fog
**Цель:** Туман работает (для дальних объектов и атмосферы)

**Спецификация (documentation.pdf p.262, d3d8_stub.h строки 257-260):**

**Формулы fog factor (0 = полный fog, 1 = нет fog):**
```
Linear: f = (end - d) / (end - start)
Exp:    f = 1 / e^(density × d)
Exp2:   f = 1 / e^(density × d)²
```

**Применение:**
```
finalColor = f × objectColor + (1-f) × fogColor
```

**Render States:**
```
D3DRS_FOGENABLE       → вкл/выкл
D3DRS_FOGCOLOR        → D3DCOLOR (ARGB)
D3DRS_FOGTABLEMODE    → 0=none, 1=exp, 2=exp2, 3=linear (pixel fog)
D3DRS_FOGVERTEXMODE   → то же для vertex fog
D3DRS_FOGSTART        → float (для linear)
D3DRS_FOGEND          → float (для linear)
D3DRS_FOGDENSITY      → float (для exp/exp2)
```

**Задачи:**
1. Передать fog параметры во FragmentUniforms
2. В vertex shader: вычислить distance для vertex fog
3. В fragment shader: применить формулу и blend с fog color

**Критерий:** Туман видим, дальние объекты плавно затуманиваются

---

### Этап 10: Depth Buffer + Render Targets
**Цель:** Z-buffer работает, рендеринг в текстуры для теней

**Задачи:**
1. Создать depth texture: `MTLPixelFormatDepth32Float_Stencil8`
2. Привязать к render pass descriptor
3. `CreateDepthStencilSurface` → MetalSurface8 с depth MTLTexture
4. `SetRenderTarget(colorSurface, depthSurface)` — смена render target
5. `CreateRenderTarget` → для shadow maps и post-processing
6. `GetBackBuffer` → вернуть primary render target

**Критерий:** Нет z-fighting, правильная перекрытие объектов

---

### Этап 11: Дополнительные ресурсы
**Цель:** Cube textures, volume textures, surface copy

**Задачи:**
1. `CreateCubeTexture` → MetalCubeTexture8 (для environment maps)
2. `CopyRects` → blit encoder copy
3. `GetFrontBuffer` → screenshots

---

## Порядок тестирования

| Этап | Визуальный результат |
|:---|:---|
| 0 | Компиляция, пустой экран |
| 1 | Чёрный/цветной экран (Clear) |
| 2 | Ничего (буферы без рисования) |
| 3 | Белые треугольники |
| 4 | Треугольники в правильных позициях (3D + 2D UI) |
| 5 | Текстурированные объекты |
| 6 | Корректная прозрачность, z-buffer |
| 7 | Мультитекстурирование, terrain |
| 8 | Освещённая сцена |
| 9 | Атмосферный туман |
| 10 | Тени |

## Количество кода (оценка)

| Компонент | Строки |
|:---|:---|
| MetalDevice8 (все методы) | ~2000 |
| MetalInterface8 | ~200 |
| MetalVertexBuffer8 / IndexBuffer8 | ~300 |
| MetalTexture8 / Surface8 | ~500 |
| MetalPipelineManager | ~600 |
| fixed_function.metal (vertex + fragment) | ~400 |
| FVF parser | ~150 |
| State mapping utilities | ~200 |
| **ИТОГО** | **~4350** |

## Зависимости между этапами

```
Этап 0 ──→ Этап 1 ──→ Этап 2 ──→ Этап 3 ──→ Этап 4
                                      │          │
                                      ▼          ▼
                                   Этап 5 ──→ Этап 6
                                      │          │
                                      ▼          ▼
                                   Этап 7 ──→ Этап 8 ──→ Этап 9 ──→ Этап 10
```

Этапы 0-6 — must have для рабочего 3D.
Этапы 7-10 — для качественного рендеринга.
Этап 11 — по мере необходимости.
