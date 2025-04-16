set(WWMATH_SRC
    cardinalspline.cpp
    colmathaabox.cpp
    colmathaabtri.cpp
    colmathobbobb.cpp
    colmathobbtri.cpp
)

# Targets to build.
add_library(z_wwmath STATIC)
#set_target_properties(z_wwmath PROPERTIES OUTPUT_NAME wwmath)

target_sources(z_wwmath PRIVATE ${WWMATH_SRC})

target_link_libraries(z_wwmath PRIVATE
    core_wwmath
    z_wwcommon
)
