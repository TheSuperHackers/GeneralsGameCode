add_library(engine_gameplay_combat_death STATIC)
target_sources(engine_gameplay_combat_death PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/death_weapon_state.cppm
    definitions/death_weapon_definition.cppm
    systems/death_weapon_system.cppm
    setup/death_weapon_registration.cppm)
target_compile_features(engine_gameplay_combat_death PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_combat_death PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_combat_death PUBLIC engine_gameplay_combat engine_ecs engine_time)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    foreach(source IN ITEMS components/death_weapon_state definitions/death_weapon_definition systems/death_weapon_system
        setup/death_weapon_registration)
        set_source_files_properties(${source}.test PROPERTIES LANGUAGE CXX)
        get_filename_component(stem ${source} NAME)
        add_executable(engine_${stem}_tests ${source}.test)
        set_property(TARGET engine_${stem}_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
        target_compile_definitions(engine_${stem}_tests PRIVATE BOOST_TEST_INCLUDED)
        target_link_libraries(engine_${stem}_tests PRIVATE engine_gameplay_combat_death Boost::included_unit_test_framework)
        add_test(NAME engine_${stem}_tests COMMAND engine_${stem}_tests)
    endforeach()
endif()
