# Set source files
set(WWUTIL_SRC
    mathutil.cpp
    mathutil.h
    miscutil.cpp
    miscutil.h
)

# Targets to build.
add_library(wwutil STATIC)

target_link_libraries(wwutil PRIVATE
    wwcommon
)

target_sources(wwutil PRIVATE ${WWUTIL_SRC})
