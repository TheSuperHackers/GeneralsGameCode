# Windows Flow Audit — Как движок W3D реально работает с DX8

> **Цель:** Определить специфику ядра игры при работе с DX8 — уникальные паттерны, 
> инварианты и отчехи от стандарта, которые Metal-адаптер обязан реплицировать.
>
> **Источник:** `dx8wrapper.cpp` (4617 строк), `shader.cpp` (1263 строки), 
> `dx8caps.cpp` (1172 строки), `vertmaterial.cpp`, `mapper.cpp`

---

## 1. Архитектура: DX8Wrapper — промежуточный слой

Игра **НЕ** вызывает `IDirect3DDevice8` напрямую. Между движком и DX8 стоит 
`DX8Wrapper` — статический класс-обёртка, который:

```
Игровой код (Game.cpp)
    ↓
DX8Wrapper (dx8wrapper.h/cpp) — кэширование, батчинг, отслеживание изменений
    ↓
ShaderClass::Apply() — абстракция рендер-стейтов
    ↓
IDirect3DDevice8 (наш MetalDevice8)
```

### 🔑 Ключевые особенности:

1. **DX8Wrapper кэширует ВСЕ состояния** — `RenderStates[256]`, `TextureStageStates[8][32]`, 
   `DX8Transforms[]`. Он вызывает `SetRenderState`/`SetTextureStageState` **только при изменении**
   (redundant state detection).

2. **ShaderClass** — это НЕ GPU-шейдер, а **битовое поле** (unsigned long), кодирующее 
   комбинацию DX8 рендер-стейтов (depth, blend, fog, texturing, culling, post-detail).
   При `Apply()` оно транслируется в конкретные DX8 вызовы.

3. **Invalidate_Cached_Render_States()** заполняет кэш `0x12345678` при инициализации/ресете,
   что гарантирует повторную установку всех стейтов.

---

## 2. Уникальные паттерны движка

### 2.1 🚨 Все индексы и вершины — `unsigned short` (16-bit!)

```cpp
// dx8wrapper.h
void Draw(unsigned primitive_type,
    unsigned short start_index,
    unsigned short polygon_count,
    unsigned short min_vertex_index,
    unsigned short vertex_count);
```

**Движок ограничивает все draw-параметры 16 битами** — max 65535.
Это значит:
- Vertex buffer никогда не будет > 65535 вершин
- Index buffer всегда 16-bit `D3DFMT_INDEX16`
- `CreateVertexBuffer` в MetalDevice8 кастит к `unsigned short` — **это правильно!** 
  (мы ранее считали это багом, но движок реально ограничен 16 битами)

### 2.2 🚨 `SetVertexShader` используется для FVF, не для шейдеров

```cpp
// Apply_Render_State_Changes(), line 2468:
unsigned fvf = render_state.vertex_buffers[i]->FVF_Info().Get_FVF();
if (fvf != 0) {
    Set_Vertex_Shader(fvf);  // ← передаёт FVF как "шейдер"
}
```

DX8 использует `SetVertexShader()` для двух целей:
1. Установка FVF-кода (если handle < 0x10000)
2. Установка vertex shader handle (если >= 0x10000)

**Generals uses ONLY FVF** — никогда не создаёт настоящие DX8 vertex/pixel shaders.
Наша заглушка `CreateVertexShader → handle=0;` корректна.

### 2.3 🚨 Порядок вызовов: Clear() ПЕРЕД BeginScene()

```cpp
// WW3D pipeline:
DX8Wrapper::Clear(...)        // ← вызывает IDirect3DDevice8::Clear()
DX8Wrapper::Begin_Scene(...)  // ← вызывает IDirect3DDevice8::BeginScene()
// ... draw calls ...
DX8Wrapper::End_Scene(true)   // ← EndScene() + Present()
```

**Движок вызывает Clear() ДО BeginScene()!** Это нестандартно для DX8, но допустимо.  
Наш MetalDevice8 уже обрабатывает это (auto-BeginScene в Clear, строка 1150).

### 2.4 🚨 End_Scene сбрасывает ВСЁ

