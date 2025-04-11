set(COMPRESSION_SRC
    EAC/btreeabout.cpp
    EAC/btreedecode.cpp
    EAC/btreeencode.cpp
    EAC/huffabout.cpp
    EAC/huffdecode.cpp
    EAC/huffencode.cpp
    EAC/refabout.cpp
    EAC/refdecode.cpp
    EAC/refencode.cpp
    EAC/btreecodex.h
    EAC/codex.h
    EAC/gimex.h
    EAC/huffcodex.h
    EAC/refcodex.h
    LZHCompress/NoxCompress.cpp
    LZHCompress/NoxCompress.h
    CompressionManager.cpp
    Compression.h
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
    libzlib
)
