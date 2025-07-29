# Create gitinfo.cpp
set(GIT_PRE_CONFIGURE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/gitinfo/gitinfo.cpp.in")
set(GIT_POST_CONFIGURE_FILE "${CMAKE_CURRENT_BINARY_DIR}/gitinfo.cpp")

include(gitinfo/git_watcher.cmake)

# Create resources library
add_library(resources STATIC)

target_sources(resources PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/gitinfo.cpp"
    gitinfo/gitinfo.h
)

target_include_directories(resources PUBLIC
    gitinfo
)

# Add resources (optional)
if (MSVC_IDE)
    target_sources(resources PRIVATE
        visualstudio/asciistring.natvis
        visualstudio/stllist.natvis
        visualstudio/stlvector.natvis
        visualstudio/unicodestring.natvis
    )
endif()
