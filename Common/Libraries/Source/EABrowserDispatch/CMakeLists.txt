add_library(gz_browserdispatch INTERFACE)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    add_custom_command(
        OUTPUT BrowserDispatch_i.c BrowserDispatch.h
        COMMAND midl.exe "${CMAKE_CURRENT_LIST_DIR}\\BrowserDispatch.idl" /header BrowserDispatch.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/BrowserDispatch.idl"
        VERBATIM
    )
    add_library(gz_browserdispatchwin STATIC BrowserDispatch_i.c)
    set_target_properties(gz_browserdispatchwin PROPERTIES OUTPUT_NAME browserdispatchwin)
    target_link_libraries(gz_browserdispatch INTERFACE gz_browserdispatchwin)
endif()

target_include_directories(gz_browserdispatch INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/..)
