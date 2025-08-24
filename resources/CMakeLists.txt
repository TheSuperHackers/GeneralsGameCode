# Create gitinfo.cpp
set(GIT_PRE_CONFIGURE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/gitinfo/gitinfo.cpp.in")
set(GIT_POST_CONFIGURE_FILE "${CMAKE_CURRENT_BINARY_DIR}/gitinfo.cpp")

# Ensure RELEASE_NAME is always defined for configure_file substitutions
if(NOT DEFINED RELEASE_NAME)
    set(RELEASE_NAME "")
endif()

# Override git variables when RELEASE_NAME is defined
if(DEFINED RELEASE_NAME AND NOT "${RELEASE_NAME}" STREQUAL "")
    # Set git variables to use RELEASE_NAME
    set(GIT_HEAD_SHA1 "${RELEASE_NAME}")
    set(GIT_HEAD_SHORT_SHA1 "${RELEASE_NAME}")
    set(GIT_TAG "${RELEASE_NAME}")
    set(GIT_COMMIT_DATE_ISO8601 "")
    set(GIT_AUTHOR_NAME "")
    set(GIT_COMMIT_TSTAMP 0)
    set(GIT_IS_DIRTY false)
    set(GIT_RETRIEVED_STATE true)
    set(GIT_REV_LIST_COUNT 1)

    # Configure the file directly with our variables
    configure_file("${GIT_PRE_CONFIGURE_FILE}" "${GIT_POST_CONFIGURE_FILE}" @ONLY)
else()
    # Use the normal git watcher for development builds
    include(gitinfo/git_watcher.cmake)
endif()

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
