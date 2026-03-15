# TheSuperHackers @build JohnsterID 05/01/2026 Add MinGW-w64 cross-compilation support
# MinGW-w64 specific compiler and linker configurations

if(MINGW)
    message(STATUS "Configuring MinGW-w64 build settings")
    
    # Detect if this is 32-bit or 64-bit MinGW
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(IS_MINGW32 TRUE)
        message(STATUS "MinGW-w64 32-bit (i686) detected")
    else()
        message(FATAL_ERROR "MinGW-w64 64-bit (x86_64) detected, but this project only supports 32-bit builds. Use the i686-w64-mingw32 toolchain.")
    endif()
    
    # Windows subsystem
    add_link_options(-mwindows)
    
    # Static linking of GCC runtime libraries
    # This embeds libgcc and libstdc++ into the executable to avoid DLL dependencies
    add_link_options(-static-libgcc -static-libstdc++)
    
    # Compatibility flags for legacy code
    add_compile_options(
        -fno-strict-aliasing        # Avoid type-punning issues with DX8/COM
    )
    
    # MSVC compatibility macros for MinGW
    # Note: MinGW already defines _cdecl and _stdcall correctly, so we only add __forceinline
    # The escaped syntax below expands to: -D__forceinline="inline __attribute__((always_inline))"
    # Escaping rules: \( \) = literal parentheses, \ (backslash-space) = space in definition
    add_compile_definitions(
        __forceinline=inline\ __attribute__\(\(always_inline\)\)
        __int64=long\ long
        _int64=long\ long
    )
    
    # Enable math constants in MinGW's <math.h>
    # MinGW provides M_PI, M_E, etc. in <math.h>, but only when -std=c++XX is NOT used (strict ANSI mode),
    # or when _USE_MATH_DEFINES is defined. Since we compile with -std=c++20, we need this define.
    # The header mingw.h (included via always.h) provides the missing constants (M_1_SQRTPI, M_SQRT_2 alias).
    add_compile_definitions(
        _USE_MATH_DEFINES
    )
    
    # Ensure proper calling conventions are defined
    # MinGW-w64 should define these, but verify they exist
    include(CheckCXXSymbolExists)
    check_cxx_symbol_exists(STDMETHODCALLTYPE "windows.h" HAVE_STDMETHODCALLTYPE)
    if(NOT HAVE_STDMETHODCALLTYPE)
        add_compile_definitions(
            STDMETHODCALLTYPE=__stdcall
            STDMETHODIMP=HRESULT\ __stdcall
        )
    endif()
    
    # Required Windows libraries for DX8 + COM
    link_libraries(
        uuid        # COM GUIDs
        ole32       # COM runtime
        oleaut32    # COM automation
        gdi32       # GDI
        user32      # User interface
        comctl32    # Common controls
        winmm       # Multimedia (timeGetTime, etc.)
        vfw32       # Video for Windows (AVIFile functions)
        d3d8        # Direct3D 8
        dinput8     # DirectInput 8
        dsound      # DirectSound
        imm32       # Input Method Manager (IME)
    )
    
    # Note: MinGW-w64 does not provide comsuppw (COM support utilities library).
    # COM support utilities (_com_util::ConvertStringToBSTR, ConvertBSTRToString)
    # are provided by Dependencies/Utility/Utility/comsupp_compat.h as header-only
    # implementations. No library linking required.
    
    # MinGW-w64 compatibility: Create d3dx8 as an alias to d3dx8d
    # MinGW-w64 only provides libd3dx8d.a (debug library), not libd3dx8.a
    # The min-dx8-sdk (dx8.cmake) handles this correctly via d3d8lib interface target,
    # but for compatibility with direct library references in main executables,
    # we create an alias so that linking to d3dx8 automatically uses d3dx8d
    if(NOT TARGET d3dx8)
        add_library(d3dx8 INTERFACE IMPORTED GLOBAL)
        set_target_properties(d3dx8 PROPERTIES
            INTERFACE_LINK_LIBRARIES "d3dx8d"
        )
        message(STATUS "Created d3dx8 -> d3dx8d alias for MinGW-w64")
    endif()
    
    message(STATUS "MinGW-w64 configuration complete")
endif()
