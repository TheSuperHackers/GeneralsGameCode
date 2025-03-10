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
add_library(wwdebug STATIC)

target_link_libraries(wwdebug PRIVATE
    wwcommon
)

target_sources(wwdebug PRIVATE ${WWDEBUG_SRC})
