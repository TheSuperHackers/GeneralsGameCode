add_library(generalszh_collision_definitions STATIC)
target_sources(generalszh_collision_definitions PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    definitions/zero_hour_collision_definition.cppm)
target_compile_features(generalszh_collision_definitions PUBLIC cxx_std_23)
set_property(TARGET generalszh_collision_definitions PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_collision_definitions PUBLIC engine_gameplay_spatial_contacts)

if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(definitions/zero_hour_collision_definition.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_collision_definition_tests definitions/zero_hour_collision_definition.test)
    set_property(TARGET generalszh_collision_definition_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(generalszh_collision_definition_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_collision_definition_tests PRIVATE
        generalszh_collision_definitions Boost::included_unit_test_framework)
    add_test(NAME generalszh_collision_definition_tests COMMAND generalszh_collision_definition_tests)
endif()
