# stb is supplied by the vcpkg manifest through its FindStb module.
find_package(Stb REQUIRED)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${Stb_INCLUDE_DIR})
