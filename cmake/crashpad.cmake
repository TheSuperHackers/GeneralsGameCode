option(RTS_BUILD_OPTION_CRASHPAD "Use optional local-only Crashpad reporting in the games" OFF)
option(RTS_BUILD_CRASHPAD_TESTS "Build the dedicated Crashpad fault-injection executable" OFF)
add_feature_info(Crashpad RTS_BUILD_OPTION_CRASHPAD "Local-only out-of-process game crash reports")

if(RTS_BUILD_OPTION_CRASHPAD)
    if(NOT RTS_CRASHDUMP_ENABLE)
        message(FATAL_ERROR "RTS_BUILD_OPTION_CRASHPAD requires RTS_CRASHDUMP_ENABLE=ON")
    endif()
    if(NOT WIN32 OR NOT MSVC OR IS_VS6_BUILD OR NOT CMAKE_SIZEOF_VOID_P EQUAL 4
       OR MSVC_VERSION LESS 1920)
        message(FATAL_ERROR "Crashpad requires modern MSVC or ClangCL targeting Win32 (x86)")
    endif()
    if(RTS_BUILD_OPTION_ASAN OR RTS_BUILD_CORE_FUZZ)
        message(FATAL_ERROR "Keep Crashpad disabled in sanitizer and fuzzing builds")
    endif()
    add_subdirectory(Dependencies/Crashpad)
elseif(RTS_BUILD_CRASHPAD_TESTS)
    message(FATAL_ERROR "RTS_BUILD_CRASHPAD_TESTS requires RTS_BUILD_OPTION_CRASHPAD=ON")
endif()

function(rts_enable_crashpad game engine)
    if(NOT RTS_BUILD_OPTION_CRASHPAD)
        return()
    endif()
    # Tools may link the same engine, but never initialize this optional backend.
    # Neither the engine nor tools link the Crashpad client or DLL import library.
    target_compile_definitions(${engine} PRIVATE RTS_USE_CRASHPAD=1)
    target_include_directories(${engine} PRIVATE "${CMAKE_SOURCE_DIR}/Dependencies/Crashpad")
    # Component expressions deliberately avoid a dependency on the executable:
    # this runtime target runs before linking, even when only the DLL changed.
    set(metadata_command ${CMAKE_COMMAND}
        "-DOUTPUT=$<TARGET_FILE_DIR:${game}>/${game}-crashpad-build.json"
        "-DGAME=$<TARGET_FILE_DIR:${game}>/$<TARGET_FILE_NAME:${game}>"
        "-DPDB=$<TARGET_PDB_FILE_DIR:${game}>/$<TARGET_PDB_FILE_NAME:${game}>"
        "-DBRIDGE=$<TARGET_FILE:rts_crashpad>" "-DBRIDGE_PDB=$<TARGET_PDB_FILE:rts_crashpad>"
        "-DHANDLER=${RTS_CRASHPAD_HANDLER}" "-DCONFIG=$<CONFIG>"
        "-DCOMPILER=${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
        "-DSOURCE=${CMAKE_SOURCE_DIR}" "-DGIT=${GIT_EXECUTABLE}"
        -P "${CMAKE_SOURCE_DIR}/cmake/crashpad-metadata.cmake")
    add_custom_target(${game}_crashpad_runtime
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${game}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:rts_crashpad>" "$<TARGET_PDB_FILE:rts_crashpad>"
            "${RTS_CRASHPAD_HANDLER}" "$<TARGET_FILE:rts_crashpad_reports>"
            ${RTS_CRASHPAD_RUNTIME_FILES} "$<TARGET_FILE_DIR:${game}>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RTS_CRASHPAD_NOTICES}" "$<TARGET_FILE_DIR:${game}>/crashpad-notices"
        COMMAND ${metadata_command}
        DEPENDS rts_crashpad rts_crashpad_reports
        VERBATIM)
    add_dependencies(${game} ${game}_crashpad_runtime)
    add_custom_command(TARGET ${game} POST_BUILD COMMAND ${metadata_command} VERBATIM)
endfunction()

function(rts_install_crashpad game destination)
    if(RTS_BUILD_OPTION_CRASHPAD)
        install(FILES "$<TARGET_FILE:rts_crashpad>" "$<TARGET_PDB_FILE:rts_crashpad>"
            "${RTS_CRASHPAD_HANDLER}" "$<TARGET_FILE:rts_crashpad_reports>"
            "$<TARGET_FILE_DIR:${game}>/${game}-crashpad-build.json"
            DESTINATION "${destination}")
        install(DIRECTORY "${RTS_CRASHPAD_NOTICES}/" DESTINATION "${destination}/crashpad-notices")
        install(FILES ${RTS_CRASHPAD_RUNTIME_FILES} DESTINATION "${destination}"
            CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
    endif()
endfunction()
