# Set source files
set(WWDEBUG_SRC
    wwdebug.cpp
    wwmemlog.cpp
    wwprofile.cpp
    wwdebug.h
    wwhack.h
    wwmemlog.h
    wwprofile.h
)

# Targets to build.
add_library(g_wwdebug STATIC)
set_target_properties(g_wwdebug PROPERTIES OUTPUT_NAME wwdebug)

target_sources(g_wwdebug PRIVATE ${WWDEBUG_SRC})

target_link_libraries(g_wwdebug PRIVATE
    g_wwcommon
)
