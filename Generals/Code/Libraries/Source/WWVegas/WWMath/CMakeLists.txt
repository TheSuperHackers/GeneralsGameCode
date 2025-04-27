set(WWMATH_SRC
    cardinalspline.cpp
    colmathaabox.cpp
    colmathaabtri.cpp
    colmathobbobb.cpp
    colmathobbtri.cpp
)

# Targets to build.
add_library(g_wwmath STATIC)
#set_target_properties(g_wwmath PROPERTIES OUTPUT_NAME wwmath)

target_sources(g_wwmath PRIVATE ${WWMATH_SRC})

target_link_libraries(g_wwmath PRIVATE
    core_wwmath
    g_wwcommon
)
