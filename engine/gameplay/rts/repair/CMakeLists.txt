add_library(engine_gameplay_rts_repair STATIC)
target_sources(engine_gameplay_rts_repair PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    definitions/repair_definition.cppm components/repair_state.cppm components/manual_repair.cppm
    algorithms/repair_progress.cppm)
target_compile_features(engine_gameplay_rts_repair PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_rts_repair PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_rts_repair PUBLIC engine_ecs engine_time engine_gameplay_navigation)
if(ENGINE_GAMEPLAY_BUILD_TESTS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set(repair_tests definitions/repair_definition.test components/repair_state.test components/manual_repair.test
        algorithms/repair_progress.test)
    set_source_files_properties(${repair_tests} PROPERTIES LANGUAGE CXX)
    add_executable(engine_gameplay_rts_repair_tests ${repair_tests})
    target_compile_definitions(engine_gameplay_rts_repair_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(engine_gameplay_rts_repair_tests PRIVATE engine_gameplay_rts_repair Boost::included_unit_test_framework)
    add_test(NAME engine_gameplay_rts_repair_tests COMMAND engine_gameplay_rts_repair_tests)
endif()
