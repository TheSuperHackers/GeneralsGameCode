# macOS CMake Integration Plan

## Цель
Единая система сборки через CMake. Одна команда `cmake --preset macos && cmake --build --preset macos`.

## Архитектура (3 слоя)

```
┌──────────────────────────────────────────┐
│ CMakePresets.json (macos preset)         │  ← Точка входа
├──────────────────────────────────────────┤
│ Root CMakeLists.txt                      │  ← Условия: APPLE → skip dx8/miles/bink,
│                                          │     подключить Platform/MacOS
├──────────────────────────────────────────┤
│ Platform/MacOS/CMakeLists.txt            │  ← Статическая библиотека macos_platform:
│   ├── Source/**/*.mm, *.cpp              │     - Metal рендер
│   ├── Include/ (shim headers)            │     - Cocoa окна
│   └── Frameworks: Metal, Cocoa, etc.     │     - Stub хедеры (windows.h и др.)
├──────────────────────────────────────────┤
│ GeneralsMD/Code/Main/CMakeLists.txt      │  ← APPLE: MacOSMain.mm вместо WinMain.cpp
│                                          │     link macos_platform вместо d3d8/winmm/etc
└──────────────────────────────────────────┘
```

## Что НЕЛЬЗЯ трогать
- Файлы в `GeneralsMD/Code/`, `Core/`, `Generals/` — НЕ МОДИФИЦИРУЕМ.
  Все совместимости решаются через include-path и shim-хедеры в Platform/MacOS/Include/.

## Шаг 1: Stub-хедеры в Platform/MacOS/Include/

PreRTS.h включает Windows-хедеры. Мы НЕ модифицируем PreRTS.h.
Вместо этого наш `-IPlatform/MacOS/Include` стоит ПЕРЕД `-IDependencies/dx8`,
и компилятор находит наши shim-файлы первыми.

Нужно создать stubs для:
- [x] windows.h        (уже есть)
- [x] malloc.h          (уже есть)
- [x] d3d8.h и d3d8*.h  (уже есть, ведут к d3d8_stub)
- [ ] atlbase.h          → пустой или minimal stub (ATL не нужен на macOS)
- [ ] direct.h           → stub
- [ ] excpt.h            → stub  
- [ ] imagehlp.h         → stub
- [ ] io.h               → stub
- [ ] lmcons.h           → stub
- [ ] mmsystem.h         → stub (timeGetTime и др.)
- [ ] objbase.h          → stub (COM)
- [ ] ocidl.h            → stub
- [ ] process.h          → stub (_beginthread и др.)
- [ ] shellapi.h         → stub
- [ ] shlobj.h           → stub
- [ ] shlguid.h          → stub
- [ ] snmp.h             → stub
- [ ] dinput.h           → stub (keyboard/mouse через Cocoa)
- [ ] tchar.h            → forward to Utility/tchar_compat.h

## Шаг 2: Platform/MacOS/CMakeLists.txt

```cmake
add_library(macos_platform STATIC
    Source/Main/MacOSMain.mm
    Source/Main/MacOSRenderer.mm
    Source/Main/MacOSWindowManager.mm
    Source/Main/MacOSGameClient.mm
    Source/Main/MacOSGameWindowManager.mm
    Source/Main/MacOSGadgetDraw.mm
    Source/Main/StdKeyboard.mm
    Source/Main/StdMouse.mm
    Source/Main/D3DXStubs.mm
    Source/Metal/MetalDevice8.mm
    Source/Metal/MetalInterface8.mm
    Source/Metal/MetalTexture8.mm
    Source/Metal/MetalSurface8.mm
    Source/Metal/MetalVertexBuffer8.mm
    Source/Metal/MetalIndexBuffer8.mm
    Source/Audio/MacOSAudioManager.mm
    Source/Client/MacOSDisplay.mm
    Source/Client/MacOSDisplayString.mm
    Source/Renderer/MacOSTexture.mm
    Source/Debug/MacOSScreenshot.mm
    Source/Common/StdBIGFile.cpp
    Source/Common/StdBIGFileSystem.cpp
    Source/Common/StdLocalFile.cpp
    Source/Common/StdLocalFileSystem.cpp
    Source/MacOSGameEngineHooks.cpp
    Source/Stubs/GameSpyStubs.cpp
    Source/Stubs/GitInfoStubs.cpp
    Source/Stubs/LZHLStubs.cpp
    Source/Stubs/MacOSW3DShaderManager.mm
    Source/Stubs/WWDownloadStubs.cpp
    Source/Main/MacOSShaders.metal
)

target_include_directories(macos_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Include    # shim headers FIRST
)

target_compile_definitions(macos_platform PUBLIC
    RA_MACOS=1
)

target_link_libraries(macos_platform PUBLIC
    "-framework Metal"
    "-framework Cocoa"
    "-framework Foundation"
    "-framework QuartzCore"
)
```

## Шаг 3: Root CMakeLists.txt changes

```cmake
# After existing FetchContent block:
if(APPLE)
    add_subdirectory(Platform/MacOS)
endif()

# Modify the dx8/miles/bink condition:
if((WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows") AND ${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    include(cmake/miles.cmake)
    include(cmake/bink.cmake)
    include(cmake/dx8.cmake)
endif()
# ↑ Already correct — these are Windows-only
```

## Шаг 4: GeneralsMD/Code/Main/CMakeLists.txt changes

```cmake
if(APPLE)
    # macOS build
    target_sources(z_generals PRIVATE MacOSMain.mm)  # If needed, or use from macos_platform
    target_link_libraries(z_generals PRIVATE
        macos_platform
        z_gameengine
        z_gameenginedevice
        zi_always
        "-lz"
    )
else()
    # Windows build (existing)
    target_link_libraries(z_generals PRIVATE
        binkstub comctl32 d3d8 d3dx8 dinput8 dxguid
        imm32 milesstub vfw32 winmm
        z_gameengine z_gameenginedevice zi_always
    )
endif()
```

## Шаг 5: CMakePresets.json

```json
{
    "name": "macos",
    "displayName": "macOS ARM64 Release",
    "generator": "Ninja",
    "binaryDir": "${sourceDir}/build/${presetName}",
    "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
        "CMAKE_BUILD_TYPE": "Debug",
        "RTS_BUILD_ZEROHOUR": "ON"
    }
}
```

## Шаг 6: Удаляем
- build_macos.sh
- setup_dependencies.sh
- Platform/MacOS/Scripts/files.sh (список файлов теперь в CMakeLists.txt)

## Порядок выполнения
1. Создать все stub-хедеры (Шаг 1)
2. Создать Platform/MacOS/CMakeLists.txt (Шаг 2)
3. Модифицировать root CMakeLists.txt (Шаг 3)
4. Модифицировать GeneralsMD/Code/Main/CMakeLists.txt (Шаг 4)
5. Добавить macos preset (Шаг 5)
6. Попробовать cmake --preset macos
7. Итеративно фиксить ошибки компиляции
