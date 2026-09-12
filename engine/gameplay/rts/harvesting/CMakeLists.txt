add_library(engine_gameplay_rts_harvesting STATIC)
target_sources(engine_gameplay_rts_harvesting PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/harvest_state.cppm snapshots/harvest_snapshots.cppm
    systems/harvest_system.cppm)
target_compile_features(engine_gameplay_rts_harvesting PUBLIC cxx_std_23)
target_link_libraries(engine_gameplay_rts_harvesting PUBLIC engine_gameplay_navigation engine_gameplay_combat engine_events)
if(ENGINE_GAMEPLAY_BUILD_TESTS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set(harvest_tests components/harvest_state.test snapshots/harvest_snapshots.test
        systems/harvest_system.test)
    set_source_files_properties(${harvest_tests} PROPERTIES LANGUAGE CXX)
    add_executable(engine_gameplay_harvesting_tests ${harvest_tests})
    target_compile_definitions(engine_gameplay_harvesting_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(engine_gameplay_harvesting_tests PRIVATE engine_gameplay_rts_harvesting Boost::included_unit_test_framework)
    add_test(NAME engine_gameplay_harvesting_tests COMMAND engine_gameplay_harvesting_tests)
endif()
