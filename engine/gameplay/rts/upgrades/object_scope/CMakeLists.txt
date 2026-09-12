add_library(engine_gameplay_rts_upgrades_object_scope STATIC)
target_sources(engine_gameplay_rts_upgrades_object_scope PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/object_upgrade_scope.cppm
    inputs/object_upgrade_batch.cppm
    systems/object_upgrade_system.cppm)
target_compile_features(engine_gameplay_rts_upgrades_object_scope PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_rts_upgrades_object_scope PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_rts_upgrades_object_scope PUBLIC
    engine_ecs engine_gameplay_rts_upgrades)

if(ENGINE_GAMEPLAY_BUILD_TESTS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(
        components/object_upgrade_scope.test
        inputs/object_upgrade_batch.test
        systems/object_upgrade_system.test
        PROPERTIES LANGUAGE CXX)
    add_executable(engine_gameplay_rts_object_upgrade_scope_tests
        components/object_upgrade_scope.test
        inputs/object_upgrade_batch.test
        systems/object_upgrade_system.test)
    set_property(TARGET engine_gameplay_rts_object_upgrade_scope_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(engine_gameplay_rts_object_upgrade_scope_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(engine_gameplay_rts_object_upgrade_scope_tests PRIVATE
        engine_gameplay_rts_upgrades_object_scope Boost::included_unit_test_framework)
    add_test(NAME engine_gameplay_rts_object_upgrade_scope_tests
        COMMAND engine_gameplay_rts_object_upgrade_scope_tests)
endif()
