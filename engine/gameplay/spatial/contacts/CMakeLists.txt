add_library(engine_gameplay_spatial_contacts STATIC)
target_sources(engine_gameplay_spatial_contacts PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/contact_geometry.cppm components/contact_participation.cppm
    events/contact_pair.cppm algorithms/contact_geometry.cppm algorithms/contact_intersection.cppm
    algorithms/contact_participation.cppm
    systems/contact_system.cppm)
target_compile_features(engine_gameplay_spatial_contacts PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_spatial_contacts PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_spatial_contacts PUBLIC engine_ecs engine_events
    engine_gameplay_combat engine_gameplay_navigation engine_gameplay_spatial)

if(ENGINE_GAMEPLAY_BUILD_TESTS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set(contact_tests
        components/contact_geometry.test components/contact_participation.test
        events/contact_pair.test algorithms/contact_geometry.test algorithms/contact_intersection.test
        algorithms/contact_participation.test
        systems/contact_system.test)
    set_source_files_properties(${contact_tests} PROPERTIES LANGUAGE CXX)
    add_executable(engine_gameplay_spatial_contacts_tests ${contact_tests})
    set_property(TARGET engine_gameplay_spatial_contacts_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(engine_gameplay_spatial_contacts_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(engine_gameplay_spatial_contacts_tests PRIVATE
        engine_gameplay_spatial_contacts Boost::included_unit_test_framework)
    add_test(NAME engine_gameplay_spatial_contacts_tests COMMAND engine_gameplay_spatial_contacts_tests)
endif()
