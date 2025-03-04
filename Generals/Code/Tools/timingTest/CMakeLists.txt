set(TIMINGTEST_SRC
    "StdAfx.cpp"
    "StdAfx.h"
    "timingTest.cpp"
)

add_executable(g_timingtest WIN32)
set_target_properties(g_timingtest PROPERTIES OUTPUT_NAME timingtest)

target_sources(g_timingtest PRIVATE ${TIMINGTEST_SRC})

target_link_libraries(g_timingtest PRIVATE
    winmm
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(g_timingtest PRIVATE /subsystem:console)
endif()
