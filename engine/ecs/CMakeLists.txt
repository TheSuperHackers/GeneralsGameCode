cmake_minimum_required(VERSION 4.4.3)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	project(engine_ecs_check LANGUAGES CXX)
endif()

set(ENGINE_ECS_MODULES
	core/component_registry.cppm
	core/entity.cppm
	core/world.cppm
	commands/command_buffer.cppm
	query/access.cppm
	query/query.cppm
	query/query_cache.cppm
	system/system.cppm
	scheduler/scheduler.cppm
	storage/archetype.cppm
	storage/chunk.cppm
	storage/chunk_layout.cppm
	storage/signature.cppm
	ecs.cppm
)

set(ENGINE_ECS_TESTS
	core/component_registry.test
	core/entity.test
	core/world.test
	commands/command_buffer.test
	system/system.test
	scheduler/scheduler.test
	query/access.test
	query/query.test
	query/query_cache.test
	storage/archetype.test
	storage/chunk.test
	storage/chunk_layout.test
	storage/signature.test
	ecs.test
)

if(NOT TARGET engine_jobs)
	set(ENGINE_JOBS_BUILD_TESTS ${ENGINE_ECS_BUILD_TESTS})
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../jobs" "${CMAKE_CURRENT_BINARY_DIR}/../engine_jobs")
endif()

add_library(engine_ecs STATIC)

if(NOT TARGET engine_time)
	set(ENGINE_TIME_BUILD_TESTS ${ENGINE_ECS_BUILD_TESTS})
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../time" "${CMAKE_CURRENT_BINARY_DIR}/../engine_time")
endif()
target_link_libraries(engine_ecs PUBLIC engine_time)

target_sources(engine_ecs PUBLIC
	FILE_SET cxx_modules
	TYPE CXX_MODULES
	FILES ${ENGINE_ECS_MODULES}
)

target_include_directories(engine_ecs PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/../.."
)

target_compile_features(engine_ecs PUBLIC cxx_std_23)
set_property(TARGET engine_ecs PROPERTY CXX_SCAN_FOR_MODULES ON)

# core_config carries the repository-wide warning, runtime, and configuration policy.
# ECS has no dependency on legacy game headers or legacy gameplay types.
if(TARGET core_config)
	target_link_libraries(engine_ecs PUBLIC
		engine_jobs
		core_config
	)
else()
	target_link_libraries(engine_ecs PUBLIC
		engine_jobs
	)
endif()

if(ENGINE_ECS_BUILD_TESTS)
	find_package(boost_included_unit_test_framework CONFIG REQUIRED)

	set_source_files_properties(${ENGINE_ECS_TESTS} PROPERTIES
		LANGUAGE CXX
	)

	add_executable(engine_ecs_tests
		${ENGINE_ECS_TESTS}
	)
	set_property(TARGET engine_ecs_tests PROPERTY CXX_SCAN_FOR_MODULES ON)
	target_compile_features(engine_ecs_tests PRIVATE cxx_std_23)
	target_compile_definitions(engine_ecs_tests PRIVATE BOOST_TEST_INCLUDED)
	target_link_libraries(engine_ecs_tests PRIVATE
		engine_ecs
		Boost::included_unit_test_framework
	)

	enable_testing()
	add_test(NAME engine_ecs_tests COMMAND engine_ecs_tests)
endif()
