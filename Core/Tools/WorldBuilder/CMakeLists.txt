set(WORLDBUILDER_SRC
    "include/BorderTool.h"
    "include/WHeightMapEdit.h"
    "src/BorderTool.cpp"
    "src/WHeightMapEdit.cpp"
)

add_library(corei_worldbuilder INTERFACE)

target_sources(corei_worldbuilder INTERFACE ${WORLDBUILDER_SRC})

target_include_directories(corei_worldbuilder INTERFACE
    include
)
