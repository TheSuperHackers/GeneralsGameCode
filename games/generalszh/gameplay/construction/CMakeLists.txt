add_library(generalszh_construction_definitions STATIC)
target_sources(generalszh_construction_definitions PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/structure.cppm definitions/building_definition.cppm definitions/building_catalog.cppm)
target_compile_features(generalszh_construction_definitions PUBLIC cxx_std_23)
set_property(TARGET generalszh_construction_definitions PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_construction_definitions PUBLIC engine_gameplay_rts_construction engine_gameplay_rts_repair
    generalszh_garrison_definitions generalszh_demolition_definitions generalszh_demolition_components
    engine_gameplay_rts_visibility generalszh_cash_bounty_components)
target_link_libraries(generalszh_construction_definitions PUBLIC engine_gameplay_damage)
add_library(generalszh_construction STATIC)
target_sources(generalszh_construction PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/construction_health.cppm components/construction_admission.cppm
    algorithms/completed_structure.cppm
    inputs/construction_batches.cppm systems/construction_placement_system.cppm
    systems/builder_task_system.cppm systems/construction_system.cppm
    setup/construction_registration.cppm)
target_compile_features(generalszh_construction PUBLIC cxx_std_23)
target_link_libraries(generalszh_construction PUBLIC
    generalszh_construction_definitions generalszh_build_execution generalszh_match_rules
    engine_gameplay_rts_economy engine_gameplay_rts_power_execution generalszh_demolition_components)
target_link_libraries(generalszh_construction PUBLIC generalszh_harvesting_execution generalszh_sale_components generalszh_rally_components)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    foreach(source IN ITEMS components/structure.test definitions/building_definition.test definitions/building_catalog.test
        components/construction_health.test components/construction_admission.test
        algorithms/completed_structure.test
        inputs/construction_batches.test systems/construction_placement_system.test
        systems/builder_task_system.test systems/construction_system.test
        setup/construction_registration.test)
        get_filename_component(stem "${source}" NAME_WE)
        set_source_files_properties(${source} PROPERTIES LANGUAGE CXX)
        add_executable(generalszh_${stem}_tests ${source})
        target_link_libraries(generalszh_${stem}_tests PRIVATE generalszh_construction Boost::included_unit_test_framework)
        add_test(NAME generalszh_${stem}_tests COMMAND generalszh_${stem}_tests)
    endforeach()
endif()
