if(NOT MSVC OR MINGW)
    return()
endif()

function(_genzh_get_msvc_toolset_root out_var compiler_path)
    get_filename_component(_toolset_root "${compiler_path}" DIRECTORY)
    get_filename_component(_toolset_root "${_toolset_root}" DIRECTORY)
    get_filename_component(_toolset_root "${_toolset_root}" DIRECTORY)
    get_filename_component(_toolset_root "${_toolset_root}" DIRECTORY)
    set(${out_var} "${_toolset_root}" PARENT_SCOPE)
endfunction()

function(_genzh_add_msvc_atl_paths)
    _genzh_get_msvc_toolset_root(_current_toolset_root "${CMAKE_CXX_COMPILER}")

    set(_current_atl_include "${_current_toolset_root}/atlmfc/include")
    if(EXISTS "${_current_atl_include}/atlbase.h")
        message(STATUS "MSVC ATL found in active toolset: ${_current_atl_include}")
        include_directories(BEFORE SYSTEM "${_current_atl_include}")
        string(APPEND CMAKE_RC_FLAGS " /I\"${_current_atl_include}\"")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            link_directories("${_current_toolset_root}/atlmfc/lib/x64")
        else()
            link_directories("${_current_toolset_root}/atlmfc/lib/x86")
        endif()
        return()
    endif()

    set(_atl_arch "x86")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_atl_arch "x64")
    endif()

    get_filename_component(_current_toolset_version "${_current_toolset_root}" NAME)
    set(_atl_candidate_include "")
    set(_atl_candidate_lib "")

    file(GLOB _msvc_toolset_candidates LIST_DIRECTORIES TRUE
        "C:/Program Files/Microsoft Visual Studio/*/*/VC/Tools/MSVC/*"
        "C:/Program Files (x86)/Microsoft Visual Studio/*/*/VC/Tools/MSVC/*")

    foreach(_candidate_root IN LISTS _msvc_toolset_candidates)
        if(NOT EXISTS "${_candidate_root}/atlmfc/include/atlbase.h")
            continue()
        endif()
        if(NOT EXISTS "${_candidate_root}/atlmfc/lib/${_atl_arch}/atls.lib")
            continue()
        endif()

        get_filename_component(_candidate_version "${_candidate_root}" NAME)
        if(_candidate_version STREQUAL _current_toolset_version)
            set(_atl_candidate_include "${_candidate_root}/atlmfc/include")
            set(_atl_candidate_lib "${_candidate_root}/atlmfc/lib/${_atl_arch}")
            break()
        endif()

        if(_atl_candidate_include STREQUAL "")
            set(_atl_candidate_include "${_candidate_root}/atlmfc/include")
            set(_atl_candidate_lib "${_candidate_root}/atlmfc/lib/${_atl_arch}")
        endif()
    endforeach()

    if(_atl_candidate_include STREQUAL "" OR _atl_candidate_lib STREQUAL "")
        message(FATAL_ERROR
            "MSVC ATL headers/libs were not found.\n"
            "Compiler toolset: ${_current_toolset_root}\n"
            "Expected header: ${_current_toolset_root}/atlmfc/include/atlbase.h\n"
            "Install the Visual Studio ATL/MFC component, or provide a VS toolset installation that contains atlmfc.")
    endif()

    message(STATUS "MSVC ATL fallback include: ${_atl_candidate_include}")
    message(STATUS "MSVC ATL fallback lib: ${_atl_candidate_lib}")
    include_directories(BEFORE SYSTEM "${_atl_candidate_include}")
    string(APPEND CMAKE_RC_FLAGS " /I\"${_atl_candidate_include}\"")
    link_directories("${_atl_candidate_lib}")
endfunction()

_genzh_add_msvc_atl_paths()
