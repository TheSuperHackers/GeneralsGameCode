add_library(generalszh_object_upgrade_content STATIC)
target_sources(generalszh_object_upgrade_content PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    object_upgrade_definition.cppm)
target_compile_features(generalszh_object_upgrade_content PUBLIC cxx_std_23)
set_property(TARGET generalszh_object_upgrade_content PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_object_upgrade_content PUBLIC
    generalszh_content_ini generalszh_upgrade_content generalszh_upgrade_catalog
    generalszh_object_upgrade_creation)

if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(object_upgrade_definition.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_object_upgrade_content_tests object_upgrade_definition.test)
    set_property(TARGET generalszh_object_upgrade_content_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(generalszh_object_upgrade_content_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_object_upgrade_content_tests PRIVATE
        generalszh_object_upgrade_content Boost::included_unit_test_framework)
    add_test(NAME generalszh_object_upgrade_content_tests
        COMMAND generalszh_object_upgrade_content_tests)
endif()
