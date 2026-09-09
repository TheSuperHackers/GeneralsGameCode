cmake_minimum_required(VERSION 3.28)

# Standalone configuration keeps characterization tests free of game/SDK dependencies.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(engine_navigation LANGUAGES CXX)
    set(NAVIGATION_STANDALONE ON)
endif()

option(ENGINE_NAVIGATION_BUILD_TESTS "Build navigation characterization tests" ${NAVIGATION_STANDALONE})

add_library(engine_navigation STATIC)
target_sources(engine_navigation PUBLIC
    FILE_SET CXX_MODULES FILES
        navigation.cppm
        costs.cppm
        querymemo.cppm
        search_workspace.cppm
        dynamic_request_queue.cppm
        jump_grid.cppm
        topology/clearance_cache.cppm
        movement/occupancy_policy.cppm
)
target_include_directories(engine_navigation PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../..")
target_compile_features(engine_navigation PUBLIC cxx_std_20)
set_property(TARGET engine_navigation PROPERTY CXX_SCAN_FOR_MODULES ON)
if(TARGET core_config)
    target_link_libraries(engine_navigation PUBLIC core_config)
endif()

# Attach the game-facing module after the GeneralsMD dependency targets exist.
# The adapter uses the game's headers and memory policy.
function(engine_navigation_attach_generalsmd game_target)
    add_library(engine_navigation_generalsmd STATIC)
    target_sources(engine_navigation_generalsmd PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_FUNCTION_LIST_DIR}"
        FILES "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pathfinder.cppm"
    )
    set_source_files_properties("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pathfinder.cppm"
        PROPERTIES SKIP_PRECOMPILE_HEADERS ON
    )
    get_target_property(game_includes ${game_target} INCLUDE_DIRECTORIES)
    target_include_directories(engine_navigation_generalsmd PRIVATE
        ${game_includes}
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../../Core/GameEngine/Include/Precompiled"
    )
    target_compile_definitions(engine_navigation_generalsmd PRIVATE IG_DEBUG_STACKTRACE)
    target_compile_features(engine_navigation_generalsmd PUBLIC cxx_std_20)
    set_property(TARGET engine_navigation_generalsmd PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_link_libraries(engine_navigation_generalsmd PUBLIC engine_navigation)
    target_link_libraries(engine_navigation_generalsmd PRIVATE
        zi_always
        corei_gameengine_public
        core_profile_legacy
        z_wwvegas
        SDL3::SDL3
    )
    target_link_libraries(${game_target} PRIVATE engine_navigation_generalsmd)
endfunction()

function(engine_navigation_add_world_tests)
    if(NOT ENGINE_NAVIGATION_BUILD_TESTS)
        return()
    endif()
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set(nav_dir "${CMAKE_CURRENT_FUNCTION_LIST_DIR}")
    foreach(world_target IN ITEMS engine_navigation_world_tests engine_navigation_world_bench)
        if(world_target STREQUAL "engine_navigation_world_tests")
            set(world_sources
                "${nav_dir}/pathfinder.test"
                "${nav_dir}/jps_adapter.test"
                "${nav_dir}/partition.test"
            )
            set(world_modules
                "${nav_dir}/test/world_fixture.cppm"
                "${nav_dir}/test/partition_fixture.cppm"
            )
        else()
            set(world_sources "${nav_dir}/test/pathfinder.bench.cpp")
            set(world_modules "${nav_dir}/test/world_fixture.cppm")
        endif()
        add_executable(${world_target} ${world_sources})
        set_source_files_properties(${world_sources} PROPERTIES LANGUAGE CXX SKIP_PRECOMPILE_HEADERS ON)
        target_sources(${world_target} PRIVATE
            FILE_SET CXX_MODULES BASE_DIRS "${nav_dir}" FILES ${world_modules})
        set_source_files_properties(${world_modules} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
        target_compile_definitions(${world_target} PRIVATE BOOST_TEST_INCLUDED IG_DEBUG_STACKTRACE)
        target_include_directories(${world_target} PRIVATE "${nav_dir}/../../Core/GameEngine/Include/Precompiled")
        target_link_libraries(${world_target} PRIVATE
            engine_navigation_generalsmd Boost::included_unit_test_framework
            z_gameengine z_gameenginedevice zi_always
            comctl32 d3d9lib imm32 SDL3::SDL3 vfw32 winmm)
        set_property(TARGET ${world_target} PROPERTY CXX_SCAN_FOR_MODULES ON)
        add_custom_command(TARGET ${world_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL3::SDL3>" "$<TARGET_FILE_DIR:${world_target}>"
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${ffmpeg_SOURCE_DIR}/bin" "$<TARGET_FILE_DIR:${world_target}>")
    endforeach()
    add_test(NAME engine_navigation_world_tests COMMAND engine_navigation_world_tests)
endfunction()

if(ENGINE_NAVIGATION_BUILD_TESTS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    find_package(Threads REQUIRED)
    # Historical kernels remain available only to characterization/reference tests.
    # The game cannot accidentally select the linked lists or bounded request ring.
    add_library(engine_navigation_reference STATIC)
    target_sources(engine_navigation_reference PUBLIC FILE_SET CXX_MODULES FILES
        test/reference/search_lists.cppm
        test/reference/request_queue.cppm)
    target_link_libraries(engine_navigation_reference PUBLIC engine_navigation)
    set(ENGINE_NAVIGATION_TESTS
        navigation.test
        costs.test
        search_lists.test
        request_queue.test
        querymemo.test
        search_workspace.test
        dynamic_request_queue.test
        jump_grid.test
        topology/clearance_cache.test
        movement/occupancy_policy.test
    )
    set_source_files_properties(${ENGINE_NAVIGATION_TESTS} PROPERTIES LANGUAGE CXX)
    enable_testing()
    add_executable(engine_navigation_tests ${ENGINE_NAVIGATION_TESTS})
    target_compile_definitions(engine_navigation_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(engine_navigation_tests PRIVATE
        engine_navigation
        engine_navigation_reference
        Threads::Threads
        Boost::included_unit_test_framework
    )
    set_property(TARGET engine_navigation_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    add_test(NAME engine_navigation_tests COMMAND engine_navigation_tests)
endif()
