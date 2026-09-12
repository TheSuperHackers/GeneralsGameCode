add_library(generalszh_combat_rules STATIC)
target_sources(generalszh_combat_rules PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    targeting/target_frame.cppm targeting/target_index.cppm targeting/impact_target_projection.cppm targeting/launch_snapshot.cppm
    systems/targeting_system.cppm systems/acquisition_system.cppm
    inputs/poison_input.cppm systems/poison_cleanse_system.cppm systems/poison_input_system.cppm
    targeting/targeting_system.cppm acquisition/acquisition_system.cppm
    poison/poison_cleanse_system.cppm poison/poison_input_system.cppm)
target_compile_features(generalszh_combat_rules PUBLIC cxx_std_23)
target_link_libraries(generalszh_combat_rules PUBLIC generalszh_build_execution engine_gameplay_combat engine_gameplay_navigation
    engine_gameplay_rts_orders engine_gameplay_rts_visibility)
target_link_libraries(generalszh_combat_rules PUBLIC engine_gameplay_spatial generalszh_acquisition_policy)
target_link_libraries(generalszh_combat_rules PUBLIC generalszh_construction_definitions)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(targeting/target_frame.test targeting/impact_target_projection.test targeting/launch_snapshot.test
        systems/targeting_system.test systems/acquisition_system.test systems/poison_cleanse_system.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_combat_rules_tests targeting/target_frame.test targeting/impact_target_projection.test targeting/launch_snapshot.test
        systems/targeting_system.test systems/acquisition_system.test systems/poison_cleanse_system.test)
    target_compile_definitions(generalszh_combat_rules_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_combat_rules_tests PRIVATE generalszh_combat_rules Boost::included_unit_test_framework)
    add_test(NAME generalszh_combat_rules_tests COMMAND generalszh_combat_rules_tests)
    set_source_files_properties(targeting/target_index.test PROPERTIES LANGUAGE CXX)
    target_sources(generalszh_combat_rules_tests PRIVATE targeting/target_index.test)
    set_source_files_properties(systems/poison_input_system.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_poison_input_tests systems/poison_input_system.test)
    target_link_libraries(generalszh_poison_input_tests PRIVATE generalszh_combat_rules Boost::included_unit_test_framework)
    add_test(NAME generalszh_poison_input_tests COMMAND generalszh_poison_input_tests)
    set_source_files_properties(inputs/poison_input.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_poison_input_payload_tests inputs/poison_input.test)
    target_link_libraries(generalszh_poison_input_payload_tests PRIVATE generalszh_combat_rules Boost::included_unit_test_framework)
    add_test(NAME generalszh_poison_input_payload_tests COMMAND generalszh_poison_input_payload_tests)
endif()

# Policy must also be usable by build definitions without importing the combat
# execution target (which itself consumes produced-unit ownership).
add_library(generalszh_acquisition_policy STATIC)
target_sources(generalszh_acquisition_policy PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/acquisition_policy.cppm acquisition/acquisition_policy.cppm)
target_compile_features(generalszh_acquisition_policy PUBLIC cxx_std_23)
target_link_libraries(generalszh_acquisition_policy PUBLIC engine_ecs)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    set_source_files_properties(components/acquisition_policy.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_acquisition_policy_tests components/acquisition_policy.test)
    target_link_libraries(generalszh_acquisition_policy_tests PRIVATE generalszh_acquisition_policy Boost::included_unit_test_framework)
    add_test(NAME generalszh_acquisition_policy_tests COMMAND generalszh_acquisition_policy_tests)
endif()
