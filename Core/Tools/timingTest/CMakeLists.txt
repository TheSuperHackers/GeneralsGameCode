set(TIMINGTEST_SRC
    "StdAfx.cpp"
    "StdAfx.h"
    "timingTest.cpp"
)

add_executable(core_timingtest WIN32)
set_target_properties(core_timingtest PROPERTIES OUTPUT_NAME timingtest)

target_sources(core_timingtest PRIVATE ${TIMINGTEST_SRC})

target_link_libraries(core_timingtest PRIVATE
    core_config
    corei_always
    winmm
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(core_timingtest PRIVATE /subsystem:console)
endif()
