FetchContent_Declare(
    stlport
    URL https://downloads.sourceforge.net/project/stlport/STLport%20archive/STLport%204/STLport-4.6.2.tar.gz
)
FetchContent_MakeAvailable(stlport)

add_library(stlport INTERFACE)
target_include_directories(stlport INTERFACE "${stlport_SOURCE_DIR}/stlport")
target_compile_definitions(stlport INTERFACE USING_STLPORT=1 _STLP_NO_NEW_IOSTREAMS=1)
