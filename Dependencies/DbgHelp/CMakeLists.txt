set(DBGHELP_SRC
    DbgHelpGuard.cpp
    DbgHelpGuard.h
    DbgHelpLoader.cpp
    DbgHelpLoader.h
    imagehlp_adapter.h
    minidump_subset.h
)

## dbghelploader

add_library(dbghelploader STATIC)
set_target_properties(dbghelploader PROPERTIES OUTPUT_NAME dbghelploader)

target_sources(dbghelploader PRIVATE ${DBGHELP_SRC})

target_include_directories(dbghelploader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # include "DbgHelpLoader.h", "DbgHelpGuard.h" and "imagehlp_adapter.h"
)

target_link_libraries(dbghelploader PRIVATE
    core_precompiled # Gives force included headers. Link early.
    core_utility
    stlport
)
