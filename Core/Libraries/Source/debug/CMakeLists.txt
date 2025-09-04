set(DEBUG_SRC
    "_pch.cpp"
    "_pch.h"
    "debug_cmd.cpp"
    "debug_cmd.h"
    "debug_debug.cpp"
    "debug_debug.h"
    "debug_doc.h"
    "debug_except.cpp"
    "debug_getdefaultcommands.cpp"
    "debug_internal.cpp"
    "debug_io_con.cpp"
    "debug_io_flat.cpp"
    "debug_io_net.cpp"
    "debug_io_ods.cpp"
    "debug_io.h"
    "debug_macro.h"
    "debug_stack.cpp"
    "debug_stack.h"
    "debug.cpp"
    "debug.h"
    "internal_except.h"
    "internal_io.h"
    "internal.h"
)

add_library(core_debug STATIC)

target_sources(core_debug PRIVATE ${DEBUG_SRC})

target_include_directories(core_debug INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_debug PRIVATE
    core_wwcommon
    core_config
    corei_always
)
