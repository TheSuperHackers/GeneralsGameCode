add_library(engine_gameplay_rts_construction STATIC)
target_sources(engine_gameplay_rts_construction PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
    components/construction_components.cppm observations/builder_frame.cppm
    algorithms/builder_routing.cppm)
target_compile_features(engine_gameplay_rts_construction PUBLIC cxx_std_23)
target_link_libraries(engine_gameplay_rts_construction PUBLIC
    engine_gameplay_rts_production engine_gameplay_navigation engine_gameplay_combat)
if(ENGINE_GAMEPLAY_BUILD_TESTS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    foreach(source IN ITEMS components/construction_components.test observations/builder_frame.test
        algorithms/builder_routing.test)
        get_filename_component(stem "${source}" NAME_WE)
        set_source_files_properties(${source} PROPERTIES LANGUAGE CXX)
        add_executable(engine_${stem}_tests ${source})
        target_link_libraries(engine_${stem}_tests PRIVATE
            engine_gameplay_rts_construction Boost::included_unit_test_framework)
        add_test(NAME engine_${stem}_tests COMMAND engine_${stem}_tests)
    endforeach()
endif()
