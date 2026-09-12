add_library(engine_gameplay_rts_rank STATIC)
target_sources(engine_gameplay_rts_rank PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
	definitions/rank_definition.cppm
	components/rank_state.cppm components/rank_inbox.cppm
	components/rank_skill_award_modifier.cppm
	inputs/rank_batch.cppm algorithms/rank_initialization.cppm
	algorithms/rank_skill_award.cppm
	systems/rank_system.cppm)
target_compile_features(engine_gameplay_rts_rank PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_rts_rank PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_rts_rank PUBLIC engine_ecs engine_gameplay_rts_unlocks)
if(ENGINE_GAMEPLAY_BUILD_TESTS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)
	set(rank_tests
		definitions/rank_definition.test
		components/rank_state.test
		components/rank_inbox.test
		components/rank_skill_award_modifier.test
		inputs/rank_batch.test
		algorithms/rank_initialization.test
		algorithms/rank_skill_award.test
		systems/rank_system.test)
	set_source_files_properties(${rank_tests} PROPERTIES LANGUAGE CXX)
	add_executable(engine_gameplay_rts_rank_tests ${rank_tests})
	set_property(TARGET engine_gameplay_rts_rank_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
	target_compile_definitions(engine_gameplay_rts_rank_tests PRIVATE BOOST_TEST_INCLUDED)
	target_link_libraries(engine_gameplay_rts_rank_tests PRIVATE engine_gameplay_rts_rank Boost::included_unit_test_framework)
	add_test(NAME engine_gameplay_rts_rank_tests COMMAND engine_gameplay_rts_rank_tests)
endif()
