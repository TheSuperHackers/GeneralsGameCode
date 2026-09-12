add_library(engine_gameplay_containment STATIC)
target_sources(engine_gameplay_containment PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/passenger_membership.cppm components/containment_task.cppm
    definitions/transport_definition.cppm inputs/containment_batch.cppm)
target_compile_features(engine_gameplay_containment PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_containment PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_containment PUBLIC engine_ecs engine_time)
if(ENGINE_GAMEPLAY_BUILD_TESTS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set(containment_tests components/passenger_membership.test components/containment_task.test
        definitions/transport_definition.test inputs/containment_batch.test)
    set_source_files_properties(${containment_tests} PROPERTIES LANGUAGE CXX)
    add_executable(engine_gameplay_containment_tests ${containment_tests})
    target_compile_definitions(engine_gameplay_containment_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(engine_gameplay_containment_tests PRIVATE engine_gameplay_containment Boost::included_unit_test_framework)
    add_test(NAME engine_gameplay_containment_tests COMMAND engine_gameplay_containment_tests)
endif()
