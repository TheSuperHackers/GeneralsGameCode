cmake_minimum_required(VERSION 3.28)

# Generic WND runtime code is owned by this directory. Keep the old engine
# trees from quietly gaining another copy when a new control is added.
set(_wnd_legacy_roots
    "${PROJECT_SOURCE_DIR}/Core/GameEngine/Include/GameClient"
    "${PROJECT_SOURCE_DIR}/Core/GameEngine/Source/GameClient/GUI"
    "${PROJECT_SOURCE_DIR}/Core/GameEngineDevice/Include/W3DDevice/GameClient"
    "${PROJECT_SOURCE_DIR}/Core/GameEngineDevice/Source/W3DDevice/GameClient/GUI"
    "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include/GameClient"
    "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/GameClient/GUI"
    "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient"
    "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/GUI")
set(_wnd_legacy_patterns
    "Gadget*.h"
    "Gadget/*.cpp"
    "Gadget/*.cppm"
    "GameFont.h"
    "GameFont.cpp"
    "GameFont.cppm"
    "GameWindow*.h"
    "GameWindow*.cpp"
    "GameWindow*.cppm"
    "WindowLayout.h"
    "WindowLayout.cpp"
    "WindowLayout.cppm"
    "WinInstanceData.h"
    "WinInstanceData.cpp"
    "WinInstanceData.cppm"
    "W3DGadget.h"
    "W3DGameFont.h"
    "W3DGameFont.cpp"
    "W3DGameFont.cppm"
    "W3DGameWindow*.h"
    "W3DGameWindow*.cpp"
    "W3DGameWindow*.cppm"
    "GUICallbacks/W3D*.cpp"
    "GUICallbacks/W3D*.cppm"
    "W3D*/**/*.cpp"
    "W3D*/**/*.cppm")
foreach(_wnd_legacy_root IN LISTS _wnd_legacy_roots)
    foreach(_wnd_legacy_pattern IN LISTS _wnd_legacy_patterns)
        file(GLOB_RECURSE _wnd_legacy_matches
            LIST_DIRECTORIES FALSE
            "${_wnd_legacy_root}/${_wnd_legacy_pattern}")
        if(_wnd_legacy_matches)
            message(FATAL_ERROR
                "Generic WND code must live under engine/gui/wnd; found: ${_wnd_legacy_matches}")
        endif()
    endforeach()
endforeach()

add_library(generals_ui_wnd STATIC)
set_property(TARGET generals_ui_wnd PROPERTY CXX_SCAN_FOR_MODULES ON)

target_sources(generals_ui_wnd
    PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/renderer"
            "${CMAKE_CURRENT_SOURCE_DIR}/controls"
            "${CMAKE_CURRENT_SOURCE_DIR}/layout"
            "${CMAKE_CURRENT_SOURCE_DIR}/document"
            "${CMAKE_CURRENT_SOURCE_DIR}/runtime"
        FILES
            renderer/WNDRenderer.cppm
            controls/WNDControls.cppm
            layout/WNDLayout.cppm
            document/WNDDocument.cppm
            runtime/WNDRuntime.cppm
)

target_compile_features(generals_ui_wnd PUBLIC cxx_std_23)
target_link_libraries(generals_ui_wnd PUBLIC generals_graphics generals_assets)

