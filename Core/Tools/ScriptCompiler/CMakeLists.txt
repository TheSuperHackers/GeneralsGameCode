set(SCRIPTCOMPILER_SRC
    "ScriptCompiler.cpp"
)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(core_scriptcompiler)
set_target_properties(core_scriptcompiler PROPERTIES OUTPUT_NAME scriptcompiler)

target_sources(core_scriptcompiler PRIVATE ${SCRIPTCOMPILER_SRC})

target_link_libraries(core_scriptcompiler PRIVATE
    nlohmann_json::nlohmann_json
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(core_scriptcompiler PRIVATE /subsystem:console)
endif()

