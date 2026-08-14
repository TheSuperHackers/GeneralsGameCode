add_executable(z_runtime_regression_tests WIN32)

target_sources(z_runtime_regression_tests PRIVATE
    RuntimeRegressionTests.cpp
)

target_link_libraries(z_runtime_regression_tests PRIVATE
    z_gameengine
    zi_always
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(z_runtime_regression_tests PRIVATE /subsystem:console)
endif()
