add_library(generalszh_crate_definitions STATIC)
target_sources(generalszh_crate_definitions PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/crate_state.cppm
    definitions/crate_definition.cppm definitions/account_pickup_policy.cppm
    definitions/collector_definition.cppm
    algorithms/crate_veterancy_projection.cppm
    inputs/crate_pickup_batch.cppm inputs/crate_reward_batch.cppm)
target_compile_features(generalszh_crate_definitions PUBLIC cxx_std_23)
set_property(TARGET generalszh_crate_definitions PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_crate_definitions PUBLIC
    engine_ecs engine_gameplay_navigation engine_gameplay_progression
    engine_gameplay_pickups engine_gameplay_rts_unlocks generalszh_account_execution)

add_library(generalszh_crate_execution STATIC)
target_sources(generalszh_crate_execution PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    systems/crate_claim_system.cppm systems/crate_reward_system.cppm)
target_compile_features(generalszh_crate_execution PUBLIC cxx_std_23)
set_property(TARGET generalszh_crate_execution PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_crate_execution PUBLIC generalszh_crate_definitions
    engine_gameplay_combat generalszh_build_execution
    generalszh_construction_definitions generalszh_upgrade_state
    engine_gameplay_spatial_contacts)

if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set(crate_definition_tests
        components/crate_state.test
        definitions/crate_definition.test
        definitions/account_pickup_policy.test
        definitions/collector_definition.test
        algorithms/crate_veterancy_projection.test
        inputs/crate_pickup_batch.test
        inputs/crate_reward_batch.test)
    set_source_files_properties(${crate_definition_tests} PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_crate_definition_tests ${crate_definition_tests})
    set_property(TARGET generalszh_crate_definition_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(generalszh_crate_definition_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_crate_definition_tests PRIVATE
        generalszh_crate_definitions Boost::included_unit_test_framework)
    add_test(NAME generalszh_crate_definition_tests COMMAND generalszh_crate_definition_tests)

    set_source_files_properties(systems/crate_claim_system.test systems/crate_reward_system.test
        PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_crate_execution_tests
        systems/crate_claim_system.test systems/crate_reward_system.test)
    set_property(TARGET generalszh_crate_execution_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(generalszh_crate_execution_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_crate_execution_tests PRIVATE
        generalszh_crate_execution Boost::included_unit_test_framework)
    add_test(NAME generalszh_crate_execution_tests COMMAND generalszh_crate_execution_tests)
endif()
