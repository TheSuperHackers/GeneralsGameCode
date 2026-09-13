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
        visualstudio/wwlib/hash.natvis
        visualstudio/wwlib/hashtemplate.natvis
        visualstudio/wwlib/listnode.natvis
        visualstudio/wwlib/mpsc_intrusive_queue.natvis
        visualstudio/wwlib/multilist.natvis
        visualstudio/wwlib/sharebuf.natvis
        visualstudio/wwlib/simplevec.natvis
        visualstudio/wwlib/slist.natvis
        visualstudio/wwlib/uarray.natvis
        visualstudio/wwlib/vector.natvis
        visualstudio/wwlib/widestring.natvis
        visualstudio/wwlib/wwstring.natvis
    )
endif()
