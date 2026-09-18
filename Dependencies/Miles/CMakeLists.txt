set(MILES_SRC
    mss/mss.h
    MilesLoader.cpp
    MilesLoader.h
)

## milesloader

add_library(milesloader STATIC)
set_target_properties(milesloader PROPERTIES OUTPUT_NAME milesloader)

target_sources(milesloader PRIVATE ${MILES_SRC})

target_include_directories(milesloader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}     # include "mss/mss.h" and "MilesLoader.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/mss # include "mss.h"
)

target_link_libraries(milesloader PRIVATE
    core_utility
)
