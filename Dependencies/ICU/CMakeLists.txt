# ICU selection and public link settings are configured in cmake/icu.cmake.
target_sources(deps_icu PRIVATE
    ICU/IcuLoader.cpp
    ICU/IcuLoader.h
    ICU/utf8.cpp
    ICU/utf8.h
)

target_include_directories(deps_icu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(deps_icu PRIVATE
    depsi_always # Gives force included headers. Link early.
    deps_utility
    stlport
)

# Windows tests also run the conversions against linked ICU, next to IcuLoader in the game tests.
if(WIN32 AND RTS_BUILD_OPTION_TESTS AND ICU_FOUND)
    add_library(deps_icu_linked STATIC
        ICU/utf8.cpp
        ICU/utf8.h
    )

    target_include_directories(deps_icu_linked PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(deps_icu_linked PRIVATE RTS_HAS_ICU)
    target_link_libraries(deps_icu_linked PUBLIC ICU::uc)
    target_link_libraries(deps_icu_linked PRIVATE
        depsi_always # Gives force included headers. Link early.
        stlport
    )
endif()
