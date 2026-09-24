set(PRECOMPILED_SRC
    Precompiled/CppMacros.h
    Precompiled/CppTypes.h
    Precompiled/precompiled.h
    Precompiled/stdint_adapter.h
    Precompiled/stdint_subset.h
)

## core_precompiled

add_library(core_precompiled INTERFACE)

target_include_directories(core_precompiled INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(core_precompiled PRIVATE ${PRECOMPILED_SRC})

# Makes the C++98 compatibility macros and types available in every C++ translation
# unit of the targets that link this, before any other header is read.
# Force included rather than precompiled, because a precompiled header entry
# inherited from an interface library always sorts after the consuming target's
# own entries, and these things must be defined before any other header is read.
if (MSVC)
    target_compile_options(core_precompiled INTERFACE
        "$<$<COMPILE_LANGUAGE:CXX>:/FIPrecompiled/precompiled.h>"
    )
else()
    target_compile_options(core_precompiled INTERFACE
        "$<$<COMPILE_LANGUAGE:CXX>:SHELL:-include Precompiled/precompiled.h>"
    )
endif()
