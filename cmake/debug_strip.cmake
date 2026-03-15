# TheSuperHackers @build JohnsterID 05/01/2026 Add debug symbol stripping for MinGW Release builds
# Debug Symbol Stripping for MinGW-w64 Release Builds
#
# Separates debug symbols from executables into .debug files, matching MSVC PDB workflow.
# This reduces shipped binary size while preserving symbols for crash analysis.

# Find the required tools for symbol stripping
if(MINGW)
    # Use the cross-compiler toolchain's objcopy and strip
    # These should be in the same directory as the compiler
    get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    
    find_program(MINGW_OBJCOPY
        NAMES ${CMAKE_CXX_COMPILER_TARGET}-objcopy
              ${CMAKE_SYSTEM_PROCESSOR}-w64-mingw32-objcopy
              objcopy
        HINTS ${COMPILER_DIR}
        DOC "MinGW objcopy tool for extracting debug symbols"
    )
    
    find_program(MINGW_STRIP
        NAMES ${CMAKE_CXX_COMPILER_TARGET}-strip
              ${CMAKE_SYSTEM_PROCESSOR}-w64-mingw32-strip
              strip
        HINTS ${COMPILER_DIR}
        DOC "MinGW strip tool for removing debug symbols"
    )
    
    if(MINGW_OBJCOPY AND MINGW_STRIP)
        message(STATUS "Debug symbol stripping enabled:")
        message(STATUS "  objcopy: ${MINGW_OBJCOPY}")
        message(STATUS "  strip:   ${MINGW_STRIP}")
        set(DEBUG_STRIP_AVAILABLE TRUE)
    else()
        message(WARNING "Debug symbol stripping not available - tools not found")
        if(NOT MINGW_OBJCOPY)
            message(WARNING "  objcopy not found")
        endif()
        if(NOT MINGW_STRIP)
            message(WARNING "  strip not found")
        endif()
        set(DEBUG_STRIP_AVAILABLE FALSE)
    endif()
    
    # Function to strip debug symbols from a target and create a separate .debug file
    #
    # This implements a three-step process:
    # 1. Extract debug symbols to separate file
    # 2. Strip debug symbols from main executable
    # 3. Add debug link so debuggers can find the symbols
    #
    # Usage:
    #   add_debug_strip_target(target_name)
    #
    # Result (for Release builds only):
    #   program.exe        - Stripped executable (smaller)
    #   program.exe.debug  - Debug symbols (can be distributed separately)
    #
    function(add_debug_strip_target target_name)
        if(NOT DEBUG_STRIP_AVAILABLE)
            return()
        endif()
        
        # Only strip Release builds
        # Debug builds keep symbols embedded for development convenience
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_custom_command(TARGET ${target_name} POST_BUILD
                # Step 1: Extract all debug sections to separate file
                COMMAND ${MINGW_OBJCOPY} 
                    --only-keep-debug
                    $<TARGET_FILE:${target_name}>
                    $<TARGET_FILE:${target_name}>.debug
                
                # Step 2: Strip debug sections from executable
                COMMAND ${MINGW_STRIP}
                    --strip-debug
                    --strip-unneeded
                    $<TARGET_FILE:${target_name}>
                
                # Step 3: Add GNU debug link (debuggers use this to find symbols)
                COMMAND ${MINGW_OBJCOPY}
                    --add-gnu-debuglink=$<TARGET_FILE:${target_name}>.debug
                    $<TARGET_FILE:${target_name}>
                
                COMMENT "Stripping debug symbols from ${target_name} (Release)"
                VERBATIM
            )
            
            message(STATUS "Debug symbol stripping configured for target: ${target_name}")
        endif()
    endfunction()
endif()
