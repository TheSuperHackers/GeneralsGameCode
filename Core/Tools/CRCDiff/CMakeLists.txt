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

add_executable(core_crcdiff WIN32)
set_target_properties(core_crcdiff PROPERTIES OUTPUT_NAME crcdiff)

target_sources(core_crcdiff PRIVATE ${CRCDIFF_SRC})

target_link_libraries(core_crcdiff PRIVATE
    corei_always
    stlport
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(core_crcdiff PRIVATE /subsystem:console)
endif()
