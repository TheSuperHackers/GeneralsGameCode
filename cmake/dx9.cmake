FetchContent_Declare(
    dx9sdk # Name for FetchContent to manage
    GIT_REPOSITORY https://github.com/OmarAglan/DX9SDK.git
    GIT_TAG        423475713f7f8a396474fd6907d4357b8da8e24a # Use the specific commit hash you just pushed
)

# Make the dx9sdk source available (will be in build/_deps/dx9sdk-src)
FetchContent_MakeAvailable(dx9sdk)