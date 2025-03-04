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
add_library(z_wwsaveload STATIC)
set_target_properties(z_wwsaveload PROPERTIES OUTPUT_NAME wwsaveload)

target_sources(z_wwsaveload PRIVATE ${WWSAVELOAD_SRC})

target_link_libraries(z_wwsaveload PRIVATE
    z_wwcommon
)
