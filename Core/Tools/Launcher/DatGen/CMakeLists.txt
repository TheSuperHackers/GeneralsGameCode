set(DATGEN_SRC
    "DatGen.cpp"
    "../BFISH.CPP"
    "../BFISH.H"
    "../Toolkit/Debug/DebugPrint.cpp"
    "../Toolkit/Debug/DebugPrint.h"
)

add_library(corei_datgen INTERFACE)

target_include_directories(corei_datgen INTERFACE
    ../
    ../Toolkit
)

target_link_libraries(corei_datgen INTERFACE
    core_config
    safedisc
)

target_sources(corei_datgen INTERFACE ${DATGEN_SRC})
