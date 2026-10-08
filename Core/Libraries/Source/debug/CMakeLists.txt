set(DEBUG_SRC
    "Include/debug/debug_cmd.h"
    "Include/debug/debug_debug.h"
    "Include/debug/debug_doc.h"
    "Include/debug/debug_io.h"
    "Include/debug/debug_macro.h"
    "Include/debug/debug_stack.h"
    "Include/debug/debug.h"
    "debug_cmd.cpp"
    "debug_debug.cpp"
    "debug_except.cpp"
    "debug_getdefaultcommands.cpp"
    "debug_internal.cpp"
    "debug_io_con.cpp"
    "debug_io_flat.cpp"
    "debug_io_net.cpp"
    "debug_io_ods.cpp"
    "debug_stack.cpp"
    "internal_except.h"
    "internal_io.h"
    "internal.h"
)

add_library(core_debug STATIC)

target_sources(core_debug PRIVATE ${DEBUG_SRC})

target_include_directories(core_debug PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

target_precompile_headers(core_debug PRIVATE
    [["Precompiled/precompiled.h"]] # Must be first
    "Include/debug/debug.h"
    "internal.h"
    "internal_except.h"
    "internal_io.h"
    <windows.h>
)

target_link_libraries(core_debug PRIVATE
    depsi_always # Gives force included headers. Link early.
    core_config
    deps_utility
    stlport
)
