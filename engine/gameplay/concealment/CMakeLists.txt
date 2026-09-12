add_library(engine_gameplay_concealment STATIC)
target_sources(engine_gameplay_concealment PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    definitions/concealment_definition.cppm definitions/detection_definition.cppm
    components/concealment_state.cppm components/detection_state.cppm components/concealment_binding.cppm
    components/concealment_eligibility.cppm inputs/uncloak_input.cppm
    algorithms/detection_lease.cppm algorithms/detection_index.cppm systems/concealment_system.cppm)
target_compile_features(engine_gameplay_concealment PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_concealment PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_concealment PUBLIC engine_ecs engine_time engine_gameplay_spatial
    engine_gameplay_navigation engine_gameplay_combat engine_gameplay_containment)
if(ENGINE_GAMEPLAY_BUILD_TESTS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set(concealment_tests
        definitions/concealment_definition.test definitions/detection_definition.test
        components/concealment_state.test components/detection_state.test components/concealment_binding.test
        components/concealment_eligibility.test inputs/uncloak_input.test
        algorithms/detection_lease.test algorithms/detection_index.test systems/concealment_system.test)
    set_source_files_properties(${concealment_tests} PROPERTIES LANGUAGE CXX)
    add_executable(engine_gameplay_concealment_tests ${concealment_tests})
    set_property(TARGET engine_gameplay_concealment_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(engine_gameplay_concealment_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(engine_gameplay_concealment_tests PRIVATE engine_gameplay_concealment
        Boost::included_unit_test_framework)
    add_test(NAME engine_gameplay_concealment_tests COMMAND engine_gameplay_concealment_tests)
endif()
