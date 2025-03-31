# Set source files
set(WWUTIL_SRC
    mathutil.cpp
    mathutil.h
    miscutil.cpp
    miscutil.h
)

# Targets to build.
add_library(gz_wwutil STATIC)
set_target_properties(gz_wwutil PROPERTIES OUTPUT_NAME wwutil)

target_link_libraries(gz_wwutil PRIVATE
    gz_wwcommon
)

target_sources(gz_wwutil PRIVATE ${WWUTIL_SRC})
