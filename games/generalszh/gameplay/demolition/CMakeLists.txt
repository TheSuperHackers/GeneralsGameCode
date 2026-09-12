add_library(generalszh_demolition_components STATIC)
target_sources(generalszh_demolition_components PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/demolition_trap.cppm)
target_compile_features(generalszh_demolition_components PUBLIC cxx_std_23)
set_property(TARGET generalszh_demolition_components PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_demolition_components PUBLIC engine_ecs)

add_library(generalszh_demolition_definitions STATIC)
target_sources(generalszh_demolition_definitions PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    definitions/demolition_trap_definition.cppm)
target_compile_features(generalszh_demolition_definitions PUBLIC cxx_std_23)
set_property(TARGET generalszh_demolition_definitions PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_demolition_definitions PUBLIC generalszh_demolition_components engine_gameplay_combat_death engine_time)

add_library(generalszh_demolition STATIC)
target_sources(generalszh_demolition PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    inputs/demolition_trap_inputs.cppm
    algorithms/demolition_source_projection.cppm
    algorithms/demolition_target_index.cppm
    systems/demolition_trap_system.cppm
    setup/demolition_registration.cppm)
target_compile_features(generalszh_demolition PUBLIC cxx_std_23)
set_property(TARGET generalszh_demolition PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_demolition PUBLIC generalszh_demolition_components generalszh_demolition_definitions
    generalszh_build_execution generalszh_construction_definitions generalszh_sale_components engine_gameplay_combat_death
    engine_gameplay_navigation engine_gameplay_containment)

if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    foreach(source IN ITEMS components/demolition_trap definitions/demolition_trap_definition
        inputs/demolition_trap_inputs algorithms/demolition_source_projection algorithms/demolition_target_index
        systems/demolition_trap_system setup/demolition_registration)
        set_source_files_properties(${source}.test PROPERTIES LANGUAGE CXX)
        get_filename_component(stem ${source} NAME)
        add_executable(generalszh_${stem}_tests ${source}.test)
        set_property(TARGET generalszh_${stem}_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
        target_compile_definitions(generalszh_${stem}_tests PRIVATE BOOST_TEST_INCLUDED)
        target_link_libraries(generalszh_${stem}_tests PRIVATE generalszh_demolition Boost::included_unit_test_framework)
        add_test(NAME generalszh_${stem}_tests COMMAND generalszh_${stem}_tests)
    endforeach()
endif()
