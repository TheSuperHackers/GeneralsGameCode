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

add_executable(gz_crcdiff WIN32)
set_target_properties(gz_crcdiff PROPERTIES OUTPUT_NAME crcdiff)

target_sources(gz_crcdiff PRIVATE ${CRCDIFF_SRC})

target_link_libraries(gz_crcdiff PRIVATE
    gz_config
    gz_utility
    stlport
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(gz_crcdiff PRIVATE /subsystem:console)
endif()
