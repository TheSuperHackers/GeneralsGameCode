# Interface libraries to set common defines and includes, avoid duplication later
add_library(z_wwcommon INTERFACE)

target_link_libraries(z_wwcommon INTERFACE
    core_wwcommon
)

target_include_directories(z_wwcommon INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(WWDownload)

# Helpful interface to bundle the ww modules together.
add_library(z_wwvegas INTERFACE)

target_include_directories(z_wwvegas INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(z_wwvegas INTERFACE
    core_wwvegas
    z_wwdownload
)
