option(RTS_BUILD_OPTION_CRASHPAD "Use optional local-only Crashpad reporting in the games" OFF)
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
endif()

function(rts_enable_crashpad game engine)
    if(NOT RTS_BUILD_OPTION_CRASHPAD)
        return()
    endif()
    # Tools may link the same engine, but never initialize this optional backend.
    # Neither the engine nor tools link the Crashpad client or DLL import library.
    target_compile_definitions(${engine} PRIVATE RTS_USE_CRASHPAD=1)
    target_include_directories(${engine} PRIVATE "${CMAKE_SOURCE_DIR}/Dependencies/Crashpad")
    # Refresh runtime files before linking, even when only the DLL changed.
    add_custom_target(${game}_crashpad_runtime
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${game}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:rts_crashpad>" "$<TARGET_PDB_FILE:rts_crashpad>"
            "${RTS_CRASHPAD_HANDLER}"
            ${RTS_CRASHPAD_RUNTIME_FILES} "$<TARGET_FILE_DIR:${game}>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RTS_CRASHPAD_NOTICES}" "$<TARGET_FILE_DIR:${game}>/crashpad-notices"
        DEPENDS rts_crashpad
        VERBATIM)
    add_dependencies(${game} ${game}_crashpad_runtime)
endfunction()

function(rts_install_crashpad game destination)
    if(RTS_BUILD_OPTION_CRASHPAD)
        install(FILES "$<TARGET_FILE:rts_crashpad>" "$<TARGET_PDB_FILE:rts_crashpad>"
            "${RTS_CRASHPAD_HANDLER}"
            DESTINATION "${destination}")
        install(DIRECTORY "${RTS_CRASHPAD_NOTICES}/" DESTINATION "${destination}/crashpad-notices")
        install(FILES ${RTS_CRASHPAD_RUNTIME_FILES} DESTINATION "${destination}"
            CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
    endif()
endfunction()
