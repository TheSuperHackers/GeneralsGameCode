# Base headers and debug utilities shared by all libraries
add_subdirectory(Source/Lib)

# WW common libraries decended from earliest C&C games
add_subdirectory(Source/WWVegas)

# profiling library
add_subdirectory(Source/profile)

# debugging library
add_subdirectory(Source/debug)

# crash dump library
add_subdirectory(Source/MiniDumper)

add_subdirectory(Source/EABrowserDispatch)
add_subdirectory(Source/EABrowserEngine)
add_subdirectory(Source/Compression)
