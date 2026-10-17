# ICU4C for the engine.
#
# Preference order:
# 1. find_package(ICU) from vcpkg or the system (C API: uc, data)
# 2. Runtime LoadLibrary of OS icu.dll on Windows
#
# Windows without a linked ICU uses IcuLoader for the UTF conversion exports, without ICU headers
# or import libraries. If the DLL or either export is missing, conversion falls back to Win32 CP_UTF8.

add_library(deps_icu STATIC)

set(RTS_ICU_LINKED FALSE)
set(RTS_ICU_DYNAMIC FALSE)

if(NOT IS_VS6_BUILD)
    find_package(ICU QUIET COMPONENTS uc OPTIONAL_COMPONENTS data)
endif()

if(ICU_FOUND AND NOT IS_VS6_BUILD)
    set(RTS_ICU_LINKED TRUE)
    target_link_libraries(deps_icu PUBLIC ICU::uc)
    if(TARGET ICU::data)
        target_link_libraries(deps_icu PUBLIC ICU::data)
    endif()
    target_compile_definitions(deps_icu PUBLIC RTS_HAS_ICU)
    message(STATUS "ICU4C: linked via find_package")
elseif(WIN32)
    set(RTS_ICU_DYNAMIC TRUE)
    target_compile_definitions(deps_icu PUBLIC RTS_ICU_DYNAMIC)
    message(STATUS "ICU4C: IcuLoader resolves UTF conversion exports from icu.dll; Win32 fallback if unavailable")
else()
    message(FATAL_ERROR "ICU4C is required on non-Windows platforms. Install libicu or enable vcpkg.")
endif()

add_feature_info(IcuLinked RTS_ICU_LINKED "Link ICU4C into the engine")
add_feature_info(IcuDynamic RTS_ICU_DYNAMIC "Load OS icu.dll at runtime")
