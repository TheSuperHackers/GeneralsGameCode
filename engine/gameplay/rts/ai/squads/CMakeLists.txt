add_library(engine_gameplay_rts_ai_squads STATIC)
target_sources(engine_gameplay_rts_ai_squads PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
	components/squad_membership.cppm
	components/squad_goal.cppm
	components/squad_goal_application.cppm
	algorithms/squad_anchor.cppm
	algorithms/squad_goal.cppm
	algorithms/squad_order_decision.cppm
	inputs/squad_order_request_batch.cppm)
target_compile_features(engine_gameplay_rts_ai_squads PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_rts_ai_squads PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_rts_ai_squads PUBLIC
	engine_ecs engine_gameplay_navigation engine_gameplay_rts_orders)

if(ENGINE_GAMEPLAY_BUILD_TESTS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)
	set(squad_tests
		components/squad_membership.test
		components/squad_goal.test
		components/squad_goal_application.test
		algorithms/squad_anchor.test
		algorithms/squad_goal.test
		algorithms/squad_order_decision.test
		inputs/squad_order_request_batch.test)
	set_source_files_properties(${squad_tests} PROPERTIES LANGUAGE CXX)
	add_executable(engine_gameplay_rts_ai_squads_tests ${squad_tests})
	set_property(TARGET engine_gameplay_rts_ai_squads_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
	target_compile_definitions(engine_gameplay_rts_ai_squads_tests PRIVATE BOOST_TEST_INCLUDED)
	target_link_libraries(engine_gameplay_rts_ai_squads_tests PRIVATE
		engine_gameplay_rts_ai_squads Boost::included_unit_test_framework)
	add_test(NAME engine_gameplay_rts_ai_squads_tests
		COMMAND engine_gameplay_rts_ai_squads_tests)
endif()
