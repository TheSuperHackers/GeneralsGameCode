set(USP10_SRC
    usp10loader.cpp
    usp10loader.h
    usp10_adapter.h
    usp10_subset.h
)

add_library(deps_usp10 STATIC)

target_sources(deps_usp10 PRIVATE ${USP10_SRC})

target_include_directories(deps_usp10 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(deps_usp10 PRIVATE
    deps_precompiled # Gives force included headers. Link early.
    deps_utility
)
