# Interface libraries to set common defines and includes, avoid duplication later
add_library(g_wwcommon INTERFACE)

target_link_libraries(g_wwcommon INTERFACE
    d3d8lib
    core_config
    core_utility
    milesstub
    stlport
)

target_include_directories(g_wwcommon INTERFACE
    .
    WW3D2
    WWAudio
    WWDebug
    WWLib
    WWMath
    WWSaveLoad
    Wwutil
)

add_subdirectory(WWDebug)
add_subdirectory(WWLib)
add_subdirectory(WWMath)
add_subdirectory(Wwutil)
add_subdirectory(WWSaveLoad)
add_subdirectory(WW3D2)
add_subdirectory(WWDownload)

# Helpful interface to bundle the ww modules together.
add_library(g_wwvegas INTERFACE)

target_include_directories(g_wwvegas INTERFACE
    .
    WW3D2
    WWDebug
    WWDownload
    WWLib
    WWMath
    WWSaveLoad
    Wwutil
)

target_link_libraries(g_wwvegas INTERFACE
    g_ww3d2
    g_wwdebug
    g_wwdownload
    g_wwlib
    g_wwmath
    g_wwsaveload
    g_wwutil
)
