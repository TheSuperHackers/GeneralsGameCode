cmake_minimum_required(VERSION 3.25)

project(z_generals LANGUAGES C CXX)

if (RTS_BUILD_ZEROHOUR_DOCS)
    find_package(Doxygen REQUIRED)
    doxygen_add_docs(z_docs Code)
endif()

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Add main build targets
add_subdirectory(Code)

# If we are building on windows for windows, try and get the game install path from the registry.
if("${CMAKE_HOST_SYSTEM}" MATCHES "Windows" AND "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(NOT RTS_INSTALL_PREFIX_ZEROHOUR)
        get_filename_component(RTS_INSTALL_PREFIX_ZEROHOUR  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour;InstallPath]" ABSOLUTE CACHE)
    endif()

    # Check the "First Decade" registry path
    if(NOT RTS_INSTALL_PREFIX_ZEROHOUR OR "${RTS_INSTALL_PREFIX_ZEROHOUR}" STREQUAL "/registry")
        get_filename_component(RTS_INSTALL_PREFIX_ZEROHOUR  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer The First Decade;zh_folder]" ABSOLUTE CACHE)
    endif()

    # Check the Steam registry path
    if(NOT RTS_INSTALL_PREFIX_ZEROHOUR OR "${RTS_INSTALL_PREFIX_ZEROHOUR}" STREQUAL "/registry")
        get_filename_component(RTS_INSTALL_PREFIX_ZEROHOUR  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\ZeroHour;installPath]" ABSOLUTE CACHE)
    endif()
endif()

if(RTS_INSTALL_PREFIX_ZEROHOUR AND NOT "${RTS_INSTALL_PREFIX_ZEROHOUR}" STREQUAL "/registry")
    install(TARGETS z_generals RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
    install(FILES $<TARGET_PDB_FILE:z_generals> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)

    if(TARGET z_worldbuilder)
        install(TARGETS z_worldbuilder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_worldbuilder> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_particleeditor)
        install(TARGETS z_particleeditor RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_particleeditor> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_debugwindow)
        install(TARGETS z_debugwindow RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_debugwindow> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_guiedit)
        install(TARGETS z_guiedit RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_guiedit> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_imagepacker)
        install(TARGETS z_imagepacker RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_imagepacker> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_mapcachebuilder)
        install(TARGETS z_mapcachebuilder RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_mapcachebuilder> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()
    
    if(TARGET z_w3dview)
        install(TARGETS z_w3dview RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_w3dview> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()

    if(TARGET z_wdump)
        install(TARGETS z_wdump RUNTIME DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}")
        install(FILES $<TARGET_PDB_FILE:z_wdump> DESTINATION "${RTS_INSTALL_PREFIX_ZEROHOUR}" OPTIONAL)
    endif()
endif()
