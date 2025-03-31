set(TEXTURECOMPRESS_SRC
    "resource.h"
    "textureCompress.cpp"
)

add_executable(g_texturecompress WIN32)
set_target_properties(g_texturecompress PROPERTIES OUTPUT_NAME texturecompress)

target_sources(g_texturecompress PRIVATE ${TEXTURECOMPRESS_SRC})

target_link_libraries(g_texturecompress PRIVATE
    gz_config
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(g_texturecompress PRIVATE textureCompress.rc)
endif()
