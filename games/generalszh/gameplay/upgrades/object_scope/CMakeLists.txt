add_library(generalszh_object_upgrade_creation STATIC)
target_sources(generalszh_object_upgrade_creation PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    definitions/object_upgrade_creation_definition.cppm
    inputs/object_upgrade_creation.cppm)
target_compile_features(generalszh_object_upgrade_creation PUBLIC cxx_std_23)
set_property(TARGET generalszh_object_upgrade_creation PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_object_upgrade_creation PUBLIC
    engine_gameplay_rts_upgrades_object_scope)

if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(
        definitions/object_upgrade_creation_definition.test
        inputs/object_upgrade_creation.test
        PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_object_upgrade_creation_tests
        definitions/object_upgrade_creation_definition.test
        inputs/object_upgrade_creation.test)
    set_property(TARGET generalszh_object_upgrade_creation_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(generalszh_object_upgrade_creation_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_object_upgrade_creation_tests PRIVATE
        generalszh_object_upgrade_creation Boost::included_unit_test_framework)
    add_test(NAME generalszh_object_upgrade_creation_tests
        COMMAND generalszh_object_upgrade_creation_tests)
endif()
