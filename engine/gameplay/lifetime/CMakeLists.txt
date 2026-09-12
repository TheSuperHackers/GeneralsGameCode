add_library(engine_gameplay_lifetime STATIC)
target_sources(engine_gameplay_lifetime PUBLIC
	FILE_SET cxx_modules TYPE CXX_MODULES FILES
		components/expiration.cppm
		components/expiration_eligibility.cppm
		systems/expiration_system.cppm
)
target_compile_features(engine_gameplay_lifetime PUBLIC cxx_std_23)
set_property(TARGET engine_gameplay_lifetime PROPERTY CXX_SCAN_FOR_MODULES ON)
target_link_libraries(engine_gameplay_lifetime PUBLIC engine_ecs)

if(ENGINE_GAMEPLAY_BUILD_TESTS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)
	set(lifetime_tests components/expiration.test components/expiration_eligibility.test systems/expiration_system.test)
	set_source_files_properties(${lifetime_tests} PROPERTIES LANGUAGE CXX)
	add_executable(engine_gameplay_lifetime_tests ${lifetime_tests})
	set_property(TARGET engine_gameplay_lifetime_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
	target_compile_definitions(engine_gameplay_lifetime_tests PRIVATE BOOST_TEST_INCLUDED)
	target_link_libraries(engine_gameplay_lifetime_tests PRIVATE
		engine_gameplay_lifetime Boost::included_unit_test_framework
	)
	add_test(NAME engine_gameplay_lifetime_tests COMMAND engine_gameplay_lifetime_tests)
endif()
