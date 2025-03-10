# Interface libraries to set common defines and includes, avoid duplication later
add_library(wwcommong INTERFACE)

target_compile_definitions(wwcommong INTERFACE
    #NOMINMAX
    WIN32_LEAN_AND_MEAN
    $<$<CONFIG:Debug>:WWDEBUG>
    $<$<CONFIG:Debug>:_DEBUG>
)

target_compile_options(wwcommong INTERFACE
    ${WARNING_FLAGS}
)

target_link_libraries(wwcommong INTERFACE
    stlport
    milesstub
    d3d8lib
)

target_include_directories(wwcommong INTERFACE
    WWDebug
    WWLib
    WWMath
    Wwutil
    WWSaveLoad
    WW3D2
    WWAudio
    .
)

add_subdirectory(WWDebug)
add_subdirectory(WWLib)
add_subdirectory(WWMath)
add_subdirectory(Wwutil)
add_subdirectory(WWSaveLoad)
add_subdirectory(WW3D2)
add_subdirectory(WWDownload)

# Helpful interface to bundle the ww modules together.
add_library(wwvegasg INTERFACE)

target_include_directories(wwvegasg INTERFACE
    WWDebug
    WWLib
    WWMath
    WW3D2
    WWDownload
    WWSaveLoad
    Wwutil
    .
)

target_link_libraries(wwvegasg INTERFACE
    wwdebugg
    wwlibg
    wwmathg
    wwutilg
    wwsaveloadg
    ww3d2g
    wwdownloadg
)
