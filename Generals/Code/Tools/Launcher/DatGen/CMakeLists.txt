set(DATGEN_SRC
    "DatGen.cpp"
    "../BFISH.CPP"
    "../BFISH.H"
    "../Toolkit/Debug/DebugPrint.cpp"
    "../Toolkit/Debug/DebugPrint.h"
)

add_executable(g_datgen WIN32)
set_target_properties(g_datgen PROPERTIES OUTPUT_NAME datgen)

target_include_directories(g_datgen PRIVATE
    ../
    ../Toolkit
)

target_link_libraries(g_datgen PRIVATE
    core_config
    safedisc
)

target_sources(g_datgen PRIVATE ${DATGEN_SRC})
