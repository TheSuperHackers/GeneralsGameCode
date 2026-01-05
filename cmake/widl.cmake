# TheSuperHackers @build JohnsterID 05/01/2026 Add widl integration for COM interface generation
# WIDL (Wine IDL Compiler) detection and configuration
# Used as MIDL replacement for MinGW-w64 builds

if(MINGW)
    # Find widl executable
    find_program(WIDL_EXECUTABLE
        NAMES widl widl-stable
        DOC "Wine IDL compiler for MinGW-w64"
    )
    
    if(WIDL_EXECUTABLE)
        # Get widl version
        execute_process(
            COMMAND ${WIDL_EXECUTABLE} -V
            OUTPUT_VARIABLE WIDL_VERSION_OUTPUT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        if(WIDL_VERSION_OUTPUT MATCHES "Wine IDL Compiler version ([0-9.]+)")
            set(WIDL_VERSION ${CMAKE_MATCH_1})
            message(STATUS "Found widl: ${WIDL_EXECUTABLE} (version ${WIDL_VERSION})")
        else()
            message(STATUS "Found widl: ${WIDL_EXECUTABLE}")
        endif()
        
        set(IDL_COMPILER ${WIDL_EXECUTABLE})
        set(IDL_COMPILER_FOUND TRUE)
        
        # Detect Wine include paths dynamically
        find_path(WINE_WINDOWS_INCLUDE_DIR
            NAMES oaidl.idl
            PATHS
                /usr/include/wine/wine/windows
                /usr/include/wine/windows
                /usr/include/wine-development/windows
                /opt/wine-stable/include/wine/windows
                /usr/local/include/wine/windows
            NO_DEFAULT_PATH
            NO_CMAKE_FIND_ROOT_PATH
            DOC "Wine Windows headers directory"
        )
        
        if(WINE_WINDOWS_INCLUDE_DIR)
            get_filename_component(WINE_BASE_INCLUDE_DIR "${WINE_WINDOWS_INCLUDE_DIR}/.." ABSOLUTE)
            message(STATUS "Wine include directory: ${WINE_WINDOWS_INCLUDE_DIR}")
            set(WIDL_INCLUDE_PATHS
                -I${WINE_WINDOWS_INCLUDE_DIR}
                -I${WINE_BASE_INCLUDE_DIR}
            )
        else()
            message(WARNING "Wine include directory not found. widl may fail to compile IDL files.")
            set(WIDL_INCLUDE_PATHS "")
        endif()
        
    else()
        message(WARNING "widl not found. Install with: apt-get install wine-stable-dev (Debian/Ubuntu) or wine-devel (Fedora/RHEL)")
        set(IDL_COMPILER_FOUND FALSE)
    endif()
    
    # WIDL command function (compatible with MIDL)
    function(add_idl_file target_name idl_file)
        get_filename_component(idl_basename ${idl_file} NAME_WE)
        get_filename_component(idl_dir ${idl_file} DIRECTORY)
        
        set(header_file "${CMAKE_CURRENT_BINARY_DIR}/${idl_basename}.h")
        set(iid_file "${CMAKE_CURRENT_BINARY_DIR}/${idl_basename}_i.c")
        
        # Build widl flags with dynamically detected Wine paths
        set(WIDL_FLAGS
            --win32
            -I${idl_dir}
            ${WIDL_INCLUDE_PATHS}
            -D__WIDL__
            -DDECLSPEC_ALIGN\(x\)=
        )
        
        # Generate header file
        add_custom_command(
            OUTPUT ${header_file}
            COMMAND ${IDL_COMPILER}
                ${WIDL_FLAGS}
                -h -o ${header_file}
                ${idl_file}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${idl_file}
            COMMENT "Compiling IDL to header with widl: ${idl_file}"
            VERBATIM
        )
        
        # Generate IID file
        add_custom_command(
            OUTPUT ${iid_file}
            COMMAND ${IDL_COMPILER}
                ${WIDL_FLAGS}
                -u -o ${iid_file}
                ${idl_file}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${idl_file}
            COMMENT "Compiling IDL to IID with widl: ${idl_file}"
            VERBATIM
        )
        
        # Return output files to parent scope
        set(${target_name}_HEADER ${header_file} PARENT_SCOPE)
        set(${target_name}_IID ${iid_file} PARENT_SCOPE)
    endfunction()
    
elseif(MSVC)
    # MSVC uses midl.exe
    find_program(MIDL_EXECUTABLE
        NAMES midl.exe midl
        DOC "Microsoft IDL compiler"
    )
    
    if(MIDL_EXECUTABLE)
        message(STATUS "Found midl: ${MIDL_EXECUTABLE}")
        set(IDL_COMPILER ${MIDL_EXECUTABLE})
        set(IDL_COMPILER_FOUND TRUE)
    else()
        # midl.exe is usually in PATH with Visual Studio
        set(IDL_COMPILER "midl.exe")
        set(IDL_COMPILER_FOUND TRUE)
        message(STATUS "Using midl.exe from PATH")
    endif()
    
    # MIDL command function
    function(add_idl_file target_name idl_file)
        get_filename_component(idl_basename ${idl_file} NAME_WE)
        
        set(header_file "${CMAKE_CURRENT_BINARY_DIR}/${idl_basename}.h")
        set(iid_file "${CMAKE_CURRENT_BINARY_DIR}/${idl_basename}_i.c")
        
        # Convert forward slashes to backslashes for MIDL
        file(TO_NATIVE_PATH ${idl_file} idl_file_native)
        
        add_custom_command(
            OUTPUT ${header_file} ${iid_file}
            COMMAND ${IDL_COMPILER} "${idl_file_native}" /header ${idl_basename}.h /iid ${idl_basename}_i.c
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${idl_file}
            COMMENT "Compiling IDL file ${idl_file} with midl"
            VERBATIM
        )
        
        # Return output files to parent scope
        set(${target_name}_HEADER ${header_file} PARENT_SCOPE)
        set(${target_name}_IID ${iid_file} PARENT_SCOPE)
    endfunction()
else()
    message(WARNING "No IDL compiler configured for this platform")
    set(IDL_COMPILER_FOUND FALSE)
endif()
