set(LIBRARIES_INCLUDE_SRC
    Lib/BaseFunctions.h
    Lib/BaseType.h
    Lib/Coord2D.h
    Lib/Coord3D.h
    Lib/ICoord2D.h
    Lib/ICoord3D.h
    Lib/IRegion2D.h
    Lib/IRegion3D.h
    Lib/PathUtil.h
    Lib/RealRange.h
    Lib/Region2D.h
    Lib/Region3D.h
    Lib/RGBColor.h
    Lib/trig.h
    rts/debug.h
    rts/profile.h
    Precompiled/Precompiled/BaseMacros.h
    Precompiled/Precompiled/BasePragmas.h
    Precompiled/Precompiled/BasePrecomp.h
    Precompiled/Precompiled/BaseTypes.h
)

## core_libraries

add_library(core_libraries INTERFACE)

target_include_directories(core_libraries INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Precompiled
)

target_sources(core_libraries PRIVATE ${LIBRARIES_INCLUDE_SRC})

target_link_libraries(core_libraries INTERFACE
    core_precompiled
)

# Makes the game base macros and types available in every C++ translation unit
# of the targets that link this.
# Force included rather than precompiled, because a precompiled header entry
# inherited from an interface library always sorts after the consuming target's
# own entries, and these things must be defined before any other header is read.
# Targets that do have a precompiled header must still list Precompiled/BasePrecomp.h
# as their first entry, because the Visual Studio generator prepends the precompiled
# header to the forced include list, so this one would otherwise be read too late.
if (MSVC)
    target_compile_options(core_libraries INTERFACE
        "$<$<COMPILE_LANGUAGE:CXX>:/FIPrecompiled/BasePrecomp.h>"
    )
else()
    target_compile_options(core_libraries INTERFACE
        "$<$<COMPILE_LANGUAGE:CXX>:SHELL:-include Precompiled/BasePrecomp.h>"
    )
endif()
