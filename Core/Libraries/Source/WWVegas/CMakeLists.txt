# Interface libraries to set common defines and includes, avoid duplication later
add_library(core_wwcommon INTERFACE)

target_compile_definitions(core_wwcommon INTERFACE
    #NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_link_libraries(core_wwcommon INTERFACE
    d3d8lib
    core_config
    core_utility
    milesstub
    stlport
)

target_include_directories(core_wwcommon INTERFACE
    .
    WWDebug
    WWLib
)

# add_subdirectory(WWAudio)
add_subdirectory(WWDebug)
add_subdirectory(WWLib)
# add_subdirectory(WWMath)
# add_subdirectory(Wwutil)
# add_subdirectory(WWSaveLoad)
# add_subdirectory(WW3D2)
# add_subdirectory(WWDownload)
# add_subdirectory(wwshade)

# Helpful interface to bundle the ww modules together.
add_library(core_wwvegas INTERFACE)

target_include_directories(core_wwvegas INTERFACE
    .
    #WW3D2
    WWDebug
    #WWDownload
    WWLib
    #WWMath
    #WWSaveLoad
    #Wwutil
    #wwshade
)

target_link_libraries(core_wwvegas INTERFACE
    # core_ww3d2
    core_wwdebug
    # core_wwdownload
    core_wwlib
    # core_wwmath
    # core_wwsaveload
    # core_wwshade
    # core_wwutil
)
