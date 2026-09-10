# Slang is consumed from the vcpkg manifest. The package provides both the
# runtime library and the slangc shader compiler without requiring this project
# to build Slang's internal source generators.
find_package(slang CONFIG REQUIRED)

if(NOT DEFINED SLANG_EXECUTABLE OR NOT EXISTS "${SLANG_EXECUTABLE}")
    find_program(SLANG_EXECUTABLE
        NAMES slangc slangc.exe
        HINTS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/shader-slang"
    )
endif()

if(NOT SLANG_EXECUTABLE OR NOT EXISTS "${SLANG_EXECUTABLE}")
    message(FATAL_ERROR
        "The vcpkg Slang package was found, but slangc was not. "
        "Install shader-slang through the project vcpkg manifest.")
endif()

# Keep the existing shader-generation target name as a local compatibility
# target. It now points at vcpkg's slangc executable and is never built here.
if(NOT TARGET slang-bootstrap)
    add_executable(slang-bootstrap IMPORTED GLOBAL)
    set_target_properties(slang-bootstrap PROPERTIES
        IMPORTED_LOCATION "${SLANG_EXECUTABLE}"
    )
endif()

message(STATUS "Slang compiler: ${SLANG_EXECUTABLE}")
