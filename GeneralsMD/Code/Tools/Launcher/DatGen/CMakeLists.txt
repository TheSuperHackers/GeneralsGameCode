set(DATGEN_SRC
    "DatGen.cpp"
    "../BFISH.CPP"
    "../BFISH.H"
    "../Toolkit/Debug/DebugPrint.cpp"
    "../Toolkit/Debug/DebugPrint.h"
)

add_executable(z_datgen WIN32)
set_target_properties(z_datgen PROPERTIES OUTPUT_NAME datgen)

target_include_directories(z_datgen PRIVATE
    ../
    ../Toolkit
)

target_link_libraries(z_datgen PRIVATE
    safedisc
)

target_sources(z_datgen PRIVATE ${DATGEN_SRC})