```cpp
void DX8Wrapper::End_Scene(bool flip_frames) {
    DX8CALL(EndScene());
    // ... Present если flip_frames ...
    
    // КАЖДЫЙ КАДР:
    Set_Vertex_Buffer(nullptr);
    Set_Index_Buffer(nullptr, 0);
    for (int i = 0; ...) Set_Texture(i, nullptr);
    Set_Material(nullptr);
}
```

**Каждый кадр** обнуляются все текстуры, VB, IB, материал. Это значит:
- Наш адаптер НЕ может полагаться на persistence стейтов между кадрами
- Всё пересоздаётся/перебиндится на следующем кадре

### 2.5 🚨 `D3DBLENDOP` НЕ ИСПОЛЬЗУЕТСЯ движком

Анализируя `ShaderClass::Apply()` и все preset-шейдеры:
- Движок устанавливает только `D3DRS_SRCBLEND` и `D3DRS_DESTBLEND`
- **Нигде не устанавливается `D3DRS_BLENDOP`** — всегда используется дефолт `D3DBLENDOP_ADD`
- Наш хардкод `MTLBlendOperationAdd` — **корректен для Generals!**

### 2.6 🚨 Ограниченный набор blend-комбинаций

Из preset-шейдеров движка, используются ТОЛЬКО эти комбинации:

| Preset | SrcBlend | DstBlend | Описание |
|:---|:---|:---|:---|
| Opaque | ONE | ZERO | Непрозрачный |
| Additive | ONE | ONE | Аддитивный |
| Alpha | SRC_ALPHA | INV_SRC_ALPHA | Альфа-блендинг |
| Multiplicative | ZERO | SRC_COLOR | Мультипликативный |
| Screen | ONE | INV_SRC_COLOR | Screen-эффект |
| BumpEnvMap | ONE | ONE | Bump map (аддитивный) |
| (Custom) | INV_SRC_ALPHA | SRC_ALPHA | Обратный альфа |
| (Custom) | DEST_COLOR | — | Вершинный цвет |

**Движок НИКОГДА не использует:** `D3DBLEND_BOTHSRCALPHA`, `D3DBLEND_BOTHINVSRCALPHA`,
`D3DBLEND_DESTALPHA`, `D3DBLEND_INVDESTALPHA`.

---

## 3. ShaderClass::Apply() — Как движок устанавливает TSS

**Это КРИТИЧЕСКИ важная функция!** Она определяет, какие D3DTOP используются реально.

### 3.1 Stage 0 — Primary Gradient

Движок использует ТОЛЬКО эти комбинации для stage 0:

| Gradient Mode | ColorOp | Args | AlphaOp | Args |
|:---|:---|:---|:---|:---|
| DISABLE (Decal) | SELECTARG1 | TEX, _ | SELECTARG1 | TEX, _ |
| MODULATE | MODULATE | TEX, DIFFUSE | MODULATE | TEX, DIFFUSE |
| ADD | ADD | TEX, DIFFUSE | MODULATE | TEX, DIFFUSE |
| MODULATE2X | MODULATE2X | TEX, DIFFUSE | MODULATE | TEX, DIFFUSE |
| BUMPENVMAP | BUMPENVMAP | TEX, DIFFUSE | DISABLE | — |
| BUMPENVMAPLUMINANCE | BUMPENVMAPLUMINANCE | TEX, DIFFUSE | DISABLE | — |
| (No texture) DISABLE | DISABLE | — | DISABLE | — |
| (No texture) MODULATE | SELECTARG2 | _, DIFFUSE | SELECTARG2 | _, DIFFUSE |

### 3.2 Stage 1 — Post-Detail (Secondary)

Движок использует ТОЛЬКО эти D3DTOP для stage 1:

