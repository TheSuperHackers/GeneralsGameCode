# Interface libraries to set common defines and includes, avoid duplication later
add_library(gz_wwcommon INTERFACE)

target_compile_definitions(gz_wwcommon INTERFACE
    #NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_link_libraries(gz_wwcommon INTERFACE
    d3d8lib
    gz_config
    gz_utility
    milesstub
    stlport
)

target_include_directories(gz_wwcommon INTERFACE
    .
    WWDebug
    WWLib
)

add_subdirectory(WWDebug)
add_subdirectory(WWLib)

# Helpful interface to bundle the ww modules together.
add_library(gz_wwvegas INTERFACE)

target_include_directories(gz_wwvegas INTERFACE
    .
    WWDebug
    WWLib
)

target_link_libraries(gz_wwvegas INTERFACE
    gz_wwdebug
    gz_wwlib
)
