add_library(generalszh_lifetime_adapter STATIC)
target_sources(generalszh_lifetime_adapter PUBLIC
	FILE_SET cxx_modules TYPE CXX_MODULES FILES lifetime_store.cppm
)
target_compile_features(generalszh_lifetime_adapter PUBLIC cxx_std_23)
set_property(TARGET generalszh_lifetime_adapter PROPERTY CXX_SCAN_FOR_MODULES ON)
target_include_directories(generalszh_lifetime_adapter PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../../../..")
target_link_libraries(generalszh_lifetime_adapter PUBLIC generalszh_gameplay_state)

if(RTS_BUILD_ZEROHOUR_EXTRAS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)
	set_source_files_properties(lifetime_store.test PROPERTIES LANGUAGE CXX)
	add_executable(generalszh_lifetime_adapter_tests lifetime_store.test)
	set_property(TARGET generalszh_lifetime_adapter_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
	target_compile_definitions(generalszh_lifetime_adapter_tests PRIVATE BOOST_TEST_INCLUDED)
	target_link_libraries(generalszh_lifetime_adapter_tests PRIVATE
		generalszh_lifetime_adapter Boost::included_unit_test_framework
	)
	enable_testing()
	add_test(NAME generalszh_lifetime_adapter_tests COMMAND generalszh_lifetime_adapter_tests)
endif()
