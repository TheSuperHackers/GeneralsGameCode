add_library(generalszh_economy_rules STATIC)
target_sources(generalszh_economy_rules PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
	components/capture_reward_state.cppm components/income_history.cppm
	components/capture_range.cppm components/income_components.cppm
	algorithms/auto_deposit.cppm algorithms/income_history.cppm
	auto_deposit.cppm income_history.cppm transactions/account_transactions.cppm
	income/income_components.cppm capture/capture_batch.cppm systems/capture_system.cppm
	systems/income_prepare_system.cppm systems/income_payout_system.cppm)
target_compile_features(generalszh_economy_rules PUBLIC cxx_std_23)
set_property(TARGET generalszh_economy_rules PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_economy_rules PUBLIC engine_gameplay_rts_economy)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)
	set(economy_rule_tests algorithms/auto_deposit.test algorithms/income_history.test
		components/capture_reward_state.test components/income_history.test
		components/capture_range.test components/income_components.test
		transactions/account_transactions.test capture/capture_batch.test systems/capture_system.test
		systems/income_prepare_system.test systems/income_payout_system.test)
	set_source_files_properties(${economy_rule_tests} PROPERTIES LANGUAGE CXX)
	add_executable(generalszh_economy_rules_tests ${economy_rule_tests})
	set_property(TARGET generalszh_economy_rules_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
	target_compile_definitions(generalszh_economy_rules_tests PRIVATE BOOST_TEST_INCLUDED)
	target_link_libraries(generalszh_economy_rules_tests PRIVATE generalszh_economy_rules Boost::included_unit_test_framework)
	add_test(NAME generalszh_economy_rules_tests COMMAND generalszh_economy_rules_tests)
endif()

# Independent modern account execution; existing legacy owners do not link it.
add_library(generalszh_account_execution STATIC)
target_sources(generalszh_account_execution PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/income_account.cppm components/account_range.cppm
    transactions/account_batch.cppm systems/account_system.cppm income/income_account.cppm)
target_compile_features(generalszh_account_execution PUBLIC cxx_std_23)
set_property(TARGET generalszh_account_execution PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_account_execution PUBLIC generalszh_economy_rules)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    set_source_files_properties(transactions/account_batch.test systems/account_system.test
        components/income_account.test components/account_range.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_account_execution_tests transactions/account_batch.test systems/account_system.test
        components/income_account.test components/account_range.test)
    set_property(TARGET generalszh_account_execution_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(generalszh_account_execution_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_account_execution_tests PRIVATE generalszh_account_execution Boost::included_unit_test_framework)
    add_test(NAME generalszh_account_execution_tests COMMAND generalszh_account_execution_tests)
endif()

add_library(generalszh_economy_execution STATIC)
target_sources(generalszh_economy_execution PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES systems/economy_system.cppm)
target_compile_features(generalszh_economy_execution PUBLIC cxx_std_23)
set_property(TARGET generalszh_economy_execution PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_economy_execution PUBLIC generalszh_account_execution
    generalszh_harvesting_execution generalszh_cash_bounty_batch)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    set_source_files_properties(systems/economy_system.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_economy_execution_tests systems/economy_system.test)
    set_property(TARGET generalszh_economy_execution_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_link_libraries(generalszh_economy_execution_tests PRIVATE generalszh_economy_execution Boost::included_unit_test_framework)
    add_test(NAME generalszh_economy_execution_tests COMMAND generalszh_economy_execution_tests)
endif()
