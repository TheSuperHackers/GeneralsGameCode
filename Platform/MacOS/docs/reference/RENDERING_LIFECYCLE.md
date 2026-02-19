# Жизненный цикл инициализации и отрисовки (macOS/Metal) [RENDERING_FLOW.md]

Этот документ описывает цепочку вызовов от запуска приложения до вывода пикселей на экран.
**Последнее обновление:** 2026-02-18

## 1. Инициализация (Startup Flow)

1.  **`MacOSMain.mm :: main()`**: Точка входа. Инициализирует менеджер памяти и сигналы. Вызывает `MacOS_Main`.
2.  **`MacOSWindowManager.mm :: MacOS_Main()`**: 
    *   Настраивает `NSApplication` (Cocoa).
    *   Создаёт окно (`MacOS_CreateWindow`).
    *   Инициализирует Metal-рендерер (`MacOS_InitRenderer` -> `MacOSRenderDevice::Initialize`).
    *   Вызывает **`GameMain()`**.
3.  **`GameMain.cpp :: GameMain()`**: Создаёт глобальные объекты `TheGameEngine` и `TheFramePacer`. Вызывает `TheGameEngine->init()`.
4.  **`GameEngine.cpp :: GameEngine::init()`**: Инициализирует все подсистемы (File System, INI, Audio, AI).
    *   Создаёт `TheGameClient` (на macOS это объект `MacOSGameClient`).
    *   `MacOSGameClient` создаёт `TheDisplay` (объект `MacOSDisplay`).

## 2. Главный цикл (Main Loop)

Сердце игры находится в **`GameEngine::execute()`** (`GameEngine.cpp`):

```cpp
while (!m_quitting) {
    MacOS_PumpEvents();    // Обработка событий мыши/клавы
    update();              // Обновление логики и ЗАПУСК ИЕРАРХИИ ОТРИСОВКИ
    TheFramePacer->update();
}
```

## 3. Иерархия отрисовки (Rendering Hierarchy)

Отрисовка в Generals — это не один вызов, а цепочка вложенных систем:

### Уровень 1: Engine Update (`GameEngine::update`)
Вызывает `TheGameClient->UPDATE()`.

### Уровень 2: Client Update (`GameClient::update`)
В зависимости от состояния (меню или игра) вызывает:
*   `TheDisplay->UPDATE()`
*   `TheDisplay->DRAW()` (Здесь начинается магия W3D).

### Уровень 3: W3D / Metal Lifecycle (`MacOSDisplay::draw`)
Этот метод в `MacOSDisplay.mm` управляет состоянием кадра:
1.  **`WW3D::Begin_Render()`** -> MetalDevice8::BeginScene()
2.  **`Display::draw()`** -> Отрисовка 3D видов (TacticalView).
3.  **`TheWindowManager->winRepaint()`** -> Отрисовка 2D интерфейса (SAGE UI).
4.  **`WW3D::End_Render()`** -> MetalDevice8::EndScene()

## 4. Metal Renderer (MetalDevice8.mm)

### 4.1. Frame Lifecycle

*   **`BeginScene()`**:
    *   Проверяет `m_InScene`.
    *   Создаёт `MTLCommandBuffer` из `m_CommandQueue`.
    *   Получает `CAMetalDrawable` из `CAMetalLayer`.
*   **`Clear(count, rects, flags, color, z, stencil)`**:
    *   Завершает текущий encoder (если есть).
    *   Создает `MTLRenderPassDescriptor`:
      - `D3DCLEAR_TARGET` → `MTLLoadActionClear` + clearColor.
      - Без D3DCLEAR_TARGET → `MTLLoadActionLoad`.
    *   Создаёт новый `MTLRenderCommandEncoder`.
    *   Устанавливает `MTLViewport` из `m_Viewport`.
*   **`EndScene()`**: Сбрасывает `m_InScene`.
*   **`Present()`**:
    *   `endEncoding` на текущем encoder.
    *   `presentDrawable` + `commit` command buffer.
    *   Освобождает encoder, drawable, command buffer.

### 4.2. Pipeline State Objects (PSO)

**`GetPSO(DWORD fvf)`** — создаёт или извлекает из кэша (`m_PsoCache`):

1.  **Vertex Function**: `vertex_main` (из `MacOSShaders.metal`).
2.  **Fragment Function**: `fragment_main`.
3.  **Color Attachment**: `MTLPixelFormatBGRA8Unorm`, blending = Standard Alpha (hardcoded).
4.  **Vertex Descriptor** — строится по FVF:
    - `D3DFVF_XYZ` → attr[0] float3 (12B)
    - `D3DFVF_XYZRHW` → attr[0] float4 (16B)
    - `D3DFVF_NORMAL` → skip 12B (не в шейдере)
    - `D3DFVF_DIFFUSE` → attr[1] uchar4norm (4B)
    - `D3DFVF_SPECULAR` → skip 4B
    - `D3DFVF_TEX1` → attr[2] float2 (8B)
5.  Кэшируется по `fvf` ключу: `std::map<uint32_t, void*>`.

**⚠️ Известные пробелы PSO:**
- Blending всегда Standard Alpha (нет PSO re-creation по D3DRS_SRCBLEND/DESTBLEND).
- Нет Depth/Stencil state object.
- Нет CullMode binding.
- Не обрабатываются `D3DFVF_TEX2`, `D3DFVF_XYZBn` (bone weights).

### 4.3. Draw Calls

**`DrawPrimitive(pt, sv, pc)`** и **`DrawIndexedPrimitive(pt, mi, nv, si, pc)`**:

