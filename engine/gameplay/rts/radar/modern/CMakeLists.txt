# Leaf target for the independent provider-driven radar implementation.  The
# existing radar target above remains the historical aggregate command path.
add_library(engine_gameplay_rts_radar_modern STATIC)
target_sources(engine_gameplay_rts_radar_modern PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES BASE_DIRS .. FILES
	../components/radar_provider_contribution.cppm
	../events/radar_availability_transition.cppm
	../systems/radar_availability_system.cppm)
target_compile_features(engine_gameplay_rts_radar_modern PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_rts_radar_modern PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_rts_radar_modern PUBLIC
	engine_gameplay_rts_radar engine_events)

if(ENGINE_GAMEPLAY_BUILD_TESTS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)
	foreach(radar_test IN ITEMS
		../components/radar_provider_contribution
		../events/radar_availability_transition
		../systems/radar_availability_system)
		get_filename_component(radar_stem "${radar_test}" NAME)
		set_source_files_properties("${radar_test}.test" PROPERTIES LANGUAGE CXX)
		add_executable("engine_radar_modern_${radar_stem}_tests" "${radar_test}.test")
		set_property(TARGET "engine_radar_modern_${radar_stem}_tests" PROPERTY CXX_SCAN_FOR_MODULES ON)
		target_compile_definitions("engine_radar_modern_${radar_stem}_tests" PRIVATE BOOST_TEST_INCLUDED)
		target_link_libraries("engine_radar_modern_${radar_stem}_tests" PRIVATE
			engine_gameplay_rts_radar_modern engine_gameplay_rts_upgrades
			engine_gameplay_combat
			Boost::included_unit_test_framework)
		add_test(NAME "engine_radar_modern_${radar_stem}_tests"
			COMMAND "engine_radar_modern_${radar_stem}_tests")
	endforeach()
endif()
