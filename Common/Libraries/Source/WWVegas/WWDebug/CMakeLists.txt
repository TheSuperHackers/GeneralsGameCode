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
add_library(gz_wwdebug STATIC)
set_target_properties(gz_wwdebug PROPERTIES OUTPUT_NAME wwdebug)

target_sources(gz_wwdebug PRIVATE ${WWDEBUG_SRC})

target_link_libraries(gz_wwdebug PRIVATE
    gz_wwcommon
)
