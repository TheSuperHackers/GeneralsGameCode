add_library(generalszh_concealment_definitions STATIC)
target_sources(generalszh_concealment_definitions PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    definitions/stealth_policy.cppm)
target_compile_features(generalszh_concealment_definitions PUBLIC cxx_std_23)
set_property(TARGET generalszh_concealment_definitions PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_concealment_definitions PUBLIC engine_ecs)

add_library(generalszh_concealment_components STATIC)
target_sources(generalszh_concealment_components PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/stealth_binding.cppm)
target_compile_features(generalszh_concealment_components PUBLIC cxx_std_23)
set_property(TARGET generalszh_concealment_components PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_concealment_components PUBLIC engine_ecs generalszh_concealment_definitions)

add_library(generalszh_concealment_rules STATIC)
target_sources(generalszh_concealment_rules PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    systems/stealth_policy_system.cppm)
target_compile_features(generalszh_concealment_rules PUBLIC cxx_std_23)
set_property(TARGET generalszh_concealment_rules PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_concealment_rules PUBLIC generalszh_concealment_definitions
    generalszh_concealment_components generalszh_build_execution engine_gameplay_concealment
    engine_gameplay_combat engine_gameplay_navigation engine_ecs)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set(concealment_tests definitions/stealth_policy.test components/stealth_binding.test systems/stealth_policy_system.test)
    set_source_files_properties(${concealment_tests} PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_concealment_rules_tests ${concealment_tests})
    set_property(TARGET generalszh_concealment_rules_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(generalszh_concealment_rules_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_concealment_rules_tests PRIVATE generalszh_concealment_rules
        Boost::included_unit_test_framework)
    add_test(NAME generalszh_concealment_rules_tests COMMAND generalszh_concealment_rules_tests)
endif()
