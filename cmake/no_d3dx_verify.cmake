# D3DX Dependency Verification for NO_D3DX builds
#
# Verifies that when NO_D3DX is enabled, executables have no d3dx8*.dll imports.
# This ensures the compatibility layer is working correctly.

if(MINGW AND MINGW_NO_D3DX)
    # Find objdump tool for checking DLL imports
    find_program(MINGW_OBJDUMP
        NAMES ${CMAKE_CXX_COMPILER_TARGET}-objdump
              ${CMAKE_SYSTEM_PROCESSOR}-w64-mingw32-objdump
              objdump
        DOC "MinGW objdump tool for verifying DLL imports"
    )
    
    if(MINGW_OBJDUMP)
        message(STATUS "D3DX elimination verification enabled:")
        message(STATUS "  objdump: ${MINGW_OBJDUMP}")
        set(NO_D3DX_VERIFY_AVAILABLE TRUE)
    else()
        message(WARNING "D3DX verification not available - objdump not found")
        set(NO_D3DX_VERIFY_AVAILABLE FALSE)
    endif()
    
    # Function to add D3DX verification to a target
    #
    # Checks that the built executable does not import d3dx8*.dll
    # Fails the build if D3DX dependency is detected
    #
    # Usage:
    #   add_no_d3dx_verification(target_name)
    #
    function(add_no_d3dx_verification target_name)
        if(NOT NO_D3DX_VERIFY_AVAILABLE)
            return()
        endif()
        
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND bash -c 
                "${MINGW_OBJDUMP} -p $<TARGET_FILE:${target_name}> | grep -i 'd3dx8' && echo 'ERROR: D3DX8 DLL dependency detected! NO_D3DX failed.' && exit 1 || echo 'SUCCESS: No D3DX8 DLL dependency (NO_D3DX working)'"
            COMMENT "Verifying no D3DX8 dependency in ${target_name}"
            VERBATIM
        )
        
        message(STATUS "D3DX elimination verification configured for target: ${target_name}")
    endfunction()
endif()
