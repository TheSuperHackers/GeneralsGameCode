cmake_minimum_required(VERSION 3.25)

project(g_generals LANGUAGES C CXX)

if (RTS_BUILD_GENERALS_DOCS)
    find_package(Doxygen REQUIRED)
    doxygen_add_docs(g_docs Code)
endif()

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Add main build targets
add_subdirectory(Code)

# If we are building on windows for windows, try and get the game install path from the registry.
if("${CMAKE_HOST_SYSTEM}" MATCHES "Windows" AND "${CMAKE_SYSTEM}" MATCHES "Windows")
    # Check the CD registry path
    if(NOT RTS_INSTALL_PREFIX_GENERALS)
        get_filename_component(RTS_INSTALL_PREFIX_GENERALS  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Generals;InstallPath]" ABSOLUTE CACHE)
    endif()

    # Check the "First Decade" registry path
    if(NOT RTS_INSTALL_PREFIX_GENERALS OR "${RTS_INSTALL_PREFIX_GENERALS}" STREQUAL "/registry")
        get_filename_component(RTS_INSTALL_PREFIX_GENERALS  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer The First Decade;gr_folder]" ABSOLUTE CACHE)
    endif()

    # Check the Steam registry path
    if(NOT RTS_INSTALL_PREFIX_GENERALS OR "${RTS_INSTALL_PREFIX_GENERALS}" STREQUAL "/registry")
        get_filename_component(RTS_INSTALL_PREFIX_GENERALS  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Generals;installPath]" ABSOLUTE CACHE)
    endif()
endif()

if(RTS_INSTALL_PREFIX_GENERALS AND NOT "${RTS_INSTALL_PREFIX_GENERALS}" STREQUAL "/registry")
    install(TARGETS g_generals RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
    install(FILES $<TARGET_PDB_FILE:g_generals> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)

    if(TARGET g_worldbuilder)
        install(TARGETS g_worldbuilder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:g_worldbuilder> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()

    if(TARGET g_particleeditor)
        install(TARGETS g_particleeditor RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:g_particleeditor> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()

    if(TARGET g_debugwindow)
        install(TARGETS g_debugwindow RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:g_debugwindow> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()

    if(TARGET g_guiedit)
        install(TARGETS g_guiedit RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:g_guiedit> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()

    if(TARGET g_imagepacker)
        install(TARGETS g_imagepacker RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:g_imagepacker> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()

    if(TARGET g_mapcachebuilder)
        install(TARGETS g_mapcachebuilder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}")
        install(FILES $<TARGET_PDB_FILE:g_mapcachebuilder> DESTINATION "${RTS_INSTALL_PREFIX_GENERALS}" OPTIONAL)
    endif()
endif()
