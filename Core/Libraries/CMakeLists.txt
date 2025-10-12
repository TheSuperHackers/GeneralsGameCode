# WW common libraries decended from earliest C&C games
add_subdirectory(Source/WWVegas)

# profiling library
add_subdirectory(Source/profile)

# debugging library
add_subdirectory(Source/debug)

add_subdirectory(Source/EABrowserDispatch)
add_subdirectory(Source/EABrowserEngine)
add_subdirectory(Source/Compression)

target_include_directories(corei_libraries_include INTERFACE
    Include
    Include/Lib
)
