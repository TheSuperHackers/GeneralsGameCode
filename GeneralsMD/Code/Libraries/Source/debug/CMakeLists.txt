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

add_library(z_debug STATIC)

target_sources(z_debug PRIVATE ${DEBUG_SRC})

target_include_directories(z_debug INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(z_debug PRIVATE
    core_config
    zi_always
)
