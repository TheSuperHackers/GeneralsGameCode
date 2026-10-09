# Base headers shared by all libraries
set(LIBRARIES_INCLUDE_SRC
    Include/Lib/BaseFunctions.h
    Include/Lib/BaseType.h
    Include/Lib/Coord2D.h
    Include/Lib/Coord3D.h
    Include/Lib/ICoord2D.h
    Include/Lib/ICoord3D.h
    Include/Lib/IRegion2D.h
    Include/Lib/IRegion3D.h
    Include/Lib/PathUtil.h
    Include/Lib/RealRange.h
    Include/Lib/Region2D.h
    Include/Lib/Region3D.h
    Include/Lib/RGBColor.h
    Include/Lib/trig.h
    Include/rts/debug.h
    Include/rts/profile.h
    Include/Precompiled/Precompiled/BaseAlgorithm.h
    Include/Precompiled/Precompiled/BaseMacros.h
    Include/Precompiled/Precompiled/BasePragmas.h
    Include/Precompiled/Precompiled/BasePrecomp.h
    Include/Precompiled/Precompiled/BaseTypes.h
)

add_library(core_libraries INTERFACE)

target_include_directories(core_libraries INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Precompiled
)

target_sources(core_libraries PRIVATE ${LIBRARIES_INCLUDE_SRC})

target_link_libraries(core_libraries INTERFACE
    deps_precompiled
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

# WW common libraries decended from earliest C&C games
add_subdirectory(Source/WWVegas)

# profiling library
add_subdirectory(Source/profile)

# debugging library
add_subdirectory(Source/debug)

add_subdirectory(Source/EABrowserDispatch)
add_subdirectory(Source/EABrowserEngine)
add_subdirectory(Source/Compression)
