add_library(generalszh_base_repair STATIC)
target_sources(generalszh_base_repair PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    inputs/manual_repair_batches.cppm systems/base_repair_system.cppm systems/manual_repair_system.cppm)
target_compile_features(generalszh_base_repair PUBLIC cxx_std_23)
set_property(TARGET generalszh_base_repair PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_base_repair PUBLIC engine_gameplay_rts_repair engine_gameplay_combat
    engine_gameplay_navigation engine_gameplay_rts_economy engine_gameplay_containment engine_events
    generalszh_construction_definitions generalszh_sale_components generalszh_capture_definitions)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set(repair_tests systems/base_repair_system.test inputs/manual_repair_batches.test systems/manual_repair_system.test)
    set_source_files_properties(${repair_tests} PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_base_repair_tests ${repair_tests})
    target_compile_definitions(generalszh_base_repair_tests PRIVATE BOOST_TEST_INCLUDED)
    target_link_libraries(generalszh_base_repair_tests PRIVATE generalszh_base_repair engine_gameplay_rts_harvesting
        Boost::included_unit_test_framework)
    add_test(NAME generalszh_base_repair_tests COMMAND generalszh_base_repair_tests)
endif()
