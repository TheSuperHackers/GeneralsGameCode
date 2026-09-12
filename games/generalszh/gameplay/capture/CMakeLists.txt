add_library(generalszh_capture_definitions STATIC)
target_sources(generalszh_capture_definitions PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/capture_state.cppm definitions/capture_definition.cppm)
target_compile_features(generalszh_capture_definitions PUBLIC cxx_std_23)
target_link_libraries(generalszh_capture_definitions PUBLIC engine_ecs engine_time)

add_library(generalszh_capture STATIC)
target_sources(generalszh_capture PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    inputs/capture_batch.cppm systems/capture_system.cppm systems/capture_upgrade_activation_system.cppm)
target_compile_features(generalszh_capture PUBLIC cxx_std_23)
target_link_libraries(generalszh_capture PUBLIC generalszh_capture_definitions generalszh_match_rules
    generalszh_build_execution generalszh_harvesting_execution generalszh_economy_rules generalszh_sale_components engine_gameplay_rts_power_execution engine_gameplay_rts_economy)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set(capture_tests components/capture_state.test definitions/capture_definition.test inputs/capture_batch.test systems/capture_system.test systems/capture_upgrade_activation_system.test)
    set_source_files_properties(${capture_tests} PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_capture_tests ${capture_tests})
    target_compile_definitions(generalszh_capture_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_capture_tests PRIVATE generalszh_capture Boost::included_unit_test_framework)
    add_test(NAME generalszh_capture_tests COMMAND generalszh_capture_tests)
endif()
