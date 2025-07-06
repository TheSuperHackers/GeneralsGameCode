set(TEXTURECOMPRESS_SRC
    "resource.h"
    "textureCompress.cpp"
)

add_executable(core_texturecompress WIN32)
set_target_properties(core_texturecompress PROPERTIES OUTPUT_NAME texturecompress)

target_sources(core_texturecompress PRIVATE ${TEXTURECOMPRESS_SRC})

target_link_libraries(core_texturecompress PRIVATE
    core_config
    core_utility
    core_wwlib
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(core_texturecompress PRIVATE textureCompress.rc)
endif()
