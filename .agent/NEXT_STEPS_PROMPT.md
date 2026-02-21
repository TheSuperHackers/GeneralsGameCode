# Фаза: Fixing Crash After Object Loading

Привет, следующий агент! Мы портируем **Command & Conquer Generals (Zero Hour)** на macOS через Metal API.

## Текущее состояние (2026-02-21)

**� Исправлена корневая причина отсутствия 3D объектов!** Фабричный метод `createGameLogic()` создавал `GameLogic` вместо `W3DGameLogic`, из-за чего `createTerrainLogic()` возвращал обычный `TerrainLogic` вместо `W3DTerrainLogic`. Это приводило к тому, что `WorldHeightMap` НЕ парсил `ObjectsList` чанк из `.map` файла → 0 объектов в мире.

**Исправление** (одна строка в `MacOSMain.mm:271`):
```cpp
// БЫЛО (неправильно):
return new GameLogic();
// СТАЛО (правильно):
return NEW W3DGameLogic();
```

Теперь: **771 MapObject загружается**, объекты создаются (`Infa_ChinaTankGattling`, `Lazr_AmericaTankCrusader`, `ChainLinkFence03` и т.д.), terrain загружается через `W3DTerrainLogic` с правильными данными высот.

### ✅ Что работает

1. **W3DGameLogic** — правильный фабричный метод, `W3DTerrainLogic` + `W3DGhostObjectManager`
2. **MapObject loading** — 771 объект парсится из shell map через `WorldHeightMap`
3. **Object creation** — `ThingFactory::newObject()` вызывается, объекты создаются
4. **3D terrain рендеринг** — terrain видимый с vertex diffuse lighting цветами
5. **2D UI** — кнопки, текст, диалоги рендерятся через TSS pipeline
6. **Shell map** — 3D фон загружается, main menu (`MainMenu.wnd`) загружается
7. **Frame lifecycle** — рендерится ~86 кадров, потом загружается меню

### 🔴 Приоритет 1: SIGSEGV (exit code 139) после загрузки меню

**Симптом:** Игра успешно загружает shell map, рендерит 86 кадров (fps от 5000 до 300), загружает `MainMenu.wnd`, но потом крашится с SIGSEGV.

**Последние строки лога перед крашем:**
```
SHELLMAP: showShellMap(1) shellMapOn=1 initialFile='' gameLogic=0x750455980
SHELLMAP: already in shell game, return
TRANSITION: reverse('FadeWholeScreen') found=0x7502f2a60
[processCommandList] msg type=1097 (MSG_NEW_GAME=28)
[processCommandList] msg type=1 (MSG_NEW_GAME=28)
DEBUG: Pump heartbeat #51, isActive=0, keyWin=0x0
```

**Как дебажить:**
1. `lldb build/macos/GeneralsMD/generalszh` → `run -quick` → дождаться краша → `bt` для backtrace
2. Проверить: может быть рендеринг объектов (W3D models), или transition effect, или что-то с GhostObjectManager
3. Возможно что-то с `isActive=0` — окно теряет фокус?

### 🔴 Приоритет 2: TSS Pipeline для 3D draws — MODULATE даёт ноль

**Симптом:** `MODULATE(texColor0=white, diffuse)` должен давать `diffuse`, но post-TSS `current` = (0,0,0).

В `MacOSShaders.metal` fragment_main() есть **bypass workaround** — для 3D draws (`useProjection == 1`) мы возвращаем `float4(diffuse.rgb, 1.0)` напрямую вместо TSS pipeline. Это потому что полный TSS pipeline даёт zero output для 3D draws.

```metal
// В fragment_main():
if (uniforms.useProjection == 1) {
    return float4(diffuse.rgb, 1.0);  // TEMPORARY WORKAROUND
}
```

**Что НЕ проверено:**
- Struct alignment между `simd::float4` (CPU) и `float4` (GPU)
- Buffer binding conflict — `setFragmentBytes` index 1
- Metal shader compiler optimizations

### 🔴 Приоритет 3: Terrain Texture Data пустая

TEX pass texture содержит zero data. Мультипликативный blend `srcBlend=ZERO, dstBlend=SRCCOLOR` даёт чёрный.

### 🟡 Приоритет 4: 2D Texture Colors

2D статические текстуры имеют неправильные цвета (чёрно-бело-зелёные). Проблема pixel format mapping DDS→Metal.

## Диагностические логи (МОЖНО УДАЛИТЬ)

В коде остались `printf` логи для отладки, которые можно убрать при cleanup:

| Файл | Что логирует |
|:---|:---|
| `GameEngine.cpp:1081-1091` | `GameEngine::update()` — canUpdateLogic, halted, frozen (первые 10) |
| `GameLogic.cpp:1102-1108` | `startNewGame()` — loadingSave, gameMode, mapName |
| `GameLogic.cpp:1298-1303` | `startNewGame()` — loadMap done |
| `GameLogic.cpp:1827-1833` | `startNewGame()` — MapObject count |
| `GameLogic.cpp:2576-2585` | `processCommandList()` — message types |
| `ThingFactory.cpp:310-315` | `newObject()` — первые 5 created objects |

## Terrain Rendering Pipeline (DX8)

Terrain рисуется в 2 прохода за кадр:

1. **BASE pass** (16 draws): `alphaB=0, ZFunc=LESSEQUAL`
   - Без текстуры (`tex0=NULL`)
   - TSS: `MODULATE(TEXTURE, DIFFUSE)` → с tex=white = diffuse
   - Записывает diffuse цвет в framebuffer + Z-buffer
   
2. **TEX pass** (16 draws): `alphaB=1, srcBlend=ZERO, dstBlend=SRCCOLOR, ZFunc=EQUAL`
   - С текстурой (`tex0=terrain_tile`)
   - TSS: `SELECTARG1(TEXTURE)`
   - Модулирует framebuffer текстурой: `result = framebuffer × texColor`

## Ключевые файлы

| Файл | Что в нём |
|:---|:---|
| `Platform/MacOS/Source/Main/MacOSMain.mm` | **Factory methods** (createGameLogic → W3DGameLogic), Win32GameEngine stubs |
| `Platform/MacOS/Source/Main/MacOSShaders.metal` | Vertex + Fragment шейдер, TSS evaluation, 3D bypass workaround |
| `Platform/MacOS/Source/Metal/MetalDevice8.mm` | Metal rendering: BeginScene, Present, Draw*, PSO, transforms |
| `Platform/MacOS/Source/Metal/MetalSurface8.mm` | Surface lock/unlock, parent texture upload |
| `Platform/MacOS/Source/Metal/MetalTexture8.mm` | Texture creation, format mapping |
| `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameLogic/W3DTerrainLogic.cpp` | W3D terrain logic — loadMap с height data |
| `Core/GameEngineDevice/Source/W3DDevice/GameClient/WorldHeightMap.cpp` | Height map + MapObject чтение из .map файла |
| `GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp` | startNewGame(), processCommandList(), object creation loop |

## Сборка и запуск

```bash
sh build_run_mac.sh
```
