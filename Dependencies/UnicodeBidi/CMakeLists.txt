set(UNICODEBIDI_SRC
    unicodebidi.cpp
    unicodebidi.h
    supplementarybidi.inl
)

add_library(unicodebidi STATIC)
set_target_properties(unicodebidi PROPERTIES OUTPUT_NAME unicodebidi)

target_sources(unicodebidi PRIVATE ${UNICODEBIDI_SRC})

target_include_directories(unicodebidi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # include "unicodebidi.h"
)

target_link_libraries(unicodebidi PRIVATE
    core_precompiled # Gives force included headers. Link early.
)
