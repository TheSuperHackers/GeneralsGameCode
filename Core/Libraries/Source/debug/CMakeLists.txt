set(DEBUG_SRC
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
    "debug.h"
    "internal_except.h"
    "internal_io.h"
    "internal.h"
)

# Exclude Windows-only debug utilities on Apple
if(APPLE)
    list(REMOVE_ITEM DEBUG_SRC
        "debug_stack.cpp"
        "debug_io_con.cpp"
        "debug_io_flat.cpp"
        "debug_io_net.cpp"
        "debug_io_ods.cpp"
        "debug_except.cpp"
        "debug_debug.cpp"
        "debug_internal.cpp"
        "debug_cmd.cpp"
        "debug_getdefaultcommands.cpp"
    )
    # Create a dummy .cpp so the static library has at least one object file
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/debug_dummy.cpp "// Dummy source for core_debug on Apple\n")
    list(APPEND DEBUG_SRC "${CMAKE_CURRENT_BINARY_DIR}/debug_dummy.cpp")
endif()

add_library(core_debug STATIC)

target_sources(core_debug PRIVATE ${DEBUG_SRC})

target_include_directories(core_debug INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(core_debug PRIVATE
    [["Utility/CppMacros.h"]] # Must be first, to be removed when abandoning VC6
    <windows.h>
    "debug.h"
    "internal.h"
    "internal_except.h"
    "internal_io.h"
)

target_link_libraries(core_debug PRIVATE
    core_wwcommon
    corei_always
)
