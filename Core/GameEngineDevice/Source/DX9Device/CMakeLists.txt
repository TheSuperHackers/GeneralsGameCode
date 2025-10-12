# CMake for DirectX 9 Device
# This file will contain the source files for the DirectX 9 device implementation.

set(DX9_DEVICE_SOURCES
    DX9Device.cpp
)

add_library(DX9Device STATIC ${DX9_DEVICE_SOURCES})
target_include_directories(DX9Device PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # Include its own directory
target_link_libraries(DX9Device PRIVATE dx9) # Link against the dx9 SDK
