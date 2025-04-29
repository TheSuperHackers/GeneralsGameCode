set(COMPRESSION_SRC
    Compression.h
    CompressionManager.cpp
    EAC/btreeabout.cpp
    EAC/btreecodex.h
    EAC/btreedecode.cpp
    EAC/btreeencode.cpp
    EAC/codex.h
    EAC/gimex.h
    EAC/huffabout.cpp
    EAC/huffcodex.h
    EAC/huffdecode.cpp
    EAC/huffencode.cpp
    EAC/refabout.cpp
    EAC/refcodex.h
    EAC/refdecode.cpp
    EAC/refencode.cpp
    LZHCompress/NoxCompress.cpp
    LZHCompress/NoxCompress.h
)

add_library(core_compression STATIC)
set_target_properties(core_compression PROPERTIES OUTPUT_NAME compression)

target_sources(core_compression PRIVATE ${COMPRESSION_SRC})

target_include_directories(core_compression INTERFACE
    .
)

target_link_libraries(core_compression PRIVATE
    core_config
    core_utility
    corei_libraries_include
)


target_link_libraries(core_compression PUBLIC
    liblzhl
)

find_package(ZLIB)
if (ZLIB_FOUND)
    # Adds zlib from vcpkg
    target_link_libraries(core_compression PUBLIC ZLIB::ZLIB)
else()
    # Adds zlib from github repository
    include(${CMAKE_SOURCE_DIR}/cmake/zlib.cmake)
    target_link_libraries(core_compression PUBLIC libzlib)
endif()
