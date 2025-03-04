set(TIMINGTEST_SRC
    "StdAfx.cpp"
    "StdAfx.h"
    "timingTest.cpp"
)

add_executable(z_timingtest WIN32)
set_target_properties(z_timingtest PROPERTIES OUTPUT_NAME timingtest)

target_sources(z_timingtest PRIVATE ${TIMINGTEST_SRC})

target_link_libraries(z_timingtest PRIVATE
    winmm
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(z_timingtest PRIVATE /subsystem:console)
endif()
