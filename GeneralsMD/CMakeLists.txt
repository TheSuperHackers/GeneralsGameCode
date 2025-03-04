cmake_minimum_required(VERSION 3.25)

project(z_generals LANGUAGES C CXX)

# Do we want to build extra SDK stuff or just the game binary?
option(GENZH_BUILD_ZEROHOUR_TOOLS "Build tools for Zero Hour" OFF)
add_feature_info(ZeroHourTools GENZH_BUILD_ZEROHOUR_TOOLS "Build Zero Hour Mod Tools")

# Set where the build results will end up
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Add main build targets
add_subdirectory(Code)

feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
