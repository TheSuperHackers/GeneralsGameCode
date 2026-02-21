# План правок Metal-адаптера

> **На основе:** `SYSTEM_AUDIT.md` + `WINDOWS_FLOW_AUDIT.md`
> **Дата:** 2026-02-21
> **Принцип:** Фиксим то, что движок РЕАЛЬНО использует, в порядке влияния на визуал.

---

## Приоритет P0 — Прямо влияет на отображение (делать первым)

### Fix 1: Конвертация 16-bit текстур
**Файл:** `MetalTexture8.mm` → `UnlockRect()`  
**Проблема:** Движок по умолчанию создаёт текстуры с `DEFAULT_TEXTURE_BIT_DEPTH=16`.
Форматы R5G6B5, A1R5G5B5, A4R4G4B4, X1R5G5B5 создают 32-bit Metal-текстуру (BGRA8Unorm),
но данные загружаются как 16-bit → мусорные пиксели.  
**Решение:**
```
В UnlockRect(), перед replaceRegion:
  if (m_Format == D3DFMT_R5G6B5) {
      // Аллоцировать 32-bit буфер (width * height * 4)
      // Для каждого пикселя:
      //   uint16_t pixel = src[i]
      //   B = (pixel & 0x001F) << 3;  // 5 bit → 8 bit
      //   G = (pixel & 0x07E0) >> 3;  // 6 bit → 8 bit  
      //   R = (pixel & 0xF800) >> 8;  // 5 bit → 8 bit
      //   A = 0xFF;
      //   dst[i] = B | (G<<8) | (R<<16) | (A<<24)  // BGRA
      // replaceRegion с новым буфером и bytesPerRow = width * 4
  }
  // Аналогично для A1R5G5B5, A4R4G4B4, X1R5G5B5
```
**Объём:** ~80 строк  
**Влияние:** Все текстуры терейна, юнитов, UI в 16-bit режиме станут видимыми

---

### Fix 2: TextureOpCaps в D3DCAPS8
**Файл:** `MetalInterface8.mm` → `GetDeviceCaps()`  
**Проблема:** Движок проверяет `TextureOpCaps` перед КАЖДЫМ вызовом `SetTextureStageState`.
Без правильных caps → fallback-пути с ухудшенным качеством.  
**Решение:**
```cpp
caps.TextureOpCaps = 
    D3DTEXOPCAPS_DISABLE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2 |
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
    D3DPTADDRESSCAPS_WRAP | D3DPTADDRESSCAPS_MIRROR | D3DPTADDRESSCAPS_CLAMP;

caps.PrimitiveMiscCaps |= D3DPMISCCAPS_CULLCW | D3DPMISCCAPS_CULLCCW | 
    D3DPMISCCAPS_CULLNONE | D3DPMISCCAPS_BLENDOP;
    
caps.MaxTextureRepeat = 8192;
caps.MaxAnisotropy = 16;
caps.MaxPointSize = 256.0f;
```
**Объём:** ~20 строк  
**Влияние:** Движок получит корректные capabilities, перестанет использовать fallback-пути

---

### Fix 3: Рефакторинг дублированного кода Draw*
**Файл:** `MetalDevice8.mm`  
**Проблема:** Fragment uniforms, lighting uniforms, texture binding, fog — 
скопированы 3 раза в DrawPrimitive, DrawIndexedPrimitive, DrawPrimitiveUP (~150 строк × 3).  
**Решение:**
```cpp
// Новые приватные методы:
void MetalDevice8::BindUniforms(DWORD fvf);        // vertex + fragment uniforms
void MetalDevice8::BindTextures();                  // samplers + textures  
void MetalDevice8::BindLighting(DWORD fvf);         // lighting uniforms

// DrawPrimitive/DrawIndexedPrimitive/DrawPrimitiveUP вызывают их:
BindUniforms(fvf);
BindTextures();
BindLighting(fvf);
```
**Объём:** Перемещение ~150 строк в 3 метода, замена 3×50 строк на 3×3 вызова  
**Влияние:** Чистота кода, проще вносить дальнейшие фиксы. Убирает ~300 строк дублирования

---

## Приоритет P1 — Влияет на конкретные визуальные эффекты