1.  Получает FVF из VB через `GetBufferFVF(m_StreamSource)`.
2.  Получает/создаёт PSO через `GetPSO(fvf)`.
3.  Устанавливает PSO в encoder.
4.  Привязывает Vertex Buffer: `setVertexBuffer:offset:atIndex:0`.
5.  Заполняет **`MetalUniforms`** и отправляет через `setVertexBytes` / `setFragmentBytes` в buffer index 1:
    - `world`, `view`, `projection` — из `m_Transforms[]`.
    - `screenSize` — ширина/высота окна.
    - `useProjection` — 1 (3D) или 2 (Screen Space, XYZRHW).
    - `shaderSettings` — bit field (texturing on/off).
6.  Привязывает текстуру stage 0 (если есть): `setFragmentTexture:atIndex:0`.
7.  Маппинг примитивов:
    - `D3DPT_TRIANGLELIST` → `MTLPrimitiveTypeTriangle`, vertexCount = pc * 3
    - `D3DPT_TRIANGLESTRIP` → `MTLPrimitiveTypeTriangleStrip`, vertexCount = pc + 2
    - `D3DPT_LINELIST` → `MTLPrimitiveTypeLine`, vertexCount = pc * 2
8.  `drawPrimitives` или `drawIndexedPrimitives`.

**DrawPrimitiveUP / DrawIndexedPrimitiveUP** — ❌ не реализованы (заглушки, return D3D_OK).

### 4.4. Resource Creation

| Ресурс | Метод | Аллокация |
|:---|:---|:---|
| MetalTexture8 | `CreateTexture` | `W3DNEW MetalTexture8(this, w, h, l, u, f, p)` |
| MetalVertexBuffer8 | `CreateVertexBuffer` | `W3DNEW MetalVertexBuffer8(FVF, count, vertexSize)` |
| MetalIndexBuffer8 | `CreateIndexBuffer` | `W3DNEW MetalIndexBuffer8(count, is32bit)` |
| Surface | `CreateImageSurface` | ❌ Stub (E_NOTIMPL) |

### 4.5. Texture Lifecycle

1. Игра вызывает `CreateTexture(w, h, levels, usage, format, pool)` → `MetalTexture8` создаёт `MTLTexture`.
2. `LockRect(level)` → выделяет staging buffer (`malloc`), возвращает ptr + pitch.
3. Игра пишет пиксели в staging buffer.
4. `UnlockRect(level)` → `[mtlTexture replaceRegion:withBytes:bytesPerRow:]`, `free()` staging.
5. `SetTexture(0, tex)` → сохраняет в `m_Textures[0]`.
6. При draw call: `setFragmentTexture:mtlTex atIndex:0`.

D3DX texture loading (`D3DXCreateTextureFromFileExA`):
- Вызывает `GetRenderDevice()->CreateTextureFromFile()` → `MacOSTexture` (ITexture).
- Оборачивает результат в `MetalTexture8(device, mtlTexture, format)`.

**Форматы (MetalFormatFromD3D):**
- `ARGB8/XRGB8` → `MTLPixelFormatBGRA8Unorm`
- `DXT1` → `MTLPixelFormatBC1_RGBA`
- `DXT3` → `MTLPixelFormatBC2_RGBA`
- `DXT5` → `MTLPixelFormatBC3_RGBA`
- 16-bit (R5G6B5, A1R5G5B5, etc.) → fallback BGRA8 (⚠️ нет конверсии содержимого!)

## 5. Шейдеры (MacOSShaders.metal)

**Vertex Shader (`vertex_main`):**
- Входы: `position` (attr 0), `color` (attr 1), `texCoord` (attr 2).
- Uniforms: `world`, `view`, `projection`, `screenSize`, `useProjection`, `shaderSettings`.
- Если `useProjection == 1`: `pos = projection * view * world * pos` (3D).
- Если `useProjection == 2`: screen space → NDC (`pos / screenSize * 2 - 1`).
- Fog factor: dummy `clamp(-viewPos.z / 500.0, 0, 1)`.

**Fragment Shader (`fragment_main`):**
- Sampled texture if `SHIFT_TEXTURING` bit set.
- `finalColor = texColor * in.color`.
- Alpha test discard if `SHIFT_ALPHATEST` bit set and alpha < 0.375.
- Primary gradient: MODULATE (default) or ADD.
- Fog: simple mix with fogColor(0.5, 0.5, 0.5).

**⚠️ Vertex мисмatch issue:**
- При `D3DFVF_XYZRHW` vertex descriptor использует `MTLVertexFormatFloat4` для attr[0],
  но шейдер объявляет `VertexIn.position` как `float3`. Это может вызвать проблемы.
  Нужна отдельная vertex function или `packed_float4` для XYZRHW.

## 6. Шпаргалка: Нереализованное

| Функциональность | Этап плана | Статус |
|:---|:---|:---|
| Depth/Stencil texture + state | Этап 10 | ❌ |
| Dynamic blending (per-RS) | Этап 6 | ❌ |
| CullMode binding | Этап 6 | ❌ |
| Multi-texturing (stage 1+) | Этап 7 | ❌ |
| TSS формулы в шейдере | Этап 7 | ❌ |
| Sampler states | Этап 7 | ❌ |
| Per-vertex lighting | Этап 8 | ❌ |
| Real fog params | Этап 9 | ❌ |
| Render targets | Этап 10 | ❌ |
| DrawPrimitiveUP | Этап 3 | ❌ |
| Surface (GetSurfaceLevel) | Этап 5 | ❌ |
| 16-bit format conversion | Этап 5 | ❌ |
| TriangleFan → TriangleList | Этап 3 | ❌ |
| MTLDevice shared (VB/IB) | Этап 2 | ⚠️ Uses MTLCreateSystemDefaultDevice() |
