set(MILES_SRC
    mss/mss.h
    MilesLoader.cpp
    MilesLoader.h
)

add_library(deps_miles STATIC)

target_sources(deps_miles PRIVATE ${MILES_SRC})

target_include_directories(deps_miles PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mss
)

target_link_libraries(deps_miles PRIVATE
    deps_precompiled # Gives force included headers. Link early.
    deps_utility
)