if(RTS_BUILD_GRAPHICS_TESTS)
    find_package(Boost CONFIG REQUIRED)

    add_executable(generals_ui_wnd_renderer_test)
    target_sources(generals_ui_wnd_renderer_test
        PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS
                "${CMAKE_CURRENT_SOURCE_DIR}/tests"
            FILES
                tests/renderer/WNDRenderer.test.cppm
    )
    target_compile_features(generals_ui_wnd_renderer_test PRIVATE cxx_std_23)
    target_include_directories(generals_ui_wnd_renderer_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_ui_wnd_renderer_test PRIVATE generals_ui_wnd)
    set(RTS_WND_GAME_DATA_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/generalsmd/run"
        CACHE PATH "Root directory containing the configured Generals Window data")
    if(RTS_WND_GAME_DATA_ROOT)
        file(TO_CMAKE_PATH "${RTS_WND_GAME_DATA_ROOT}" WND_GAME_DATA_ROOT_CMAKE)
        target_compile_definitions(generals_ui_wnd_renderer_test PRIVATE
            ENGINE_UI_WND_GAME_DATA_ROOT="${WND_GAME_DATA_ROOT_CMAKE}")
    endif()

    include(CTest)
    add_test(NAME generals_ui_wnd_renderer_test COMMAND generals_ui_wnd_renderer_test)

    add_executable(generals_ui_wnd_document_test)
    target_sources(generals_ui_wnd_document_test
        PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests"
            FILES
                tests/support/WNDGameData.cppm
                tests/document/WNDDocument.test.cppm
    )
    target_compile_features(generals_ui_wnd_document_test PRIVATE cxx_std_23)
    target_include_directories(generals_ui_wnd_document_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_ui_wnd_document_test PRIVATE generals_ui_wnd)
    if(RTS_WND_GAME_DATA_ROOT)
        target_compile_definitions(generals_ui_wnd_document_test PRIVATE
            ENGINE_UI_WND_GAME_DATA_ROOT="${WND_GAME_DATA_ROOT_CMAKE}")
    endif()
    add_test(NAME generals_ui_wnd_document_test COMMAND generals_ui_wnd_document_test)

    add_executable(generals_ui_wnd_controls_test)
    target_sources(generals_ui_wnd_controls_test
        PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests"
            FILES
                tests/support/WNDGameData.cppm
                tests/controls/WNDControls.test.cppm
    )
    target_compile_features(generals_ui_wnd_controls_test PRIVATE cxx_std_23)
    target_include_directories(generals_ui_wnd_controls_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_ui_wnd_controls_test PRIVATE generals_ui_wnd)
    if(RTS_WND_GAME_DATA_ROOT)
        target_compile_definitions(generals_ui_wnd_controls_test PRIVATE
            ENGINE_UI_WND_GAME_DATA_ROOT="${WND_GAME_DATA_ROOT_CMAKE}")
    endif()
    add_test(NAME generals_ui_wnd_controls_test COMMAND generals_ui_wnd_controls_test)
endif()

