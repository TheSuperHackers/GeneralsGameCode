# Set source files
set(WWUTIL_SRC
    mathutil.cpp
    mathutil.h
    miscutil.cpp
    miscutil.h
)

# Targets to build.
add_library(wwutilg STATIC)

target_link_libraries(wwutilg PRIVATE
    wwcommong
)

target_sources(wwutilg PRIVATE ${WWUTIL_SRC})
