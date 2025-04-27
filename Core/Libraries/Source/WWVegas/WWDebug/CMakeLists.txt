# Set source files
set(WWDEBUG_SRC
    wwdebug.cpp
    wwdebug.h
    wwhack.h
    wwmemlog.cpp
    wwmemlog.h
    wwprofile.cpp
    wwprofile.h
)

# Targets to build.
add_library(core_wwdebug STATIC)
set_target_properties(core_wwdebug PROPERTIES OUTPUT_NAME wwdebug)

target_sources(core_wwdebug PRIVATE ${WWDEBUG_SRC})

target_link_libraries(core_wwdebug PRIVATE
    core_wwcommon
    corei_always
)
