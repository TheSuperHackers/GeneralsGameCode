set(UNICODEBIDI_SRC
    unicodebidi.cpp
    unicodebidi.h
    bididirection.inl
)

add_library(deps_unicodebidi STATIC)
set_target_properties(deps_unicodebidi PROPERTIES OUTPUT_NAME unicodebidi)

target_sources(deps_unicodebidi PRIVATE ${UNICODEBIDI_SRC})

target_include_directories(deps_unicodebidi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(deps_unicodebidi PRIVATE
    deps_precompiled # Gives force included headers. Link early.
)
