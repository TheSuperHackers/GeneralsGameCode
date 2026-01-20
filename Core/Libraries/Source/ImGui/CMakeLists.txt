FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG 791ad9b82db44ada9fedb3e26b2d900974ac0959
    SYSTEM
)

FetchContent_MakeAvailable(imgui)

# Main IMGUI sources we are going to need
set(IMGUI_BASE_SRCS
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
)


# Main Win32 DX8 specific sources we are going to need we can specify more if we need extra platforms
set(IMGUI_WIN32_DX8_IMPL_SRCS
    "${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp"
    "${CMAKE_SOURCE_DIR}/Core/Libraries/Source/ImGui/dx8_backend/imgui_impl_dx8.cpp"
)

# All Win32 DX8 sources
set(IMGUI_WIN32_DX8_ALL_SRCS ${IMGUI_BASE_SRCS} ${IMGUI_WIN32_DX8_IMPL_SRCS})

# All Include directories
set(IMGUI_INCLUDE_DIRS
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends"
    # DX8 override. Remove the following once we are using a standard backend
    "${CMAKE_SOURCE_DIR}/Core/Libraries/Source/ImGui/dx8_backend"
)

# start target build section
# we currently have a hard dependency on dx8 and win32 api
if (WIN32)
    # for now we only need it in debug configurations
    if ( NOT ( (CMAKE_BUILD_TYPE STREQUAL "Debug") OR RTS_BUILD_OPTION_DEBUG ) )
        MESSAGE(FATAL_ERROR "ImGui integration is currently only possible in Debug build modes")
    endif ()

    MESSAGE(STATUS "Enabling imgui console integration")

    add_library(lib_imgui STATIC ${IMGUI_WIN32_DX8_ALL_SRCS})
    target_include_directories(lib_imgui PUBLIC ${IMGUI_INCLUDE_DIRS})
    target_link_libraries(lib_imgui PRIVATE d3d8lib)

    # use our own imconfig.h
    target_compile_definitions(lib_imgui
        PRIVATE IMGUI_DISABLE_DEFAULT_IMCONFIG
        PRIVATE IMGUI_USER_CONFIG="${CMAKE_CURRENT_LIST_DIR}/imconfig.h"
        INTERFACE RTS_IMGUI_ENABLED
    )
else ()
    # currently only WIN32 DX is supported
    MESSAGE(FATAL_ERROR "Non-Windows platforms currently not supported for ImGui integration ${MSG_ERR_SUFFIX}")
endif ()
# end target build section
