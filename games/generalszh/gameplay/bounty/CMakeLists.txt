add_library(generalszh_cash_bounty_batch STATIC)
target_sources(generalszh_cash_bounty_batch PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    inputs/cash_bounty_batch.cppm)
target_compile_features(generalszh_cash_bounty_batch PUBLIC cxx_std_23)
set_property(TARGET generalszh_cash_bounty_batch PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_cash_bounty_batch PUBLIC generalszh_account_execution)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(inputs/cash_bounty_batch.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_cash_bounty_batch_tests inputs/cash_bounty_batch.test)
    set_property(TARGET generalszh_cash_bounty_batch_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(generalszh_cash_bounty_batch_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_cash_bounty_batch_tests PRIVATE
        generalszh_cash_bounty_batch Boost::included_unit_test_framework)
    add_test(NAME generalszh_cash_bounty_batch_tests COMMAND generalszh_cash_bounty_batch_tests)
endif()

add_library(generalszh_cash_bounty_components STATIC)
target_sources(generalszh_cash_bounty_components PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/cash_bounty_cost_binding.cppm)
target_compile_features(generalszh_cash_bounty_components PUBLIC cxx_std_23)
set_property(TARGET generalszh_cash_bounty_components PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_cash_bounty_components PUBLIC engine_ecs)

add_library(generalszh_cash_bounty_execution STATIC)
target_sources(generalszh_cash_bounty_execution PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    definitions/cash_bounty_definition.cppm definitions/cash_bounty_catalog.cppm
    systems/cash_bounty_policy_system.cppm systems/cash_bounty_award_system.cppm)
target_compile_features(generalszh_cash_bounty_execution PUBLIC cxx_std_23)
set_property(TARGET generalszh_cash_bounty_execution PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_cash_bounty_execution PUBLIC generalszh_cash_bounty_components
    generalszh_cash_bounty_batch
    engine_gameplay_rts_bounty engine_gameplay_rts_unlocks engine_gameplay_combat
    generalszh_build_execution generalszh_construction_definitions generalszh_match_rules)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    set_source_files_properties(
        components/cash_bounty_cost_binding.test definitions/cash_bounty_definition.test
        definitions/cash_bounty_catalog.test systems/cash_bounty_policy_system.test
        systems/cash_bounty_award_system.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_cash_bounty_execution_tests
        components/cash_bounty_cost_binding.test definitions/cash_bounty_definition.test
        definitions/cash_bounty_catalog.test systems/cash_bounty_policy_system.test
        systems/cash_bounty_award_system.test)
    set_property(TARGET generalszh_cash_bounty_execution_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(generalszh_cash_bounty_execution_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_cash_bounty_execution_tests PRIVATE
        generalszh_cash_bounty_execution Boost::included_unit_test_framework)
    add_test(NAME generalszh_cash_bounty_execution_tests COMMAND generalszh_cash_bounty_execution_tests)
endif()
