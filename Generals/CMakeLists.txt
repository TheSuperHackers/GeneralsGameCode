cmake_minimum_required(VERSION 3.25)

project(g_generals LANGUAGES C CXX)

# Do we want to build extra SDK stuff or just the game binary?
option(GENZH_BUILD_GENERALS_TOOLS "Build tools for Generals" ON)
add_feature_info(GeneralsTools GENZH_BUILD_GENERALS_TOOLS "Build Generals Mod Tools")
option(GENZH_BUILD_GENERALS_EXTRAS "Build extra tools/tests for Generals" OFF)
add_feature_info(GeneralsExtras GENZH_BUILD_GENERALS_EXTRAS "Build Generals Extra Tools/Tests")

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Add main build targets
add_subdirectory(Code)

# If we are building on windows for windows, try and get the game install path from the registry.
if("${CMAKE_HOST_SYSTEM}" MATCHES "Windows" AND "${CMAKE_SYSTEM}" MATCHES "Windows")
    # Check the CD registry path
    if(NOT GENZH_GENERALS_INSTALL_PREFIX)
        get_filename_component(GENZH_GENERALS_INSTALL_PREFIX  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Generals;InstallPath]" ABSOLUTE CACHE)
    endif()

    # Check the "First Decade" registry path
    if(NOT GENZH_GENERALS_INSTALL_PREFIX OR "${GENZH_GENERALS_INSTALL_PREFIX}" STREQUAL "/registry")
        get_filename_component(GENZH_GENERALS_INSTALL_PREFIX  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer The First Decade;gr_folder]" ABSOLUTE CACHE)
    endif()

    # Check the Steam registry path
    if(NOT GENZH_GENERALS_INSTALL_PREFIX OR "${GENZH_GENERALS_INSTALL_PREFIX}" STREQUAL "/registry")
        get_filename_component(GENZH_GENERALS_INSTALL_PREFIX  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Generals;installPath]" ABSOLUTE CACHE)
    endif()
endif()

if(GENZH_GENERALS_INSTALL_PREFIX AND NOT "${GENZH_GENERALS_INSTALL_PREFIX}" STREQUAL "/registry")
    install(TARGETS g_generals RUNTIME DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}")
    install(FILES $<TARGET_PDB_FILE:g_generals> DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}" OPTIONAL)
    
    if(TARGET g_worldbuilder)
        install(TARGETS g_worldbuilder RUNTIME DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:g_worldbuilder> DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}" OPTIONAL)
    endif()

    if(TARGET g_particleeditor)
        install(TARGETS g_particleeditor RUNTIME DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:g_particleeditor> DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}" OPTIONAL)
    endif()

    if(TARGET g_debugwindow)
        install(TARGETS g_debugwindow RUNTIME DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:g_debugwindow> DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}" OPTIONAL)
    endif()

    if(TARGET g_guiedit)
        install(TARGETS g_guiedit RUNTIME DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:g_guiedit> DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}" OPTIONAL)
    endif()

    if(TARGET g_imagepacker)
        install(TARGETS g_imagepacker RUNTIME DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:g_imagepacker> DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}" OPTIONAL)
    endif()

    if(TARGET g_mapcachebuilder)
        install(TARGETS g_mapcachebuilder RUNTIME DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:g_mapcachebuilder> DESTINATION "${GENZH_GENERALS_INSTALL_PREFIX}" OPTIONAL)
    endif()
endif()
