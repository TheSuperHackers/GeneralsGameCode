add_library(generalszh_power_rules STATIC)
target_sources(generalszh_power_rules PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
	components/power_recovery.cppm algorithms/power_suppression.cppm
	components/power_suppression.cppm algorithms/power_contribution.cppm
	contributions/power_contribution.cppm suppression/power_suppression.cppm
	recovery/power_recovery.cppm systems/power_recovery_system.cppm)
target_compile_features(generalszh_power_rules PUBLIC cxx_std_23)
set_property(TARGET generalszh_power_rules PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_power_rules PUBLIC engine_gameplay_rts_power)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)
	set_source_files_properties(algorithms/power_contribution.test algorithms/power_suppression.test
		components/power_recovery.test components/power_suppression.test
		systems/power_recovery_system.test PROPERTIES LANGUAGE CXX)
	add_executable(generalszh_power_rules_tests algorithms/power_contribution.test algorithms/power_suppression.test
		components/power_recovery.test components/power_suppression.test
		systems/power_recovery_system.test)
	set_property(TARGET generalszh_power_rules_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
	target_compile_definitions(generalszh_power_rules_tests PRIVATE BOOST_TEST_INCLUDED)
	target_link_libraries(generalszh_power_rules_tests PRIVATE generalszh_power_rules Boost::included_unit_test_framework)
	add_test(NAME generalszh_power_rules_tests COMMAND generalszh_power_rules_tests)
endif()

# Lightweight policy/publication data shared with production and content adapters.
add_library(generalszh_power_policy STATIC)
target_sources(generalszh_power_policy PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/power_state.cppm definitions/power_config.cppm data/power_frame.cppm)
target_compile_features(generalszh_power_policy PUBLIC cxx_std_23)
target_link_libraries(generalszh_power_policy PUBLIC engine_ecs)

# One cohesive independent power behaviour; native recovery above stays separate.
add_library(generalszh_power_execution STATIC)
target_sources(generalszh_power_execution PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    systems/power_system.cppm)
target_compile_features(generalszh_power_execution PUBLIC cxx_std_23)
target_link_libraries(generalszh_power_execution PUBLIC engine_gameplay_rts_power_execution generalszh_power_policy)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    set_source_files_properties(systems/power_system.test components/power_state.test
        definitions/power_config.test data/power_frame.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_power_execution_tests systems/power_system.test components/power_state.test
        definitions/power_config.test data/power_frame.test)
    target_compile_definitions(generalszh_power_execution_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_power_execution_tests PRIVATE generalszh_power_execution Boost::included_unit_test_framework)
    add_test(NAME generalszh_power_execution_tests COMMAND generalszh_power_execution_tests)
endif()
