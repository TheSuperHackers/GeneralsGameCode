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

add_executable(g_crcdiff WIN32)
set_target_properties(g_crcdiff PROPERTIES OUTPUT_NAME crcdiff)

target_sources(g_crcdiff PRIVATE ${CRCDIFF_SRC})

target_link_libraries(g_crcdiff PRIVATE
    gz_config
    gz_utility
    stlport
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(g_crcdiff PRIVATE /subsystem:console)
endif()
