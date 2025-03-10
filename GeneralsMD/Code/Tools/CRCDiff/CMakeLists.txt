set(CRCDIFF_SRC
    "CRCDiff.cpp"
    "debug.cpp"
    "debug.h"
    "expander.cpp"
    "expander.h"
    "KVPair.cpp"
    "KVPair.h"
    "misc.h"
)

add_executable(crcdiff WIN32)

target_link_libraries(crcdiff PRIVATE
    stlport
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(crcdiff PRIVATE /subsystem:console)
endif()

target_sources(crcdiff PRIVATE ${CRCDIFF_SRC})
