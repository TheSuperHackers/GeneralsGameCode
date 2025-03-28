cmake_minimum_required(VERSION 3.25)

project(z_generals LANGUAGES C CXX)

# Do we want to build extra SDK stuff or just the game binary?
option(GENZH_BUILD_ZEROHOUR_TOOLS "Build tools for Zero Hour" ON)
add_feature_info(ZeroHourTools GENZH_BUILD_ZEROHOUR_TOOLS "Build Zero Hour Mod Tools")
option(GENZH_BUILD_ZEROHOUR_EXTRAS "Build extra tools/tests for Zero Hour" OFF)
add_feature_info(ZeroHourExtras GENZH_BUILD_ZEROHOUR_EXTRAS "Build Zero Hour Extra Tools/Tests")

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Add main build targets
add_subdirectory(Code)

# If we are building on windows for windows, try and get the game install path from the registry.
if("${CMAKE_HOST_SYSTEM}" MATCHES "Windows" AND "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(NOT GENZH_ZEROHOUR_INSTALL_PREFIX)
        get_filename_component(GENZH_ZEROHOUR_INSTALL_PREFIX  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour;InstallPath]" ABSOLUTE CACHE)
    endif()

    # Check the "First Decade" registry path
    if(NOT GENZH_ZEROHOUR_INSTALL_PREFIX OR "${GENZH_ZEROHOUR_INSTALL_PREFIX}" STREQUAL "/registry")
        get_filename_component(GENZH_ZEROHOUR_INSTALL_PREFIX  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer The First Decade;zh_folder]" ABSOLUTE CACHE)
    endif()

    # Check the Steam registry path
    if(NOT GENZH_ZEROHOUR_INSTALL_PREFIX OR "${GENZH_ZEROHOUR_INSTALL_PREFIX}" STREQUAL "/registry")
        get_filename_component(GENZH_ZEROHOUR_INSTALL_PREFIX  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\ZeroHour;installPath]" ABSOLUTE CACHE)
    endif()
endif()

if(GENZH_ZEROHOUR_INSTALL_PREFIX AND NOT "${GENZH_ZEROHOUR_INSTALL_PREFIX}" STREQUAL "/registry")
    install(TARGETS z_generals RUNTIME DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}")
    install(FILES $<TARGET_PDB_FILE:z_generals> DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}" OPTIONAL)

    if(TARGET z_worldbuilder)
        install(TARGETS z_worldbuilder RUNTIME DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:z_worldbuilder> DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}" OPTIONAL)
    endif()

    if(TARGET z_particleeditor)
        install(TARGETS z_particleeditor RUNTIME DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:z_particleeditor> DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}" OPTIONAL)
    endif()

    if(TARGET z_debugwindow)
        install(TARGETS z_debugwindow RUNTIME DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:z_debugwindow> DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}" OPTIONAL)
    endif()

    if(TARGET z_guiedit)
        install(TARGETS z_guiedit RUNTIME DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:z_guiedit> DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}" OPTIONAL)
    endif()

    if(TARGET z_imagepacker)
        install(TARGETS z_imagepacker RUNTIME DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:z_imagepacker> DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}" OPTIONAL)
    endif()

    if(TARGET z_mapcachebuilder)
        install(TARGETS z_mapcachebuilder RUNTIME DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}")
        install(FILES $<TARGET_PDB_FILE:z_mapcachebuilder> DESTINATION "${GENZH_ZEROHOUR_INSTALL_PREFIX}" OPTIONAL)
    endif()
endif()
