# Set source files
set(WWUTIL_SRC
    mathutil.cpp
    mathutil.h
    miscutil.cpp
    miscutil.h
)

# Targets to build.
add_library(z_wwutil STATIC)
set_target_properties(z_wwutil PROPERTIES OUTPUT_NAME wwutil)

target_link_libraries(z_wwutil PRIVATE
    z_wwcommon
)

target_sources(z_wwutil PRIVATE ${WWUTIL_SRC})
