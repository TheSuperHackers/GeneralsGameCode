cmake_minimum_required(VERSION 4.4.3)

add_library(engine_gameplay_rts_ai STATIC)
target_sources(engine_gameplay_rts_ai PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
	components/force_production_controller.cppm
	definitions/force_production_plan.cppm
	algorithms/select_force_production.cppm)
target_compile_features(engine_gameplay_rts_ai PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_rts_ai PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_rts_ai PUBLIC engine_ecs engine_time)

add_subdirectory(squads)

if(ENGINE_GAMEPLAY_BUILD_TESTS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)
	foreach(ai_test IN ITEMS components/force_production_controller
		definitions/force_production_plan algorithms/select_force_production)
		set_source_files_properties("${ai_test}.test" PROPERTIES LANGUAGE CXX)
		string(REPLACE "/" "_" ai_test_name "${ai_test}")
		add_executable("engine_gameplay_rts_ai_${ai_test_name}_tests" "${ai_test}.test")
		set_property(TARGET "engine_gameplay_rts_ai_${ai_test_name}_tests" PROPERTY CXX_SCAN_FOR_MODULES ON)
		target_compile_definitions("engine_gameplay_rts_ai_${ai_test_name}_tests" PRIVATE BOOST_TEST_INCLUDED)
		target_link_libraries("engine_gameplay_rts_ai_${ai_test_name}_tests" PRIVATE
			engine_gameplay_rts_ai Boost::included_unit_test_framework)
		add_test(NAME "engine_gameplay_rts_ai_${ai_test_name}_tests"
			COMMAND "engine_gameplay_rts_ai_${ai_test_name}_tests")
	endforeach()
endif()
