# Set source files
set(WWUTIL_SRC
    mathutil.cpp
    mathutil.h
    miscutil.cpp
    miscutil.h
)

# Targets to build.
add_library(g_wwutil STATIC)
set_target_properties(g_wwutil PROPERTIES OUTPUT_NAME wwutil)

target_sources(g_wwutil PRIVATE ${WWUTIL_SRC})

target_link_libraries(g_wwutil PRIVATE
    g_wwcommon
)
