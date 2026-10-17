# ICU selection and public link settings are configured in cmake/icu.cmake.
target_sources(deps_icu PRIVATE
    ICU/IcuLoader.cpp
    ICU/IcuLoader.h
    ICU/IcuSupport.h
    ICU/utf8.cpp
    ICU/utf8.h
)

target_include_directories(deps_icu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(deps_icu PRIVATE
    depsi_always # Gives force included headers. Link early.
    deps_utility
    stlport
)
