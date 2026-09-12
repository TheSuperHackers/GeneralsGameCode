add_library(generalszh_task_rules STATIC)
target_sources(generalszh_task_rules PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES systems/task_selection_system.cppm)
target_compile_features(generalszh_task_rules PUBLIC cxx_std_23)
target_link_libraries(generalszh_task_rules PUBLIC generalszh_order_rules engine_gameplay_rts_construction
    engine_gameplay_rts_harvesting engine_gameplay_rts_repair engine_gameplay_containment
    generalszh_capture engine_events)
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    find_package(boost_included_unit_test_framework CONFIG REQUIRED)
    set_source_files_properties(systems/task_selection_system.test PROPERTIES LANGUAGE CXX)
    add_executable(generalszh_task_selection_tests systems/task_selection_system.test)
    target_link_libraries(generalszh_task_selection_tests PRIVATE generalszh_task_rules Boost::included_unit_test_framework)
    add_test(NAME generalszh_task_selection_tests COMMAND generalszh_task_selection_tests)
endif()
