set(WWSAVELOAD_SRC
    definition.cpp
    definition.h
    definitionclassids.h
    definitionfactory.cpp
    definitionfactory.h
    definitionfactorymgr.cpp
    definitionfactorymgr.h
    definitionmgr.cpp
    definitionmgr.h
    editable.h
    parameter.cpp
    parameter.h
    parameterlist.h
    parametertypes.h
    persist.h
    persistfactory.cpp
    persistfactory.h
    pointerremap.cpp
    pointerremap.h
    postloadable.h
    saveload.cpp
    saveload.h
    saveloadids.h
    saveloadstatus.cpp
    saveloadstatus.h
    saveloadstatus.h
    saveloadsubsystem.cpp
    saveloadsubsystem.h
    simpledefinitionfactory.h
    simpleparameter.h
    twiddler.cpp
    twiddler.h
    wwsaveload.cpp
    wwsaveload.h
)

add_library(core_wwsaveload STATIC)
set_target_properties(core_wwsaveload PROPERTIES OUTPUT_NAME wwsaveload)

target_sources(core_wwsaveload PRIVATE ${WWSAVELOAD_SRC})

target_link_libraries(core_wwsaveload PRIVATE
    core_wwcommon
)
