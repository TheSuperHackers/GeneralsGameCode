set(DATGEN_SRC
    "DatGen.cpp"
    "../BFISH.CPP"
    "../BFISH.H"
    "../Toolkit/Debug/DebugPrint.cpp"
    "../Toolkit/Debug/DebugPrint.h"
)

add_executable(datgen WIN32)

target_include_directories(datgen PRIVATE
    ../
    ../Toolkit
)

target_link_libraries(datgen PRIVATE
    safedisc
)

target_sources(datgen PRIVATE ${DATGEN_SRC})
