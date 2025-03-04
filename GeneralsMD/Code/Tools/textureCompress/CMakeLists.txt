set(TEXTURECOMPRESS_SRC
    "resource.h"
    "textureCompress.cpp"
)

add_executable(z_texturecompress WIN32)
set_target_properties(z_texturecompress PROPERTIES OUTPUT_NAME texturecompress)

target_sources(z_texturecompress PRIVATE ${TEXTURECOMPRESS_SRC})

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(z_texturecompress PRIVATE textureCompress.rc)
endif()
