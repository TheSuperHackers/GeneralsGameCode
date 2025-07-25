# Setting C++ default compiler flags for VC6 

string(JOIN " " VC6_CXX_FLAGS
    "/DWIN32"          # Define WIN32 macro
    "/D_WINDOWS"       # Define _WINDOWS macro
    "/Zm800"           # Cap precompiled header memory allocation to prevent running out of heap space
    "/Gd"              # __cdecl calling convention for all functions (C/C++ standard behaviour)
    "/GR"              # Enable RTTI (Run-Time Type Information, needed for things like dynamic_cast)
    "/EHs"             # Synchronous C++ exception handling (ISO-standard C++ exception handling)
    "/EHc"             # Assume extern "C" functions never throw (safe for release)
)
string(JOIN " " VC6_CXX_FLAGS_RELEASE
    "/O2"              # Optimize for speed
    "/Ob2"             # Inline any suitable function
    "/Oy"              # Omit frame pointer (smaller/faster code)
    "/DNDEBUG"
    "/O2"              # Optimize for speed
    "/nologo"          # Suppress copyright message
)
string(JOIN " " VC6_CXX_FLAGS_DEBUG
    "/Og /Oi /Gs"      # Optimize for speed (for compatibility)
    "/Ob2"             # Inline any suitable function (for compatibility)
    "/Oy-"             # Don't omit frame pointer
    "/DNDEBUG"         # (for compatibility)
)
string(JOIN " " VC6_EXE_LINKER_FLAGS
    "/machine:IX86"     # Target x86 architecture
    "/LARGEADDRESSAWARE"# Allow addresses larger than 2 gigabytes"
)
string(JOIN " " VC6_EXE_LINKER_FLAGS_RELEASE
    "/RELEASE"         # Set the checksum in the header and mark as release
    "/OPT:REF,ICF"     # Remove unreferenced code/data, COMDAT folding
    "/NOLOGO"          # Suppress linker startup banner
)
string(JOIN " " VC6_EXE_LINKER_FLAGS_DEBUG
    "/DEBUG"           # Generate debug info
    "/OPT:NOREF,ICF"   # Don't remove unreferenced code/data, COMDAT folding
)
set(CMAKE_CXX_FLAGS           "${VC6_CXX_FLAGS}"          CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE   "${VC6_CXX_FLAGS_RELEASE}"  CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG     "${VC6_CXX_FLAGS_DEBUG}"    CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS             "${VC6_CXX_FLAGS}"          CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE     "${VC6_CXX_FLAGS_RELEASE}"  CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_DEBUG       "${VC6_CXX_FLAGS_DEBUG}"    CACHE STRING "" FORCE)


set(CMAKE_EXE_LINKER_FLAGS    "${VC6_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "${VC6_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "${VC6_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)

set(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${VC6_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${VC6_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${VC6_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)

set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${VC6_EXE_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${VC6_EXE_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_MODULE_FLAGS_RELEASE "${VC6_EXE_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)

set(CMAKE_STATIC_LINKER_FLAGS "/machine:IX86" CACHE STRING "" FORCE)
set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "" CACHE STRING "" FORCE)
set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "" CACHE STRING "" FORCE)


