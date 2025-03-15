# Interface libraries to set common defines and includes, avoid duplication later
add_library(z_wwcommon INTERFACE)

target_compile_definitions(z_wwcommon INTERFACE
    #NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_link_libraries(z_wwcommon INTERFACE
    d3d8lib
    gz_config
    gz_utility
    milesstub
    stlport
)

target_include_directories(z_wwcommon INTERFACE
    .
    WW3D2
    WWAudio
    WWDebug
    WWLib
    WWMath
    WWSaveLoad
    Wwutil
    wwshade
)

add_subdirectory(WWDebug)
add_subdirectory(WWLib)
add_subdirectory(WWMath)
add_subdirectory(Wwutil)
add_subdirectory(WWSaveLoad)
add_subdirectory(WW3D2)
add_subdirectory(WWDownload)
add_subdirectory(wwshade)

# Helpful interface to bundle the ww modules together.
add_library(z_wwvegas INTERFACE)

target_include_directories(z_wwvegas INTERFACE
    .
    WW3D2
    WWDebug
    WWDownload
    WWLib
    WWMath
    WWSaveLoad
    Wwutil
    wwshade
)

target_link_libraries(z_wwvegas INTERFACE
    z_ww3d2
    z_wwdebug
    z_wwdownload
    z_wwlib
    z_wwmath
    z_wwsaveload
    z_wwshade
    z_wwutil
)