| Detail Mode | D3DTOP | Notes |
|:---|:---|:---|
| DISABLE | D3DTOP_DISABLE | Стадия отключена |
| DETAIL | D3DTOP_SELECTARG1 | Только текстура |
| SCALE | D3DTOP_MODULATE | Текстура × текущий |
| INVSCALE | D3DTOP_ADDSMOOTH (→ fallback ADD) | Текстура + текущий - тек×тек |
| ADD | D3DTOP_ADD | Текстура + текущий |
| SUB | D3DTOP_SUBTRACT | Текстура - текущий |
| SUBR | D3DTOP_SUBTRACT (reversed args!) | Текущий - текстура |
| BLEND | D3DTOP_BLENDTEXTUREALPHA | Mix по альфе текстуры |
| DETAILBLEND | D3DTOP_BLENDCURRENTALPHA | Mix по альфе текущего |
| ADDSIGNED | D3DTOP_ADDSIGNED | Текстура + текущий - 0.5 |
| ADDSIGNED2X | D3DTOP_ADDSIGNED2X | (Текстура + текущий - 0.5) × 2 |
| SCALE2X | D3DTOP_MODULATE2X | Текстура × текущий × 2 |
| MODALPHAADDCOLOR | D3DTOP_MODULATEALPHA_ADDCOLOR | RGB=текущий.rgb + текущий.a × тек2.rgb |

**Все аргументы stage 1 — `D3DTA_TEXTURE` и `D3DTA_CURRENT`.** 
Движок НИКОГДА не использует `D3DTA_TFACTOR` или `D3DTA_SPECULAR` в TSS на stage 1!

### 3.3 D3DTOP которые движок НЕ использует

Следующие операции определены в DX8: spec, но **НИКОГДА не устанавливаются движком**:
- `D3DTOP_MODULATE4X` (хотя реализован в нашем шейдере — OK)
- `D3DTOP_BLENDDIFFUSEALPHA`
- `D3DTOP_BLENDFACTORALPHA`  
- `D3DTOP_MODULATECOLOR_ADDALPHA`
- `D3DTOP_MODULATEINVALPHA_ADDCOLOR`
- `D3DTOP_MODULATEINVCOLOR_ADDALPHA`
- `D3DTOP_DOTPRODUCT3`
- `D3DTOP_MULTIPLYADD`
- `D3DTOP_LERP`
- `D3DTOP_PREMODULATE`

→ Мы можем **снизить приоритет** отсутствующих D3DTOP в нашем шейдере.

---

## 4. Система тумана — 3 режима!

Движок имеет **кастомную** систему тумана через `ShaderClass::FOG_*`:

```cpp
enum FogFuncType {
    FOG_DISABLE,          // D3DRS_FOGENABLE = FALSE
    FOG_ENABLE,           // D3DRS_FOGENABLE = TRUE, обычный цвет
    FOG_SCALE_FRAGMENT,   // D3DRS_FOGENABLE = TRUE, fogColor = 0x000000 (чёрный)
    FOG_WHITE,            // D3DRS_FOGENABLE = TRUE, fogColor = 0xFFFFFF (белый)
};
```

**`FOG_SCALE_FRAGMENT`** — уникальный паттерн! Для аддитивных объектов (src=ONE, dst=ONE) 
обычный туман не работает (объект должен затухать, а не смешиваться с цветом тумана).
Решение: fogColor = чёрный (0), тогда `mix(0, fragment, fogFactor) = fragment * fogFactor` — 
фрагмент просто масштабируется.

**`FOG_WHITE`** — для мультипликативных объектов (src=ZERO, dst=SRC_COLOR). Туман = белый 
означает объект исчезает к белому (нейтральный для умножения).

→ **Наш Metal-адаптер уже обрабатывает это корректно** через передачу fogColor.

---

## 5. Clear() — Проверяет формат depth buffer

```cpp
void DX8Wrapper::Clear(...) {
    // Получает текущий depth buffer
    _Get_D3D_Device8()->GetDepthStencilSurface(&depthbuffer);
    
    // Проверяет формат для stencil
    depthbuffer->GetDesc(&desc);
    has_stencil = (desc.Format == D3DFMT_D15S1 || 
                   desc.Format == D3DFMT_D24S8 ||
                   desc.Format == D3DFMT_D24X4S4);
    depthbuffer->Release();
    
    // Стенсил очищается ТОЛЬКО если формат поддерживает
    if (clear_z_stencil && has_stencil)
        flags |= D3DCLEAR_STENCIL;
}
```

