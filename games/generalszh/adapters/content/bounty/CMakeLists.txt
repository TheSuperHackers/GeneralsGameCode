add_library(generalszh_cash_bounty_content STATIC)
target_sources(generalszh_cash_bounty_content PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    cash_bounty_definition.cppm)
target_compile_features(generalszh_cash_bounty_content PUBLIC cxx_std_23)
set_property(TARGET generalszh_cash_bounty_content PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_cash_bounty_content PUBLIC generalszh_content_ini
    generalszh_cash_bounty_execution generalszh_build_execution engine_gameplay_rts_unlocks)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(cash_bounty_definition.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_cash_bounty_content_tests cash_bounty_definition.test)
    set_property(TARGET generalszh_cash_bounty_content_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(generalszh_cash_bounty_content_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_cash_bounty_content_tests PRIVATE
        generalszh_cash_bounty_content Boost::included_unit_test_framework)
    add_test(NAME generalszh_cash_bounty_content_tests COMMAND generalszh_cash_bounty_content_tests)
endif()
