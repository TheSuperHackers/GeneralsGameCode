add_subdirectory(damage)
add_library(engine_gameplay_combat STATIC)
target_sources(engine_gameplay_combat PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    systems/health_system.cppm systems/health_input_system.cppm inputs/health_input_system.cppm systems/weapon_system.cppm systems/projectile_system.cppm
    systems/healing_system.cppm systems/periodic_damage_system.cppm systems/timed_life_system.cppm lifetime/timed_life_system.cppm
    components/health.cppm components/healing.cppm components/weapon.cppm components/projectile.cppm
    components/periodic_damage.cppm definitions/weapon_definition.cppm definitions/periodic_damage_definition.cppm)
target_compile_features(engine_gameplay_combat PUBLIC cxx_std_23)
target_link_libraries(engine_gameplay_combat PUBLIC engine_ecs engine_gameplay_lifetime engine_gameplay_containment engine_gameplay_spatial)
target_link_libraries(engine_gameplay_combat PUBLIC engine_gameplay_damage)
if(ENGINE_GAMEPLAY_BUILD_TESTS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set(combat_tests systems/health_system.test systems/health_input_system.test systems/weapon_system.test systems/projectile_system.test
        systems/healing_system.test systems/periodic_damage_system.test systems/timed_life_system.test
        components/health.test components/healing.test components/weapon.test components/projectile.test
        components/periodic_damage.test definitions/weapon_definition.test definitions/periodic_damage_definition.test)
    set_source_files_properties(${combat_tests} PROPERTIES LANGUAGE CXX)
    add_executable(engine_gameplay_combat_tests ${combat_tests})
    target_compile_definitions(engine_gameplay_combat_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(engine_gameplay_combat_tests PRIVATE engine_gameplay_combat Boost::included_unit_test_framework)
    add_test(NAME engine_gameplay_combat_tests COMMAND engine_gameplay_combat_tests)
endif()
add_subdirectory(regeneration)
add_subdirectory(death)
