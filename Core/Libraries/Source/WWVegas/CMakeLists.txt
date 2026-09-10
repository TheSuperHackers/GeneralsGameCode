# Interface libraries to set common defines and includes, avoid duplication later
add_library(core_wwcommon INTERFACE)

target_compile_definitions(core_wwcommon INTERFACE
    #NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_link_libraries(core_wwcommon INTERFACE
    milesstub
)

target_include_directories(core_wwcommon INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(WWDebug)
add_subdirectory(WWDownload)
add_subdirectory(WWLib)
add_subdirectory(WWMath)
add_subdirectory(WWSaveLoad)
add_subdirectory(WWStub)

# Helpful interface to bundle the ww modules together.
add_library(core_wwvegas INTERFACE)

target_include_directories(core_wwvegas INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_wwvegas INTERFACE
    # core_wwaudio
    core_wwdebug
    # core_wwdownload
    core_wwlib
    core_wwmath
    core_wwsaveload
)
