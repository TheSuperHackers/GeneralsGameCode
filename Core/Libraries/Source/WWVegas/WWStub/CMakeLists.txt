set(WWSTUB_SRC
    wwallocstub.cpp
    wwdebugstub.cpp
)

add_library(core_wwstub STATIC)
set_target_properties(core_wwstub PROPERTIES OUTPUT_NAME wwstub)

target_sources(core_wwstub PRIVATE ${WWSTUB_SRC})

target_link_libraries(core_wwstub PRIVATE
    core_wwcommon
    corei_always
)
