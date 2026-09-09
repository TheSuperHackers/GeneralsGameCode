set(BINK_SRC
    bink.h
    BinkLoader.cpp
    BinkLoader.h
)

## binkloader

add_library(binkloader STATIC)
set_target_properties(binkloader PROPERTIES OUTPUT_NAME binkloader)

target_sources(binkloader PRIVATE ${BINK_SRC})

target_include_directories(binkloader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # include "bink.h" and "BinkLoader.h"
)

target_link_libraries(binkloader PRIVATE
    core_utility
)