**Важно:** Движок вызывает `GetDepthStencilSurface()` и `GetDesc()` каждый Clear().
Наш MetalDevice8 должен:
1. Возвращать корректную surface из `GetDepthStencilSurface()`
2. Surface::GetDesc() должен возвращать формат совместимый со стенсилом 
   (наш `Depth32Float_Stencil8` совместим — ✅)

---

## 6. Render-to-Texture — АКТИВНО используется!

```cpp
// Shadows, reflections, etc:
DX8Wrapper::Set_Render_Target_With_Z(texture, ztexture);
// ... draw shadow receivers ...
DX8Wrapper::Set_Render_Target(nullptr);  // restore default
```

Движок вызывает:
1. `GetRenderTarget()` → сохраняет DefaultRenderTarget
2. `GetDepthStencilSurface()` → сохраняет DefaultDepthBuffer
3. `SetRenderTarget(newSurface, depthSurface)` → переключает RT
4. Рисует сцену
5. `SetRenderTarget(DefaultRenderTarget, DefaultDepthBuffer)` → восстанавливает

**Наш `SetRenderTarget` — заглушка!** Это значит:
- Тени не работают
- Некоторые визуальные эффекты отсутствуют
- Водные отражения не рендерятся

### Что нужно реализовать:
- `GetRenderTarget()` → возвращает поверхность текущего drawable
- `SetRenderTarget(surface, depth)` → меняет color/depth attachment 
  для следующего render pass

---

## 7. Текстуры — Паттерн `D3DXCreateTexture`

```cpp
// Движок использует D3DX функцию:
D3DXCreateTexture(device, width, height, mip_levels, 
                  usage, format, pool, &texture);
```

`D3DXCreateTexture` (НЕ `CreateTexture`) автоматически:
1. **Подбирает ближайший поддерживаемый формат** если указанный не поддерживается
2. Корректирует размер до power-of-two если GPU требует
3. Генерирует mipmap levels

**Наш `CreateTexture` вызывается из заглушки `D3DXCreateTexture` в `D3DXStubs.cpp`.**
Это означает: формат может быть уже сконвертирован до вызова CreateTexture.

### DEFAULT_TEXTURE_BIT_DEPTH = 16!

```cpp
const int DEFAULT_TEXTURE_BIT_DEPTH = 16;
```

**По умолчанию движок использует 16-битные текстуры!** Форматы R5G6B5 и A4R4G4B4 будут 
встречаться часто. Конвертация 16→32 бит в MetalTexture8 — **критический путь**.

---

## 8. D3DTSS_TEXCOORDINDEX — АКТИВНО используется!

```cpp
// mapper.cpp — текстурные координаты:
Set_DX8_Texture_Stage_State(Stage, D3DTSS_TEXCOORDINDEX, 
    D3DTSS_TCI_PASSTHRU | uv_array_index);           // UV из вершины
Set_DX8_Texture_Stage_State(Stage, D3DTSS_TEXCOORDINDEX, 
    D3DTSS_TCI_CAMERASPACEPOSITION);                  // Позиция камеры
Set_DX8_Texture_Stage_State(Stage, D3DTSS_TEXCOORDINDEX, 
    D3DTSS_TCI_CAMERASPACENORMAL);                    // Нормаль камеры
Set_DX8_Texture_Stage_State(Stage, D3DTSS_TEXCOORDINDEX, 
    D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);          // Вектор отражения

// vertmaterial.cpp:
Set_DX8_Texture_Stage_State(i, D3DTSS_TEXCOORDINDEX, 
    D3DTSS_TCI_PASSTHRU | UVSource[i]);  // UVSource может быть 0 или 1
```

**Движок активно перенаправляет UV-координаты!** Используются:
1. `PASSTHRU | index` — использовать UV set `index` из вершины (может быть 0 или 1)
2. `CAMERASPACEPOSITION` — генерация UV из позиции в пространстве камеры
3. `CAMERASPACENORMAL` — генерация UV из нормали (environment mapping)  
4. `CAMERASPACEREFLECTIONVECTOR` — отражения

