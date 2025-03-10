# Set source files
set(WWSAVELOAD_SRC
    definition.cpp
    definitionfactory.cpp
    definitionfactorymgr.cpp
    definitionmgr.cpp
    parameter.cpp
    persistfactory.cpp
    pointerremap.cpp
    saveload.cpp
    saveloadstatus.cpp
    saveloadsubsystem.cpp
    twiddler.cpp
    wwsaveload.cpp
    definition.h
    definitionclassids.h
    definitionfactory.h
    definitionfactorymgr.h
    definitionmgr.h
    editable.h
    parameter.h
    parameterlist.h
    parametertypes.h
    persist.h
    persistfactory.h
    pointerremap.h
    postloadable.h
    saveload.h
    saveloadids.h
    saveloadstatus.h
    saveloadstatus.h
    saveloadsubsystem.h
    simpledefinitionfactory.h
    simpleparameter.h
    twiddler.h
    wwsaveload.h
)

# Targets to build.
add_library(wwsaveloadg STATIC)

target_link_libraries(wwsaveloadg PRIVATE
    wwcommong
)

target_sources(wwsaveloadg PRIVATE ${WWSAVELOAD_SRC})
