set(DBGHELP_SRC
    DbgHelpGuard.cpp
    DbgHelpGuard.h
    DbgHelpLoader.cpp
    DbgHelpLoader.h
    DbgHelpLoader_minidump.h
)

## dbghelploader

add_library(dbghelploader STATIC)
set_target_properties(dbghelploader PROPERTIES OUTPUT_NAME dbghelploader)

target_sources(dbghelploader PRIVATE ${DBGHELP_SRC})

target_include_directories(dbghelploader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # include "DbgHelpLoader.h" and "DbgHelpGuard.h"
)

target_link_libraries(dbghelploader PUBLIC
    core_utility
    stlport
)
