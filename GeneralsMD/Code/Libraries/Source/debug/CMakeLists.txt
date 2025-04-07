set(DEBUG_SRC
    "_pch.cpp"
    "_pch.h"
    "debug_io.h"
    "debug_io_con.cpp"
    "debug_io_flat.cpp"
    "debug_io_net.cpp"
    "debug_io_ods.cpp"
    "internal_io.h"
    "debug_except.cpp"
    "internal_except.h"
    "debug_stack.cpp"
    "debug_stack.h"
    "debug_cmd.cpp"
    "debug_cmd.h"
    "debug_getdefaultcommands.cpp"
    "debug.cpp"
    "debug.h"
    "debug_debug.cpp"
    "debug_debug.h"
    "debug_doc.h"
    "debug_internal.cpp"
    "debug_macro.h"
    "internal.h"
)

add_library(z_debug STATIC)

target_sources(z_debug PRIVATE ${DEBUG_SRC})

target_include_directories(z_debug INTERFACE
    .
)

target_link_libraries(z_debug PRIVATE
    gz_config
    zi_always
)