**Наш Metal-адаптер это полностью игнорирует!** Критично для:
- Текстурного маппинга окружения (environment maps)
- Корректного выбора UV-сета для multi-texturing
- Планарных проекций

---

## 9. CopyRects — используется для скриншотов

```cpp
// dx8wrapper.h (inline):
DX8CALL(CopyRects(
    src_surface, &rect, 1, dest_surface, &dest_pt));
```

Используется для копирования поверхностей (backbuffer → systemem surface для скриншотов).
Не критично для рендеринга, но нужно для функции скриншотов.

---

## 10. Caps-зависимые пути кода

```cpp
void ShaderClass::Apply() {
    unsigned int TextureOpCaps = 
        DX8Wrapper::Get_Current_Caps()->Get_DX8_Caps().TextureOpCaps;
    
    // Проверяет КАЖДУЮ операцию через TextureOpCaps:
    if (TextureOpCaps & D3DTEXOPCAPS_ADD) {
        SeccOp = D3DTOP_ADD;
    } else {
        // fallback...
    }
}
```

**Движок проверяет `TextureOpCaps` перед каждой TSS-операцией!**
Если мы не возвращаем правильные caps, движок будет использовать fallback-пути 
(менее качественные эффекты).

### Что нужно добавить в D3DCAPS8:

```cpp
caps.TextureOpCaps = 
    D3DTEXOPCAPS_DISABLE |
    D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2 |
    D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_MODULATE2X | D3DTEXOPCAPS_MODULATE4X |
    D3DTEXOPCAPS_ADD | D3DTEXOPCAPS_ADDSIGNED | D3DTEXOPCAPS_ADDSIGNED2X |
    D3DTEXOPCAPS_SUBTRACT | D3DTEXOPCAPS_ADDSMOOTH |
    D3DTEXOPCAPS_BLENDDIFFUSEALPHA | D3DTEXOPCAPS_BLENDTEXTUREALPHA |
    D3DTEXOPCAPS_BLENDFACTORALPHA | D3DTEXOPCAPS_BLENDCURRENTALPHA |
    D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR |
    D3DTEXOPCAPS_DOTPRODUCT3;

caps.TextureFilterCaps = 
    D3DPTFILTERCAPS_MINFPOINT | D3DPTFILTERCAPS_MINFLINEAR |
    D3DPTFILTERCAPS_MAGFPOINT | D3DPTFILTERCAPS_MAGFLINEAR |
    D3DPTFILTERCAPS_MIPFPOINT | D3DPTFILTERCAPS_MIPFLINEAR;

caps.TextureAddressCaps =
    D3DPTADDRESSCAPS_WRAP | D3DPTADDRESSCAPS_MIRROR | 
    D3DPTADDRESSCAPS_CLAMP;
```

---

## 11. DX8Caps::Compute_Caps — GPU-specific workarounds

Движок имеет специальные ветки для Voodoo3, старых ATI, NVidia и т.д.
Для Metal мы должны:
1. Возвращать vendor = Unknown / Apple
2. **НЕ попадать в Voodoo3 path** (stage-swapping hack)
3. Указать максимальные capabilities

---

## 12. Alpha Test — Хардкодированные значения!

```cpp
// shader.cpp, ShaderClass::Apply():
unsigned char alphareference = 0x60;  // 96/255 ≈ 0.376

if (sf == D3DBLEND_INVSRCALPHA) {
    Set_DX8_Render_State(D3DRS_ALPHAREF, 0xff - alphareference);  // 159
    Set_DX8_Render_State(D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL);
} else {
    Set_DX8_Render_State(D3DRS_ALPHAREF, alphareference);  // 96
    Set_DX8_Render_State(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
}
```

**Alpha reference всегда 0x60 (96)** или его инверсия 0x9F (159).
Alpha func — только `GREATEREQUAL` или `LESSEQUAL`.
Наш шейдер поддерживает все функции — это **правильно и избыточно**.

---

## 13. Device Lost / Reset Flow

