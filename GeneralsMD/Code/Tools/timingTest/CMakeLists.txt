set(TIMINGTEST_SRC
    "StdAfx.cpp"
    "StdAfx.h"
    "timingTest.cpp"
)

add_executable(timingtest WIN32)

target_link_libraries(timingtest PRIVATE
    winmm
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(timingtest PRIVATE /subsystem:console)
endif()

target_sources(timingtest PRIVATE ${TIMINGTEST_SRC})
