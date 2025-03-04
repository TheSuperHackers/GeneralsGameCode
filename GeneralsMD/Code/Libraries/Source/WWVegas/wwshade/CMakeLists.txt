set(WWSHADE_SRC
    shddef.cpp
    shddeffactory.cpp
    shddefmanager.cpp
    shdforcelinks.cpp
    shdhwshader.cpp
    shdinterface.cpp
    shdlib.cpp
    shdloader.cpp
    shdmesh.cpp
    shdrenderer.cpp
    shdsubmesh.cpp
    shdclassids.h
    shddef.h
    shddeffactory.h
    shddefmanager.h
    shdforcelinks.h
    shdhw_constants.h
    shdhwshader.h
    shdinterface.h
    shdloader.h
    shdmesh.h
    shdrenderer.h
    shdsubmesh.h
    shdsimple.cpp
    shdsimple.h
    shd6bumpspec.cpp
    shd6bumpspec.h
    shd6bumpspec_constants.h
    shd7bumpspec.cpp
    shd7bumpspec.h
    shd7bumpspec_constants.h
    shd8bumpspec.cpp
    shd8bumpspec.h
    shd8bumpspec_constants.h
    shdbumpspec.cpp
    shdbumpspec.h
    shd6bumpdiff.cpp
    shd6bumpdiff.h
    shd6bumpdiff_constants.h
    shd7bumpdiff.cpp
    shd7bumpdiff.h
    shd7bumpdiff_constants.h
    shd8bumpdiff.cpp
    shd8bumpdiff.h
    shd8bumpdiff_constants.h
    shdbumpdiff.cpp
    shdbumpdiff.h
    shdglossmask.cpp
    shdglossmask.h
    shdcubemap.cpp
    shdcubemap.h
    shdlegacyw3d.cpp
    shdlegacyw3d.h
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    file(GLOB_RECURSE SHADER_FILES "*.vsh")
    list(APPEND SHADER_LIST ${SHADER_FILES})
    file(GLOB_RECURSE SHADER_FILES "*.psh")
    list(APPEND SHADER_LIST ${SHADER_FILES})
    foreach(shdfile ${SHADER_LIST})
        get_filename_component(shdfilename ${shdfile} NAME)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${shdfilename}_code.h
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/shdpp.exe ${shdfilename}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        list(APPEND WWSHADE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/${shdfilename}_code.h)
    endforeach()
endif()

add_library(z_wwshade STATIC)
set_target_properties(z_wwshade PROPERTIES OUTPUT_NAME wwshade)

target_sources(z_wwshade PRIVATE ${WWSHADE_SRC})

target_link_libraries(z_wwshade PRIVATE
    z_wwcommon
)
