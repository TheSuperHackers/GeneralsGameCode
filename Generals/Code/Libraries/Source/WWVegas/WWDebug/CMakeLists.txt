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
add_library(wwdebugg STATIC)

target_link_libraries(wwdebugg PRIVATE
    wwcommong
)

target_sources(wwdebugg PRIVATE ${WWDEBUG_SRC})
