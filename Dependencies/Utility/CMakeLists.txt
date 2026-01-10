add_library(core_utility INTERFACE)
target_include_directories(core_utility INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_precompile_headers(core_utility INTERFACE
    [["Utility/CppMacros.h"]] # Must be first, to be removed when abandoning VC6
)

add_library(core_utility_no_pch INTERFACE)
target_include_directories(core_utility_no_pch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
