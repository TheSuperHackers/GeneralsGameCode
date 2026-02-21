# Фаза: Audio & Texture Polish

Привет, следующий агент! Мы портируем **Command & Conquer Generals (Zero Hour)** на macOS через Metal API.

## Текущее состояние (2026-02-22)

**🎉 MAJOR MILESTONE: Игра работает!** Главное меню с shell map рендерится, катсцены (cutscenes) играются, миссии загружаются с 3D юнитами, зданиями и ландшафтом. Game loop стабилен — 5500+ итераций без крашей.

### ✅ Что работает

1. **Game loop** — стабильный, без зависаний и крашей
2. **Shell map** — 3D анимированный фон (корабли, вода, взрывы)
3. **Main menu** — кнопки, навигация, все меню загружаются
4. **Cutscenes** — видео сцены перед миссиями проигрываются
5. **Mission loading** — карты загружаются, юниты и здания создаются
6. **3D rendering** — terrain, модели, тени, частично текстуры
7. **2D UI** — HUD, миникарта, командная панель, текст
8. **Keyboard/Mouse** — ввод работает
9. **`applicationShouldTerminate:`** — macOS не убивает процесс
10. **Signal handlers** — SIGSEGV/SIGBUS/SIGABRT дают backtrace через sigaction

### 🔴 Приоритет 1: Звук не работает

**Симптом:** Нет звуков — ни музыки, ни эффектов, ни голосов.

**Причина:** `MacOSAudioManager::processRequestList()` заstubлен — мы вынуждены были отключить обработку audio requests из-за SIGSEGV в `AsciiString::str()`. Corrupted/dangling `AudioEventRTS` pointers в request list.

**Как починить:**
1. Разобраться, почему `AudioEventRTS` pointers corrupted. Вероятно, объект удаляется игровой логикой, но pointer остается в request queue.
2. Возможно `allocateAudioRequest()` возвращает request с невалидным `m_pendingEvent`.
3. Нужно проверить ownership — кто создаёт и кто удаляет AudioEventRTS объекты.
4. Референс: `GeneralsMD/Code/GameEngine/Include/Common/AudioRequest.h` — структура AudioRequest.

**Файлы:**
- `Platform/MacOS/Source/Audio/MacOSAudioManager.mm` — наш stub
- `GeneralsMD/Code/GameEngine/Include/Common/AudioManager.h` — base class
- `GeneralsMD/Code/GameEngine/Source/Common/Audio/AudioManager.cpp` — base implementation

### 🔴 Приоритет 2: Белые текстуры на 3D объектах

**Симптом:** Здания и многие юниты рендерятся полностью белыми. Terrain нормальный, UI нормальный.

**Причина:** TSS pipeline не правильно применяет текстуры к 3D объектам. `MODULATE(texture, diffuse)` должен давать текстурированный объект, но даёт белый.

**Как починить:**
1. Проверить, загружены ли текстуры для 3D моделей (W3D format)
2. Разобраться с texture stage states для 3D draw calls
3. Возможно проблема в `D3DTOP_MODULATE` или `D3DTOP_SELECTARG1` в Metal шейдере
4. Проверить vertex color override — может diffuse всегда белый

**Файлы:**
- `Platform/MacOS/Source/Metal/MetalDevice8.mm` — DrawIndexedPrimitive + TSS
- `Platform/MacOS/Source/Metal/MacOSShaders.metal` — пиксельный шейдер

### 🟡 Приоритет 3: Crash при выходе из миссии

Exit code 139 (SIGSEGV) при завершении игры. Скорее всего cleanup/dealloc проблема.

## Ключевые технические детали

### Исправленные проблемы (эта сессия)
1. **SIGSEGV в MacOSAudioManager::processRequestList()** — corrupted AudioEventRTS pointers → stub
2. **Automatic Termination** — macOS убивал "idle" процесс → `disableAutomaticTermination`, `applicationShouldTerminate:` → NSTerminateCancel
3. **FramePacer null deref** — `TheScriptEngine` и `TheGlobalData` null checks добавлены
4. **displaySyncEnabled** → NO для предотвращения блокировки nextDrawable

### Архитектура
- **MetalDevice8** — реализация IDirect3DDevice8 поверх Metal API
- **MacOSAudioManager** — реализация AudioManager (пока stub)
- **MacOSWindowManager** — Cocoa NSWindow + event pump
- **Signal handlers** — sigaction-based crash reporter с backtrace
- **Frame pacing** — FramePacer контролирует FPS, displaySyncEnabled=NO

### Команды
```bash
# Собрать
cmake --build build/macos

# Запустить
GENERALS_INSTALL_PATH="/path/to/game/" GENERALS_FPS_LIMIT=60 build/macos/GeneralsMD/generalszh -quick

# Или скрипт
sh build_run_mac.sh
```
