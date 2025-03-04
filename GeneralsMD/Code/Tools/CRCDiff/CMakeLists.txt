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

add_executable(z_crcdiff WIN32)
set_target_properties(z_crcdiff PROPERTIES OUTPUT_NAME crcdiff)

target_sources(z_crcdiff PRIVATE ${CRCDIFF_SRC})

target_link_libraries(z_crcdiff PRIVATE
    stlport
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(z_crcdiff PRIVATE /subsystem:console)
endif()
