
# Print some information
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

if (DEFINED MSVC_VERSION)
    message(STATUS "MSVC_VERSION: ${MSVC_VERSION}")
endif()

# Set variable for VS6 to handle special cases.
if (DEFINED MSVC_VERSION AND MSVC_VERSION LESS 1300)
    set(IS_VS6_BUILD TRUE)
else()
    set(IS_VS6_BUILD FALSE)
endif()

if(MSVC)
    # /INCREMENTAL:NO prevents PDB size bloat in Debug configuration(s).
    add_link_options("/INCREMENTAL:NO")
    # Generate debug information for all builds
    add_link_options("/debug")

    set(CMAKE_CONFIGURATION_TYPES "Release;Debug")
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    set(CMAKE_CXX_FLAGS  "/DWIN32 /D_WINDOWS /EHac")
    set(CMAKE_C_FLAGS  "/DWIN32 /D_WINDOWS")

    if(IS_VS6_BUILD)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Release>:>$<$<CONFIG:Debug>:ProgramDatabase>")
        
        # So only one link ever runs (since vc6 can't handle multithreaded linking)
        set_property(GLOBAL PROPERTY JOB_POOLS link=1)
        set(CMAKE_JOB_POOL_LINK link)

        # Tell the linker to place the debug information in a single .PDB file
        add_link_options("/pdbtype:con")
        # Tell the linker to use both coff and cv debug info
        add_link_options("/debugtype:both")
        # Prevent heap from running out of memory 
        add_compile_options("/Zm800")
        # Make precompiled headers debug information accessible
        add_compile_options("/Yd")
        # Include line numbers in debug information
        add_compile_options("$<$<CONFIG:Release>:/Zd>")
    else()
        # Multithreaded build with Visual Studio Generator (ninja is multithreaded by default)
        if(CMAKE_GENERATOR MATCHES "Visual Studio")
            add_compile_options("/MP")
        endif()
        # Set __cplusplus macro to correct value
        add_compile_options(/Zc:__cplusplus)
    endif()
else()
        # We go a bit wild here and assume any other compiler we are going to use supports -g for debug info.
        add_compile_options("-g")
endif()

if(NOT IS_VS6_BUILD)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)  # Ensures only ISO features are used
endif()

if(RTS_BUILD_OPTION_ASAN)
    if(MSVC)
        set(ENV{ASAN_OPTIONS} "shadow_scale=2")
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()
