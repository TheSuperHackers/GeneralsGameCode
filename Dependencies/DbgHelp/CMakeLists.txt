set(DBGHELP_SRC
    DbgHelpGuard.cpp
    DbgHelpGuard.h
    DbgHelpLoader.cpp
    DbgHelpLoader.h
    imagehlp_adapter.h
    minidump_subset.h
)

add_library(deps_dbghelp STATIC)

target_sources(deps_dbghelp PRIVATE ${DBGHELP_SRC})

target_include_directories(deps_dbghelp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(deps_dbghelp PRIVATE
    deps_precompiled # Gives force included headers. Link early.
    deps_utility
    stlport
)
