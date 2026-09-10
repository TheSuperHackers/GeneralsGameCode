cmake_minimum_required(VERSION 3.28)

add_library(generals_ui_wnd STATIC)
set_property(TARGET generals_ui_wnd PROPERTY CXX_SCAN_FOR_MODULES ON)

target_sources(generals_ui_wnd
    PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES
            WNDRenderer.cppm
            WNDLayout.cppm
)

target_compile_features(generals_ui_wnd PUBLIC cxx_std_23)
target_link_libraries(generals_ui_wnd PUBLIC generals_graphics generals_assets)

if(RTS_BUILD_GRAPHICS_TESTS)
    find_package(Boost CONFIG REQUIRED)

    add_executable(generals_ui_wnd_renderer_test)
    target_sources(generals_ui_wnd_renderer_test
        PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
            FILES
                WNDRenderer.test.cppm
    )
    target_compile_features(generals_ui_wnd_renderer_test PRIVATE cxx_std_23)
    target_include_directories(generals_ui_wnd_renderer_test PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(generals_ui_wnd_renderer_test PRIVATE generals_ui_wnd)

    set(RTS_WND_GAME_DATA_ROOT "${PROJECT_SOURCE_DIR}/GeneralsMD/Run" CACHE PATH
        "Root directory containing the configured Generals Window data")
    if(RTS_WND_GAME_DATA_ROOT)
        file(TO_CMAKE_PATH "${RTS_WND_GAME_DATA_ROOT}" WND_GAME_DATA_ROOT_CMAKE)
        target_compile_definitions(generals_ui_wnd_renderer_test PRIVATE
            ENGINE_UI_WND_GAME_DATA_ROOT="${WND_GAME_DATA_ROOT_CMAKE}")
    endif()

    if(WIN32)
        get_target_property(WND_SHADER_DIRECTORY generals_graphics
            GRAPHICS_SHADER_ASSET_DIRECTORY)
        if(WND_SHADER_DIRECTORY AND NOT WND_SHADER_DIRECTORY STREQUAL "WND_SHADER_DIRECTORY-NOTFOUND")
            target_sources(generals_ui_wnd_renderer_test
                PRIVATE
                    FILE_SET CXX_MODULES
                    BASE_DIRS "${PROJECT_SOURCE_DIR}/engine/graphics"
                    FILES
                        "${PROJECT_SOURCE_DIR}/engine/graphics/tests/visual_regression/VisualRegression.cppm"
            )
            target_compile_definitions(generals_ui_wnd_renderer_test PRIVATE
                ENGINE_UI_WND_VISUAL_REGRESSION=1
                ENGINE_UI_WND_SHADER_DIRECTORY="${WND_SHADER_DIRECTORY}"
                ENGINE_UI_WND_VISUAL_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/__snapshots__"
                ENGINE_UI_WND_VISUAL_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/visual_failures"
            )
            if(TARGET generals_graphics_shaders)
                add_dependencies(generals_ui_wnd_renderer_test generals_graphics_shaders)
            endif()
        endif()
    endif()

    include(CTest)
    add_test(NAME generals_ui_wnd_renderer_test COMMAND generals_ui_wnd_renderer_test)
endif()

if(RTS_BUILD_GRAPHICS_TESTS AND WIN32)
    add_executable(generals_ui_wnd_texture_drawing_test)
    target_sources(generals_ui_wnd_texture_drawing_test PRIVATE
        FILE_SET CXX_MODULES FILES WNDTextureDrawing.test.cppm)
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
endif()
