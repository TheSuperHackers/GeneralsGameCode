add_library(browserdispatch INTERFACE)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    add_custom_command(
        OUTPUT BrowserDispatch_i.c BrowserDispatch.h
        COMMAND midl.exe "${CMAKE_CURRENT_LIST_DIR}\\BrowserDispatch.idl" /header BrowserDispatch.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/BrowserDispatch.idl"
        VERBATIM
    )
    add_library(browserdispatchwin STATIC BrowserDispatch_i.c)
    target_link_libraries(browserdispatch INTERFACE browserdispatchwin)
endif()

target_include_directories(browserdispatch INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/..)
