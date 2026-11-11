set(PLUGLIB_SRC
    "errclass.h"
    "EULER.cpp"
    "EULER.h"
    "nodefilt.cpp"
    "nodefilt.h"
    "nodelist.cpp"
    "nodelist.h"
    "PROGRESS.h"
)

add_library(core_pluglib STATIC)
set_target_properties(core_pluglib PROPERTIES OUTPUT_NAME pluglib)

target_sources(core_pluglib PRIVATE ${PLUGLIB_SRC})

target_link_libraries(core_pluglib PUBLIC
    corei_always
    core_wwvegas
    deps_maxsdk # Must stay below corei_always so that max_undef.h is force included after Precompiled/BasePrecomp.h
)
