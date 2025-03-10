set(TEXTURECOMPRESS_SRC
    "resource.h"
    "textureCompress.cpp"
)

add_executable(texturecompress WIN32)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(texturecompress PRIVATE textureCompress.rc)
endif()

target_sources(texturecompress PRIVATE ${TEXTURECOMPRESS_SRC})
