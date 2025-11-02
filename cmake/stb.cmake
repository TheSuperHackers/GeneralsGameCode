# TheSuperHackers @bobtista 02/11/2025
# STB single-file public domain libraries for image encoding
# https://github.com/nothings/stb

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master  # Could pin to specific commit for stability
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(stb)

# Create interface library for stb headers
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

