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

add_library(gz_compression STATIC)
set_target_properties(gz_compression PROPERTIES OUTPUT_NAME compression)

target_sources(gz_compression PRIVATE ${COMPRESSION_SRC})

target_include_directories(gz_compression INTERFACE
    .
)

target_link_libraries(gz_compression PRIVATE
    gz_config
    ci_libraries_include
)

target_link_libraries(gz_compression PUBLIC
    liblzhl
    libzlib
)
