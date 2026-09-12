set(ENGINE_JOBS_MODULES
	job_system.cppm
)

add_library(engine_jobs STATIC)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(engine_jobs PUBLIC Threads::Threads)

target_sources(engine_jobs PUBLIC
	FILE_SET cxx_modules
	TYPE CXX_MODULES
	FILES ${ENGINE_JOBS_MODULES}
)

target_include_directories(engine_jobs PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/../.."
)

target_compile_features(engine_jobs PUBLIC cxx_std_23)
set_property(TARGET engine_jobs PROPERTY CXX_SCAN_FOR_MODULES ON)

if(TARGET core_config)
	target_link_libraries(engine_jobs PUBLIC core_config)
endif()

if(ENGINE_JOBS_BUILD_TESTS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)

	set_source_files_properties(job_system.test PROPERTIES
		LANGUAGE CXX
	)

	add_executable(engine_jobs_tests
		job_system.test
	)
	set_property(TARGET engine_jobs_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
	target_compile_features(engine_jobs_tests PRIVATE cxx_std_23)
	target_compile_definitions(engine_jobs_tests PRIVATE BOOST_TEST_INCLUDED)
	target_link_libraries(engine_jobs_tests PRIVATE
		engine_jobs
		Boost::included_unit_test_framework
	)

	enable_testing()
	add_test(NAME engine_jobs_tests COMMAND engine_jobs_tests)
endif()
