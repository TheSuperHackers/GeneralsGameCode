# Contains the build settings that the Dependencies and Core targets share.
# Settings that are specific to the game belong to core_config instead.
add_library(deps_config INTERFACE)

add_library(core_config INTERFACE)

target_link_libraries(core_config INTERFACE deps_config)

include(${CMAKE_CURRENT_LIST_DIR}/config-build.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/config-macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/config-retail.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/config-debug.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/config-memory.cmake)
