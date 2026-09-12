add_library(generalszh_crate_content STATIC)
target_sources(generalszh_crate_content PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    crate_definition.cppm)
target_compile_features(generalszh_crate_content PUBLIC cxx_std_23)
set_property(TARGET generalszh_crate_content PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_crate_content PUBLIC generalszh_content_ini
    generalszh_crate_definitions engine_gameplay_rts_unlocks generalszh_upgrade_content)

if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(crate_definition.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_crate_content_tests crate_definition.test)
    set_property(TARGET generalszh_crate_content_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(generalszh_crate_content_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_crate_content_tests PRIVATE
        generalszh_crate_content Boost::included_unit_test_framework)
    add_test(NAME generalszh_crate_content_tests COMMAND generalszh_crate_content_tests)
endif()
