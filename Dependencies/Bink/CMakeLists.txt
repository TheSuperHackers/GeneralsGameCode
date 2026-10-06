set(BINK_SRC
    bink.h
    BinkLoader.cpp
    BinkLoader.h
)

add_library(deps_bink STATIC)

target_sources(deps_bink PRIVATE ${BINK_SRC})

target_include_directories(deps_bink PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(deps_bink PRIVATE
    deps_precompiled # Gives force included headers. Link early.
    deps_utility
)
