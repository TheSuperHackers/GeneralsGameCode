set(ZLIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/zlib-1.1.4-src/ZLib)

FetchContent_Populate(zlib DOWNLOAD_EXTRACT_TIMESTAMP
    GIT_REPOSITORY https://github.com/TheSuperHackers/zlib-1.1.4
    GIT_TAG        ac753eee3990a2f592bd4807ff0b30ff572c2104
    SOURCE_DIR     ${ZLIB_DIR}
)
    
add_library(libzlib STATIC)

target_sources(libzlib PRIVATE
    "${ZLIB_DIR}/adler32.c"
    "${ZLIB_DIR}/compress.c"
    "${ZLIB_DIR}/crc32.c"
    "${ZLIB_DIR}/gzio.c"
    "${ZLIB_DIR}/uncompr.c"
    "${ZLIB_DIR}/deflate.c"
    "${ZLIB_DIR}/trees.c"
    "${ZLIB_DIR}/zutil.c"
    "${ZLIB_DIR}/inflate.c"
    "${ZLIB_DIR}/infblock.c"
    "${ZLIB_DIR}/inftrees.c"
    "${ZLIB_DIR}/infcodes.c"
    "${ZLIB_DIR}/infutil.c"
    "${ZLIB_DIR}/inffast.c"
)

target_include_directories(libzlib PUBLIC ${ZLIB_DIR})

target_compile_definitions(libzlib PUBLIC Z_PREFIX)
