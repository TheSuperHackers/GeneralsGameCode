add_library(generalszh_radar_rules STATIC)
target_sources(generalszh_radar_rules PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
	components/radar_provider_binding.cppm
	definitions/radar_provider_definition.cppm
	policies/zero_hour_radar_policy.cppm)
target_compile_features(generalszh_radar_rules PUBLIC cxx_std_23)
set_property(TARGET generalszh_radar_rules PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(generalszh_radar_rules PUBLIC
	engine_gameplay_rts_radar_modern engine_gameplay_rts_upgrades
	engine_gameplay_rts_upgrades_object_scope
	engine_gameplay_combat generalszh_construction_definitions
	generalszh_build_execution generalszh_power_execution)

if(RTS_BUILD_ZEROHOUR_EXTRAS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)
	foreach(radar_test IN ITEMS components/radar_provider_binding
		definitions/radar_provider_definition policies/zero_hour_radar_policy)
		get_filename_component(radar_stem "${radar_test}" NAME)
		set_source_files_properties("${radar_test}.test" PROPERTIES LANGUAGE CXX)
		add_executable("generalszh_radar_${radar_stem}_tests" "${radar_test}.test")
		set_property(TARGET "generalszh_radar_${radar_stem}_tests" PROPERTY CXX_SCAN_FOR_MODULES ON)
		target_compile_definitions("generalszh_radar_${radar_stem}_tests" PRIVATE BOOST_TEST_INCLUDED)
		target_link_libraries("generalszh_radar_${radar_stem}_tests" PRIVATE
			generalszh_radar_rules Boost::included_unit_test_framework)
		add_test(NAME "generalszh_radar_${radar_stem}_tests"
			COMMAND "generalszh_radar_${radar_stem}_tests")
	endforeach()
endif()
