if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/maxsdk AND IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/maxsdk)
    message("MaxSDK has been provided, 3d Studio Max plugins can be built.")

    add_library(deps_maxsdk INTERFACE)
    target_include_directories(deps_maxsdk INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        maxsdk/Include
        maxsdk/Include/Maxscrpt
    )
    target_compile_options(deps_maxsdk INTERFACE /FImax_undef.h)
    target_link_directories(deps_maxsdk INTERFACE maxsdk/Lib)
    target_link_libraries(deps_maxsdk INTERFACE
        bmm
        core
        geom
        Maxscrpt
        maxutil
        mesh
        Paramblk2
    )
else()
    message("MaxSDK has not been provided, 3d Studio Max plugins cannot be built.")
endif()