### Fix 4: D3DTSS_TEXCOORDINDEX
**Файлы:** `MetalDevice8.mm` (передача в шейдер), `MacOSShaders.metal` (использование)  
**Проблема:** Движок активно перенаправляет UV через TEXCOORDINDEX:
- `TCI_PASSTHRU | 0` или `| 1` — выбор UV-сета из вершины
- `TCI_CAMERASPACEPOSITION` — генерация UV из позиции камеры
- `TCI_CAMERASPACENORMAL` — environment mapping  
- `TCI_CAMERASPACEREFLECTIONVECTOR` — reflections  

**Решение (этап 1 — UV-switching):**
```cpp
// В FragmentUniforms добавить:
uint32_t texCoordIndex[2];  // для каждой стадии

// В шейдере:
float2 getTexCoord(VertexOut in, uint tci) {
    uint source = tci & 0xFFFF;  // нижние 16 бит = индекс UV
    if (source == 0) return in.texCoord;
    if (source == 1) return in.texCoord2;
    return in.texCoord;
}
```
**Решение (этап 2 — texgen, позже):**
Генерация UV из камерного пространства в vertex shader (для environment maps).

**Объём:** ~30 строк  
**Влияние:** Корректный multi-texture UV routing, environment maps

---

### Fix 5: SetRenderTarget для render-to-texture
**Файл:** `MetalDevice8.mm`  
**Проблема:** SetRenderTarget — заглушка. Движок рендерит тени и эффекты в текстуры.  
**Решение:**
```
1. GetRenderTarget() — вернуть surface обёртку для текущего drawable
2. SetRenderTarget(colorSurface, depthSurface):
   a. Если surface != nullptr && surface != default:
      - endEncoding текущего encoder
      - Создать новый RPD с texture из surface
      - Создать новый encoder
   b. Если surface == nullptr:
      - Восстановить default drawable
3. Хранить DefaultRenderTarget / DefaultDepthBuffer
```
**Объём:** ~80 строк  
**Влияние:** Тени, водные отражения, render-to-texture эффекты

---

### Fix 6: Specular — условное добавление
**Файлы:** `MetalDevice8.mm` (передача флага), `MacOSShaders.metal` (условие)  
**Проблема:** Шейдер всегда добавляет specular к финальному цвету.
Движок по умолчанию ставит `D3DRS_SPECULARENABLE = FALSE`.
Хотя materialSpecular обычно (0,0,0,0), при lighting-вычислениях 
specular-компонент может быть ненулевым.  
**Решение:**
```metal
// В FragmentUniforms добавить:
uint32_t specularEnable;

// В fragment_main изменить ~строку 575:
if (fu.specularEnable) {
    current.rgb += specular.rgb;
}
```
```cpp
// В MetalDevice8, при сборке FragmentUniforms:
fu.specularEnable = m_RenderStates[D3DRS_SPECULARENABLE];
```
**Объём:** ~5 строк  
**Влияние:** Убирает паразитное осветление на non-specular объектах

---

### Fix 7: DXT2/DXT4 формат маппинг
**Файл:** `MetalTexture8.mm` → `MetalFormatFromD3D()`  
**Проблема:** DXT2 и DXT4 падают в default → BGRA8Unorm, что полностью ломает сжатые данные.  
**Решение:**
```cpp
case D3DFMT_DXT2:  // premultiplied alpha DXT3
    return MTLPixelFormatBC2_RGBA;
case D3DFMT_DXT4:  // premultiplied alpha DXT5
    return MTLPixelFormatBC3_RGBA;
```
**Объём:** 4 строки  
**Влияние:** Текстуры в DXT2/DXT4 (редко, но при наличии — полностью битые)

---

### Fix 8: Удаление отладочного логирования
**Файл:** `MetalDevice8.mm`  
**Проблема:** ~100 строк `fprintf(stderr, ...)`, `printf(...)`, `static int xxxCount` 
в BeginScene, Clear, DrawIndexedPrimitive, DrawPrimitiveUP.
Замедляет рендеринг, засоряет консоль.  
**Решение:** Заменить на DLOG_RFLOW макросы или удалить.  
**Объём:** Удаление/замена ~100 строк  
**Влияние:** Производительность, чистота логов

---

## Приоритет P2 — Улучшение качества / Edge cases

### Fix 9: BLENDTEXTUREALPHA для stage 1
**Файл:** `MacOSShaders.metal` → `evaluateBlendOp()`  
**Проблема:** Использует texColor0.a для обеих стадий, должен использовать texColor1.a 
для стадии 1. Движок использует BLENDTEXTUREALPHA на stage 1 для DETAILCOLOR_BLEND.  
**Решение:**
```metal
// Передать в evaluateBlendOp аргумент stageIndex и texColor
// Или передать массив texColors и использовать texColors[stageIndex].a
```
**Объём:** ~10 строк  
**Влияние:** Корректный блендинг деталь-текстур

