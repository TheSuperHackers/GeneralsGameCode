add_library(engine_gameplay_rts_unlocks STATIC)
target_sources(engine_gameplay_rts_unlocks PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
	definitions/unlock_definition.cppm
	definitions/unlock_catalog.cppm
	components/unlock_state.cppm
	components/unlock_inbox.cppm
	inputs/unlock_batch.cppm
	systems/unlock_system.cppm)
target_compile_features(engine_gameplay_rts_unlocks PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_rts_unlocks PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_rts_unlocks PUBLIC engine_ecs)
if(ENGINE_GAMEPLAY_BUILD_TESTS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)
	set(unlock_tests
		definitions/unlock_definition.test
		definitions/unlock_catalog.test
		components/unlock_state.test
		components/unlock_inbox.test
		inputs/unlock_batch.test
		systems/unlock_system.test)
	set_source_files_properties(${unlock_tests} PROPERTIES LANGUAGE CXX)
	add_executable(engine_gameplay_rts_unlocks_tests ${unlock_tests})
	set_property(TARGET engine_gameplay_rts_unlocks_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
	target_compile_definitions(engine_gameplay_rts_unlocks_tests PRIVATE BOOST_TEST_INCLUDED)
	target_link_libraries(engine_gameplay_rts_unlocks_tests PRIVATE engine_gameplay_rts_unlocks Boost::included_unit_test_framework)
	add_test(NAME engine_gameplay_rts_unlocks_tests COMMAND engine_gameplay_rts_unlocks_tests)
endif()