```cpp
// End_Scene:
if (hr == D3DERR_DEVICELOST) {
    hr = TestCooperativeLevel();
    if (hr == D3DERR_DEVICENOTRESET) {
        Reset_Device();
    } else {
        Sleep(200);  // ← заскипь кадр и подожди
    }
}
```

Движок проверяет device lost на каждом Present(). На macOS это невозможно 
(Metal devices не "теряются"), поэтому:
- `TestCooperativeLevel()` → всегда D3D_OK ✅
- `Reset()` → нужно реализовать для resize/fullscreen toggle

---

## 14. Сводная таблица: Что движок РЕАЛЬНО вызывает

### IDirect3DDevice8 методы, требующие полной реализации:

| Метод | Приоритет | Текущий статус |
|:---|:---:|:---:|
| `BeginScene` / `EndScene` | P0 | ✅ |
| `Present` | P0 | ✅ |
| `Clear` | P0 | ✅ |
| `SetTransform` / `GetTransform` | P0 | ✅ |
| `SetRenderState` / `GetRenderState` | P0 | ✅ |
| `SetTextureStageState` / `GetTextureStageState` | P0 | ✅ |
| `SetTexture` / `GetTexture` | P0 | ✅ |
| `SetStreamSource` | P0 | ✅ |
| `SetIndices` | P0 | ✅ |
| `SetVertexShader` (FVF only) | P0 | ✅ |
| `SetViewport` / `GetViewport` | P0 | ✅ |
| `SetMaterial` / `GetMaterial` | P0 | ✅ |
| `SetLight` / `LightEnable` | P0 | ✅ |
| `DrawIndexedPrimitive` | P0 | ✅ |
| `DrawPrimitive` | P0 | ✅ |
| `DrawPrimitiveUP` | P0 | ✅ |
| `CreateTexture` | P0 | ✅ |
| `CreateVertexBuffer` | P0 | ✅ |
| `CreateIndexBuffer` | P0 | ✅ |
| `ValidateDevice` | P0 | ✅ |
| `GetDeviceCaps` | P0 | ⚠️ неполные caps |
| `GetDepthStencilSurface` | P0 | ✅ |
| `GetRenderTarget` | P1 | ✅ (default) |
| `SetRenderTarget` | P1 | ❌ заглушка! |
| `TestCooperativeLevel` | P1 | ✅ |
| `GetBackBuffer` | P2 | ❌ |
| `CreateImageSurface` | P2 | ✅ |
| `CopyRects` | P2 | ❌ |
| `Reset` | P2 | ❌ |

---

## 15. 🔴 Критические фиксы для Metal-адаптера (по результатам аудита)

### P0 — Немедленно влияют на визуал

1. **Конвертация 16-bit текстур** — DEFAULT_TEXTURE_BIT_DEPTH=16, 
   поэтому R5G6B5 и A4R4G4B4 — основные форматы! 
   
2. **D3DTSS_TEXCOORDINDEX** — движок активно перенаправляет UV.
   Без этого environment maps и multi-texture UV-switching не работают.

3. **D3DCAPS8.TextureOpCaps** — движок проверяет caps перед установкой TSS.
   Без правильных caps будут использоваться fallback-пути.

### P1 — Влияют на конкретные визуальные эффекты

4. **SetRenderTarget** — нужен для теней и render-to-texture эффектов.

5. **FOG_SCALE_FRAGMENT / FOG_WHITE** — кастомные fog-режимы для 
   аддитивных/мультипликативных блендов. 
   **Уже работает** — fogColor правильно передаётся.

6. **BUMPENVMAP** — bump mapping через TSS. Низкий приоритет 
   (используется редко).

### P2 — Не критичные

7. **D3DBLENDOP** — подтверждено: движок НЕ использует, наш хардкод ADD корректен.
8. **D3DPT_TRIANGLEFAN** — нужно проверить, использует ли движок.
9. **D3DRS_SPECULARENABLE** — по умолчанию FALSE в движке. 
   Шейдер всегда добавляет specular — **может вызывать лёгкое осветление**,
   но т.к. specularSource=MATERIAL и materialSpecular по умолчанию (0,0,0,0), 
   эффект минимален.
