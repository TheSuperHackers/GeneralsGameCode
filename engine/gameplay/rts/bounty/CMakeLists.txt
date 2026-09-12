add_library(engine_gameplay_rts_bounty STATIC)
target_sources(engine_gameplay_rts_bounty PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/bounty_policy.cppm algorithms/bounty_amount.cppm)
target_compile_features(engine_gameplay_rts_bounty PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_rts_bounty PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_rts_bounty PUBLIC engine_ecs)
if(ENGINE_GAMEPLAY_BUILD_TESTS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(components/bounty_policy.test algorithms/bounty_amount.test PROPERTIES LANGUAGE CXX)
    add_executable(engine_gameplay_rts_bounty_tests
        components/bounty_policy.test algorithms/bounty_amount.test)
    set_property(TARGET engine_gameplay_rts_bounty_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(engine_gameplay_rts_bounty_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(engine_gameplay_rts_bounty_tests PRIVATE
        engine_gameplay_rts_bounty Boost::included_unit_test_framework)
    add_test(NAME engine_gameplay_rts_bounty_tests COMMAND engine_gameplay_rts_bounty_tests)
endif()