---

### Fix 10: Mipmap auto-generation
**Файл:** `MetalTexture8.mm` → конструктор  
**Проблема:** `m_Levels=0` зажимается в 1. По спеке DX8 0 = все уровни.  
**Решение:**
```cpp
if (m_Levels == 0) {
    m_Levels = (UINT)std::floor(std::log2(std::max(width, height))) + 1;
}
```
**Объём:** 3 строки  
**Влияние:** Mipmapping для лучшего качества текстур на расстоянии

---

### Fix 11: GetDirect3D
**Файл:** `MetalDevice8.mm`  
**Проблема:** Возвращает nullptr. Движок вызывает это для получения IDirect3D8.  
**Решение:** Хранить указатель на MetalInterface8, возвращать его с AddRef().  
**Объём:** 5 строк  
**Влияние:** Корректность API

---

### Fix 12: D3DPT_TRIANGLEFAN конвертация
**Файл:** `MetalDevice8.mm` → DrawPrimitive / DrawIndexedPrimitive  
**Проблема:** Metal не поддерживает triangle fan. Если движок вызовет — геометрия пропадёт.  
**Решение:**
```cpp
if (pt == D3DPT_TRIANGLEFAN) {
    // Конвертировать fan в triangle list:
    // Для N вершин fan: создать (N-2)*3 индексов
    // [0,1,2], [0,2,3], [0,3,4], ...
}
```
**Примечание:** Нужно проверить, использует ли движок Generals TRIANGLEFAN.
Из кода Draw() — `D3DPT_TRIANGLELIST` основной тип. Fan может не встречаться.  
**Объём:** ~20 строк  
**Влияние:** Страховка на случай использования fan

---

## Приоритет P3 — Nice-to-have

### Fix 13: D3DRS_FILLMODE (wireframe)
Для дебаг-режима. `setTriangleFillMode:MTLTriangleFillModeLines`

### Fix 14: D3DRS_ZBIAS  
`setDepthBias:slopeScale:clamp:` для устранения z-fighting

### Fix 15: Texture coordinate generation (CAMERASPACEPOSITION и т.д.)
Этап 2 от Fix 4. Нужен для environment mapping.

### Fix 16: DrawIndexedPrimitiveUP
Stub → полная реализация (если найдутся вызовы).

### Fix 17: EnumAdapterModes
Вернуть реальные режимы дисплея через CGDisplayCopyAllDisplayModes.

---

## Порядок выполнения

```
Неделя 1: Fix 1 (16-bit текстуры) + Fix 2 (caps) + Fix 3 (рефакторинг)
Неделя 2: Fix 4 (TEXCOORDINDEX) + Fix 6 (specular) + Fix 7 (DXT2/4) + Fix 8 (логи)
Неделя 3: Fix 5 (SetRenderTarget) + Fix 9 (blend stage 1) + Fix 10 (mipmaps)
По мере необходимости: Fix 11-17
```

---

## Что НЕ нужно делать (подтверждено аудитом)

| Задача | Причина |
|:---|:---|
| D3DRS_BLENDOP | Движок не использует — хардкод ADD корректен |
| D3DTOP_MULTIPLYADD, LERP, PREMODULATE | Движок не использует эти операции |
| Vertex/Pixel Shader bytecode | Движок использует только FVF pipeline |
| 32-bit index buffers | Движок ограничен unsigned short |
| > 4 источников света | Движок использует 4 (lights[4]) |
| D3DBLEND_BOTHSRCALPHA | Движок не использует |
| State blocks (CreateStateBlock) | Движок не использует |
| D3DRS_POINTSPRITE | Движок не использует (частицы через triangles) |
| Volume textures | Движок не использует |

---

## Метрики успеха

| Этап | Визуальный результат |
|:---|:---|
| После Fix 1+2 | Все текстуры видны правильно, корректные цвета |
| После Fix 3+8 | Чистый код, быстрее рендеринг |
| После Fix 4+6+7+9 | Корректные multi-texture эффекты, env maps |
| После Fix 5 | Тени, render-to-texture эффекты |
| После Fix 10 | Качественные текстуры на расстоянии |
