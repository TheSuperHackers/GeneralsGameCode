# Do we want to build extra SDK stuff or just the game binary?
option(GENZH_BUILD_ZEROHOUR "Build Zero Hour code." ON)
option(GENZH_BUILD_GENERALS "Build Generals code." ON)
option(GENZH_BUILD_INTERNAL "Build code with the \"Internal\" configuration." OFF)
option(GENZH_BUILD_PROFILE "Build code with the \"Profile\" configuration." OFF)
option(GENZH_BUILD_DEBUG "Build code with the \"Debug\" configuration." OFF)

if(NOT GENZH_BUILD_ZEROHOUR AND NOT GENZH_BUILD_GENERALS)
    set(GENZH_BUILD_ZEROHOUR TRUE)
    message("You must select one project to build, building Zero Hour by default.")
endif()

add_feature_info(ZeroHourStuff GENZH_BUILD_ZEROHOUR "Build Zero Hour code")
add_feature_info(GeneralsStuff GENZH_BUILD_GENERALS "Build Generals code")
add_feature_info(InternalBuild GENZH_BUILD_INTERNAL "Building as an \"Internal\" build")
add_feature_info(ProfileBuild GENZH_BUILD_PROFILE "Building as a \"Profile\" build")
add_feature_info(DebugBuild GENZH_BUILD_DEBUG "Building as a \"Debug\" build")

add_library(gz_config INTERFACE)

if(NOT IS_VS6_BUILD)
    # Because we set CMAKE_CXX_STANDARD_REQUIRED and CMAKE_CXX_EXTENSIONS in the compilers.cmake this should be enforced.
    target_compile_features(gz_config INTERFACE cxx_std_20)
endif()

target_compile_options(gz_config INTERFACE ${GENZH_FLAGS})

# This disables a lot of warnings steering developers to use windows only functions/function names.
if(MSVC)
    target_compile_definitions(gz_config INTERFACE _CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS $<$<CONFIG:DEBUG>:_DEBUG_CRT>)
endif()

if(GENZH_BUILD_DEBUG)
    target_compile_definitions(gz_config INTERFACE _DEBUG WWDEBUG DEBUG)
else()
    target_compile_definitions(gz_config INTERFACE _RELEASE)

    if(GENZH_BUILD_INTERNAL)
        target_compile_definitions(gz_config INTERFACE _INTERNAL)
    endif()
    if(GENZH_BUILD_PROFILE)
        target_compile_definitions(gz_config INTERFACE _PROFILE)
    endif()
endif()
