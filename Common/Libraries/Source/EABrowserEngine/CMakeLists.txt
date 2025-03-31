add_library(gz_browserengine INTERFACE)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    add_custom_command(
        OUTPUT BrowserEngine_i.c BrowserEngine.h
        COMMAND midl.exe "${CMAKE_CURRENT_LIST_DIR}\\BrowserEngine.idl" /header BrowserEngine.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/BrowserEngine.idl"
        VERBATIM
    )
    add_library(gz_browserenginewin STATIC BrowserEngine_i.c)
    set_target_properties(gz_browserenginewin PROPERTIES OUTPUT_NAME browserenginewin)
    target_link_libraries(gz_browserengine INTERFACE gz_browserenginewin)
endif()

target_include_directories(gz_browserengine INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/..)
