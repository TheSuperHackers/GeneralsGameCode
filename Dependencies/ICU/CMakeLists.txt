# ICU selection and public link settings are configured in cmake/icu.cmake.
target_sources(core_icu PRIVATE
    ICU/IcuLoader.cpp
    ICU/IcuLoader.h
    ICU/IcuSupport.h
    ICU/utf8.cpp
    ICU/utf8.h
)

target_include_directories(core_icu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core_icu PRIVATE core_config core_utility stlport)