if(RTS_BUILD_GRAPHICS_TESTS AND WIN32)
    get_target_property(WND_SHADER_DIRECTORY generals_graphics
        GRAPHICS_SHADER_ASSET_DIRECTORY)

    add_executable(generals_ui_wnd_texture_drawing_test)
    target_sources(generals_ui_wnd_texture_drawing_test PRIVATE
        FILE_SET CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        FILES tests/texture/WNDTextureDrawing.test.cppm)
    target_compile_features(generals_ui_wnd_texture_drawing_test PRIVATE cxx_std_23)
    target_include_directories(generals_ui_wnd_texture_drawing_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_ui_wnd_texture_drawing_test PRIVATE
        generals_ui_wnd
        generals_graphics_test_support)
    target_compile_definitions(generals_ui_wnd_texture_drawing_test PRIVATE
        GRAPHICS_TERRAIN_SHADER_DIRECTORY="${WND_SHADER_DIRECTORY}")
    add_dependencies(generals_ui_wnd_texture_drawing_test generals_graphics_shaders)
    set_property(GLOBAL APPEND PROPERTY GRAPHICS_TEST_TARGETS
        generals_ui_wnd_texture_drawing_test)
    foreach(graphics_test_backend IN ITEMS dx11 dx12)
        foreach(graphics_test_driver IN ITEMS hardware warp)
            set(graphics_test_instance
                "generals_ui_wnd_texture_drawing_test.${graphics_test_backend}.${graphics_test_driver}")
            add_test(NAME ${graphics_test_instance}
                COMMAND generals_ui_wnd_texture_drawing_test)
            set_tests_properties(${graphics_test_instance} PROPERTIES
                LABELS "graphics;${graphics_test_backend};${graphics_test_driver}"
                ENVIRONMENT
                    "GRAPHICS_BACKEND=${graphics_test_backend};GRAPHICS_TEST_BACKEND=${graphics_test_backend};GRAPHICS_TEST_DRIVER=${graphics_test_driver};GRAPHICS_TEST_FAILURE_SUFFIX=${graphics_test_backend}-${graphics_test_driver}")
        endforeach()
    endforeach()

    add_executable(generals_ui_wnd_asset_snapshot_test)
    target_sources(generals_ui_wnd_asset_snapshot_test
        PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS
                "${CMAKE_CURRENT_SOURCE_DIR}/tests"
                "${PROJECT_SOURCE_DIR}/engine/graphics"
            FILES
                tests/support/WNDGameData.cppm
                tests/snapshots/WNDAssetSnapshots.test.cppm
                "${PROJECT_SOURCE_DIR}/engine/graphics/tests/visual_regression/VisualRegression.cppm"
    )
    target_compile_features(generals_ui_wnd_asset_snapshot_test PRIVATE cxx_std_23)
    target_include_directories(generals_ui_wnd_asset_snapshot_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_ui_wnd_asset_snapshot_test PRIVATE
        generals_ui_wnd
        generals_graphics_test_support)
    target_compile_definitions(generals_ui_wnd_asset_snapshot_test PRIVATE
        ENGINE_UI_WND_GAME_DATA_ROOT="${WND_GAME_DATA_ROOT_CMAKE}"
        ENGINE_UI_WND_SHADER_DIRECTORY="${WND_SHADER_DIRECTORY}"
        ENGINE_UI_WND_VISUAL_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/tests/snapshots/__snapshots__"
        ENGINE_UI_WND_VISUAL_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/visual_failures")
    add_dependencies(generals_ui_wnd_asset_snapshot_test generals_graphics_shaders)
    set_property(GLOBAL APPEND PROPERTY GRAPHICS_TEST_TARGETS
        generals_ui_wnd_asset_snapshot_test)
    foreach(graphics_test_backend IN ITEMS dx11 dx12)
        foreach(graphics_test_driver IN ITEMS hardware warp)
            set(graphics_test_instance
                "generals_ui_wnd_asset_snapshot_test.${graphics_test_backend}.${graphics_test_driver}")
            add_test(NAME ${graphics_test_instance}
                COMMAND generals_ui_wnd_asset_snapshot_test)
            set_tests_properties(${graphics_test_instance} PROPERTIES
                LABELS "graphics;${graphics_test_backend};${graphics_test_driver}"
                ENVIRONMENT
                    "GRAPHICS_BACKEND=${graphics_test_backend};GRAPHICS_TEST_BACKEND=${graphics_test_backend};GRAPHICS_TEST_DRIVER=${graphics_test_driver};GRAPHICS_TEST_FAILURE_SUFFIX=${graphics_test_backend}-${graphics_test_driver}")
        endforeach()
    endforeach()

    add_executable(generals_ui_wnd_button_snapshot_test)
    target_sources(generals_ui_wnd_button_snapshot_test
        PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS
                "${CMAKE_CURRENT_SOURCE_DIR}/tests"
                "${PROJECT_SOURCE_DIR}/engine/graphics"
            FILES
                tests/support/WNDGameData.cppm
                tests/buttons/WNDButtonSnapshots.test.cppm
                "${PROJECT_SOURCE_DIR}/engine/graphics/tests/visual_regression/VisualRegression.cppm"
    )
    target_compile_features(generals_ui_wnd_button_snapshot_test PRIVATE cxx_std_23)
    target_include_directories(generals_ui_wnd_button_snapshot_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_ui_wnd_button_snapshot_test PRIVATE
        generals_ui_wnd
        generals_graphics_test_support)
    target_compile_definitions(generals_ui_wnd_button_snapshot_test PRIVATE
        ENGINE_UI_WND_GAME_DATA_ROOT="${WND_GAME_DATA_ROOT_CMAKE}"
        ENGINE_UI_WND_SHADER_DIRECTORY="${WND_SHADER_DIRECTORY}"
        ENGINE_UI_WND_VISUAL_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/tests/snapshots/__snapshots__"
        ENGINE_UI_WND_VISUAL_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/visual_failures")
    add_dependencies(generals_ui_wnd_button_snapshot_test generals_graphics_shaders)
    set_property(GLOBAL APPEND PROPERTY GRAPHICS_TEST_TARGETS
        generals_ui_wnd_button_snapshot_test)
    foreach(graphics_test_backend IN ITEMS dx11 dx12)
        foreach(graphics_test_driver IN ITEMS hardware warp)
            set(graphics_test_instance
                "generals_ui_wnd_button_snapshot_test.${graphics_test_backend}.${graphics_test_driver}")
            add_test(NAME ${graphics_test_instance}
                COMMAND generals_ui_wnd_button_snapshot_test)
            set_tests_properties(${graphics_test_instance} PROPERTIES
                LABELS "graphics;${graphics_test_backend};${graphics_test_driver}"
                ENVIRONMENT
                    "GRAPHICS_BACKEND=${graphics_test_backend};GRAPHICS_TEST_BACKEND=${graphics_test_backend};GRAPHICS_TEST_DRIVER=${graphics_test_driver};GRAPHICS_TEST_FAILURE_SUFFIX=${graphics_test_backend}-${graphics_test_driver}")
        endforeach()
    endforeach()

    add_executable(generals_ui_wnd_control_snapshot_test)
    target_sources(generals_ui_wnd_control_snapshot_test
        PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS
                "${CMAKE_CURRENT_SOURCE_DIR}/tests"
                "${PROJECT_SOURCE_DIR}/engine/graphics"
            FILES
                tests/support/WNDGameData.cppm
                tests/controls/WNDControlSnapshots.test.cppm
                "${PROJECT_SOURCE_DIR}/engine/graphics/tests/visual_regression/VisualRegression.cppm")
    target_compile_features(generals_ui_wnd_control_snapshot_test PRIVATE cxx_std_23)
    target_include_directories(generals_ui_wnd_control_snapshot_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_ui_wnd_control_snapshot_test PRIVATE
        generals_ui_wnd
        generals_graphics_test_support)
    target_compile_definitions(generals_ui_wnd_control_snapshot_test PRIVATE
        ENGINE_UI_WND_GAME_DATA_ROOT="${WND_GAME_DATA_ROOT_CMAKE}"
        ENGINE_UI_WND_SHADER_DIRECTORY="${WND_SHADER_DIRECTORY}"
        ENGINE_UI_WND_VISUAL_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/tests/snapshots/__snapshots__"
        ENGINE_UI_WND_VISUAL_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/visual_failures")
    add_dependencies(generals_ui_wnd_control_snapshot_test generals_graphics_shaders)
    set_property(GLOBAL APPEND PROPERTY GRAPHICS_TEST_TARGETS
        generals_ui_wnd_control_snapshot_test)
    foreach(graphics_test_backend IN ITEMS dx11 dx12)
        foreach(graphics_test_driver IN ITEMS hardware warp)
            set(graphics_test_instance
                "generals_ui_wnd_control_snapshot_test.${graphics_test_backend}.${graphics_test_driver}")
            add_test(NAME ${graphics_test_instance}
                COMMAND generals_ui_wnd_control_snapshot_test)
            set_tests_properties(${graphics_test_instance} PROPERTIES
                LABELS "graphics;${graphics_test_backend};${graphics_test_driver}"
                ENVIRONMENT
                    "GRAPHICS_BACKEND=${graphics_test_backend};GRAPHICS_TEST_BACKEND=${graphics_test_backend};GRAPHICS_TEST_DRIVER=${graphics_test_driver};GRAPHICS_TEST_FAILURE_SUFFIX=${graphics_test_backend}-${graphics_test_driver}")
        endforeach()
    endforeach()
endif()
