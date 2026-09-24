set(USP10_SRC
    Usp10Loader.cpp
    Usp10Loader.h
    usp10_adapter.h
    usp10_subset.h
)

## usp10loader

add_library(usp10loader STATIC)
set_target_properties(usp10loader PROPERTIES OUTPUT_NAME usp10loader)

target_sources(usp10loader PRIVATE ${USP10_SRC})

target_include_directories(usp10loader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # include "Usp10Loader.h" and "usp10_adapter.h"
)

target_link_libraries(usp10loader PRIVATE
    core_precompiled # Gives force included headers. Link early.
    core_utility
)
