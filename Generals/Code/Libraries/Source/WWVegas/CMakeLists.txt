# Interface libraries to set common defines and includes, avoid duplication later
add_library(g_wwcommon INTERFACE)

target_link_libraries(g_wwcommon INTERFACE
    d3d8lib
    core_config
    core_utility
    core_wwcommon
    milesstub
    stlport
)

target_include_directories(g_wwcommon INTERFACE
    .
    WW3D2
    WWAudio
)

add_subdirectory(WWAudio)
add_subdirectory(WW3D2)
add_subdirectory(WWDownload)

# Helpful interface to bundle the ww modules together.
add_library(g_wwvegas INTERFACE)

target_include_directories(g_wwvegas INTERFACE
    .
    WW3D2
    WWAudio
    WWDownload
)

target_link_libraries(g_wwvegas INTERFACE
    core_wwvegas
    g_ww3d2
    g_wwdownload
)
