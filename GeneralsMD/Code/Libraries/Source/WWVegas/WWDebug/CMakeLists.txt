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
add_library(z_wwdebug STATIC)
set_target_properties(z_wwdebug PROPERTIES OUTPUT_NAME wwdebug)

target_sources(z_wwdebug PRIVATE ${WWDEBUG_SRC})

target_link_libraries(z_wwdebug PRIVATE
    z_wwcommon
    zi_libraries_include
)
