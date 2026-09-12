add_library(engine_gameplay_pickups STATIC)
target_sources(engine_gameplay_pickups PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    algorithms/pickup_arbitration.cppm)
target_compile_features(engine_gameplay_pickups PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_pickups PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_pickups PUBLIC engine_ecs)

if(ENGINE_GAMEPLAY_BUILD_TESTS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(algorithms/pickup_arbitration.test PROPERTIES LANGUAGE CXX)
    add_executable(engine_gameplay_pickups_tests algorithms/pickup_arbitration.test)
    set_property(TARGET engine_gameplay_pickups_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(engine_gameplay_pickups_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(engine_gameplay_pickups_tests PRIVATE
        engine_gameplay_pickups Boost::included_unit_test_framework)
    add_test(NAME engine_gameplay_pickups_tests COMMAND engine_gameplay_pickups_tests)
endif()
