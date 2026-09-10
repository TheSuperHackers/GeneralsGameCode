cmake_minimum_required(VERSION 4.3)

# Use packagename_ROOT for FindPackage.
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(FeatureSummary)
include(CMakeDependentOption)

# We don't support in tree builds, so help people make the right choice.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

# Top level project, doesn't really affect anything.
project(genzh LANGUAGES C CXX)

set(CMAKE_CXX_SCAN_FOR_MODULES ON)

if(NOT WIN32)
    message(FATAL_ERROR "GeneralsMD supports Windows only.")
endif()

if(NOT CMAKE_C_COMPILER_ID STREQUAL "Clang" OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "GeneralsMD requires Clang/LLVM for both C and C++; found C=${CMAKE_C_COMPILER_ID}, CXX=${CMAKE_CXX_COMPILER_ID}.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "GeneralsMD requires a Windows 64-bit target.")
endif()

# Prevent legacy winsock.h from being pulled in by windows.h. The supported
# Windows target uses Winsock 2 throughout the active GeneralsMD graph.
add_compile_definitions(WIN32_LEAN_AND_MEAN _WINSOCKAPI_)

# This file handles extra settings wanted/needed for different compilers.
include(cmake/compilers.cmake)

include(FetchContent)

# Find/Add build dependencies and stubs shared by all projects
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    # WWAudio still uses the Miles-compatible SDK types (mss.h); this does
    # not provide or select the removed MilesAudioManager backend.
    include(cmake/miles.cmake)
    include(cmake/dx9.cmake)
    include(cmake/ffmpeg.cmake)
    include(cmake/sdl3.cmake)
endif()

include(cmake/config.cmake)
include(cmake/gamespy.cmake)
include(cmake/lzhl.cmake)
include(cmake/stb.cmake)

# Graphics owns the engine shader language. The Windows build compiles its
# checked-in Slang sources at build time for the active renderer targets.
if(RTS_BUILD_ZEROHOUR AND WIN32)
    include(cmake/slang.cmake)
endif()

add_subdirectory(Dependencies/Utility)

add_subdirectory(resources)

add_subdirectory(Core)

if(RTS_BUILD_ZEROHOUR)
    include(CTest)
    add_subdirectory(engine/assets)
    add_subdirectory(engine/geometry)
    add_subdirectory(engine/video)
    add_subdirectory(engine/graphics)
    add_subdirectory(engine/gui)
endif()

# Add main build targets
if(RTS_BUILD_ZEROHOUR)
    add_subdirectory(engine/navigation)
    if(ENGINE_NAVIGATION_BUILD_TESTS)
        enable_testing()
    endif()
    add_subdirectory(GeneralsMD)
endif()

if(RTS_BUILD_ZEROHOUR AND RTS_BUILD_GRAPHICS_TESTS)
    find_package(Boost CONFIG REQUIRED)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    function(add_graphics_matrix_test test_name test_target)
        set_property(GLOBAL APPEND PROPERTY GRAPHICS_TEST_TARGETS ${test_target})
        if(WIN32)
            foreach(graphics_test_backend IN ITEMS dx11 dx12)
                foreach(graphics_test_driver IN ITEMS hardware warp)
                    set(graphics_test_instance
                        "${test_name}.${graphics_test_backend}.${graphics_test_driver}")
                    add_test(NAME ${graphics_test_instance} COMMAND ${test_target})
                    set_tests_properties(${graphics_test_instance} PROPERTIES
                        LABELS "graphics;${graphics_test_backend};${graphics_test_driver}"
                        ENVIRONMENT
                            "GRAPHICS_BACKEND=${graphics_test_backend};GRAPHICS_TEST_BACKEND=${graphics_test_backend};GRAPHICS_TEST_DRIVER=${graphics_test_driver};GRAPHICS_TEST_FAILURE_SUFFIX=${graphics_test_backend}-${graphics_test_driver}")
                endforeach()
            endforeach()
        else()
            add_test(NAME ${test_name} COMMAND ${test_target})
        endif()
    endfunction()

    if(WIN32)
        foreach(graphics_test_backend IN ITEMS dx11 dx12)
            foreach(graphics_test_driver IN ITEMS hardware warp)
                set(graphics_test_instance
                    "generals_graphics_water_environment_test.${graphics_test_backend}.${graphics_test_driver}")
                add_test(NAME ${graphics_test_instance}
                    COMMAND ${Python3_EXECUTABLE}
                        "${CMAKE_CURRENT_SOURCE_DIR}/engine/graphics/scene/water/WaterEnvironment.test.py")
                set_tests_properties(${graphics_test_instance} PROPERTIES
                    LABELS "graphics;${graphics_test_backend};${graphics_test_driver}"
                        ENVIRONMENT
                            "GRAPHICS_BACKEND=${graphics_test_backend};GRAPHICS_TEST_BACKEND=${graphics_test_backend};GRAPHICS_TEST_DRIVER=${graphics_test_driver};GRAPHICS_TEST_FAILURE_SUFFIX=${graphics_test_backend}-${graphics_test_driver}")
            endforeach()
        endforeach()
    else()
        add_test(NAME generals_graphics_water_environment_test
            COMMAND ${Python3_EXECUTABLE}
                "${CMAKE_CURRENT_SOURCE_DIR}/engine/graphics/scene/water/WaterEnvironment.test.py")
    endif()

    add_executable(generals_graphics_collision_box_render_object_query_test)
    set_property(TARGET generals_graphics_collision_box_render_object_query_test
        PROPERTY CXX_SCAN_FOR_MODULES ON)

    target_sources(generals_graphics_collision_box_render_object_query_test PRIVATE
        engine/graphics/scene/debug/CollisionBoxRenderObject.test.cpp
    )
        target_compile_features(generals_graphics_collision_box_render_object_query_test PRIVATE cxx_std_23)
    target_include_directories(generals_graphics_collision_box_render_object_query_test PRIVATE
        ${Boost_INCLUDE_DIRS}
        "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
        "${CMAKE_CURRENT_SOURCE_DIR}/Core/Libraries/Source/WWVegas"
    )
    target_link_libraries(generals_graphics_collision_box_render_object_query_test PRIVATE
        z_gameenginedevice
        generals_graphics
        zi_always
    )
    if(WIN32)
        target_link_libraries(generals_graphics_collision_box_render_object_query_test PRIVATE winmm)
    endif()

    add_graphics_matrix_test(
        generals_graphics_collision_box_render_object_query_test
        generals_graphics_collision_box_render_object_query_test
    )

    if(WIN32)
        add_executable(generals_graphics_null_render_object_test)
        set_property(TARGET generals_graphics_null_render_object_test
            PROPERTY CXX_SCAN_FOR_MODULES ON)

        target_sources(generals_graphics_null_render_object_test PRIVATE
            engine/graphics/scene/debug/NullRenderObject.test.cpp
        )
        target_compile_features(generals_graphics_null_render_object_test PRIVATE cxx_std_23)
        target_include_directories(generals_graphics_null_render_object_test PRIVATE
            ${Boost_INCLUDE_DIRS}
            "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
            "${CMAKE_CURRENT_SOURCE_DIR}/Core/Libraries/Source/WWVegas"
        )
        target_link_libraries(generals_graphics_null_render_object_test PRIVATE
            z_gameenginedevice
            generals_graphics
            generals_graphics_test_support
            zi_always
            winmm
        )

        add_graphics_matrix_test(
            generals_graphics_null_render_object_test
            generals_graphics_null_render_object_test
        )

        add_executable(generals_graphics_w3d_value_conversion_test)
        set_property(TARGET generals_graphics_w3d_value_conversion_test
            PROPERTY CXX_SCAN_FOR_MODULES ON)

        target_sources(generals_graphics_w3d_value_conversion_test PRIVATE
            engine/graphics/test_scenarios/w3d/W3DValueConversion.test.cpp
        )
        target_compile_features(generals_graphics_w3d_value_conversion_test PRIVATE cxx_std_23)
        target_include_directories(generals_graphics_w3d_value_conversion_test PRIVATE
            ${Boost_INCLUDE_DIRS}
            "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
            "${CMAKE_CURRENT_SOURCE_DIR}/Core/Libraries/Source/WWVegas"
        )
        target_link_libraries(generals_graphics_w3d_value_conversion_test PRIVATE
            z_gameenginedevice
            generals_graphics
            generals_graphics_test_support
            zi_always
            winmm
        )

        get_target_property(GRAPHICS_W3D_VALUE_CONVERSION_SHADER_DIRECTORY
            generals_graphics GRAPHICS_SHADER_ASSET_DIRECTORY)
        target_compile_definitions(generals_graphics_w3d_value_conversion_test PRIVATE
            GRAPHICS_W3D_VALUE_CONVERSION_SHADER_DIRECTORY="${GRAPHICS_W3D_VALUE_CONVERSION_SHADER_DIRECTORY}"
        )
        if(TARGET generals_graphics_prop_shaders)
            add_dependencies(generals_graphics_w3d_value_conversion_test
                generals_graphics_prop_shaders)
        endif()

        add_graphics_matrix_test(
            generals_graphics_w3d_value_conversion_test
            generals_graphics_w3d_value_conversion_test
        )

        add_executable(generals_graphics_w3d_scene_adapter_test)
        set_property(TARGET generals_graphics_w3d_scene_adapter_test
            PROPERTY CXX_SCAN_FOR_MODULES ON)

        target_sources(generals_graphics_w3d_scene_adapter_test PRIVATE
            engine/graphics/test_scenarios/w3d/W3DSceneAdapter.test.cpp
        )
        target_compile_features(generals_graphics_w3d_scene_adapter_test PRIVATE cxx_std_23)
        target_include_directories(generals_graphics_w3d_scene_adapter_test PRIVATE
            ${Boost_INCLUDE_DIRS}
            "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
            "${CMAKE_CURRENT_SOURCE_DIR}/Core/Libraries/Source/WWVegas"
        )
        target_link_libraries(generals_graphics_w3d_scene_adapter_test PRIVATE
            z_gameenginedevice
            generals_graphics
            zi_always
            winmm
        )

        add_graphics_matrix_test(
            generals_graphics_w3d_scene_adapter_test
            generals_graphics_w3d_scene_adapter_test
        )

        add_executable(generals_graphics_w3d_asset_catalog_test
            engine/graphics/test_scenarios/w3d/W3DAssetCatalog.test.cpp)
        set_property(TARGET generals_graphics_w3d_asset_catalog_test
            PROPERTY CXX_SCAN_FOR_MODULES ON)
        target_compile_features(generals_graphics_w3d_asset_catalog_test PRIVATE cxx_std_23)
        target_include_directories(generals_graphics_w3d_asset_catalog_test PRIVATE
            ${Boost_INCLUDE_DIRS}
            "${CMAKE_CURRENT_SOURCE_DIR}/GeneralsMD/Code/Libraries/Source/WWVegas"
            "${CMAKE_CURRENT_SOURCE_DIR}/Core/Libraries/Source/WWVegas")
        target_link_libraries(generals_graphics_w3d_asset_catalog_test PRIVATE
            z_gameenginedevice generals_graphics generals_graphics_test_support zi_always winmm)
        target_compile_definitions(generals_graphics_w3d_asset_catalog_test PRIVATE
            GRAPHICS_W3D_CATALOG_SHADER_DIRECTORY="${GRAPHICS_W3D_VALUE_CONVERSION_SHADER_DIRECTORY}")
        if(TARGET generals_graphics_prop_shaders)
            add_dependencies(generals_graphics_w3d_asset_catalog_test generals_graphics_prop_shaders)
        endif()
        add_graphics_matrix_test(
            generals_graphics_w3d_asset_catalog_test
            generals_graphics_w3d_asset_catalog_test)
    endif()

    get_property(graphics_test_targets GLOBAL PROPERTY GRAPHICS_TEST_TARGETS)
    if(graphics_test_targets AND TARGET generals_graphics_tests)
        add_dependencies(generals_graphics_tests ${graphics_test_targets})
    elseif(graphics_test_targets AND NOT TARGET generals_graphics_tests)
        add_custom_target(generals_graphics_tests DEPENDS ${graphics_test_targets})
    endif()
endif()

feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
