add_library(core_browserengine INTERFACE)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(WIN32)
        add_custom_command(
            OUTPUT BrowserEngine_i.c BrowserEngine.h BrowserEngine.tlb
            COMMAND ${CMAKE_COMMAND} -E env
                "PATH=${LLVM_BIN_DIRECTORY};$ENV{PATH}"
                "${MIDL_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}\\BrowserEngine.idl"
                /cpp_cmd clang-cl.exe
                /I "${WINDOWS_SDK_UM_INCLUDE_DIRECTORY}"
                /I "${WINDOWS_SDK_SHARED_INCLUDE_DIRECTORY}"
                /header BrowserEngine.h
                /iid BrowserEngine_i.c
                /tlb BrowserEngine.tlb
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS "${CMAKE_CURRENT_LIST_DIR}/BrowserEngine.idl"
            VERBATIM
        )
        add_library(core_browserenginewin STATIC BrowserEngine_i.c BrowserEngine.h BrowserEngine.tlb)
    endif()
    
    if(TARGET core_browserenginewin)
        set_target_properties(core_browserenginewin PROPERTIES OUTPUT_NAME browserenginewin)
        target_link_libraries(core_browserengine INTERFACE core_browserenginewin)
    endif()
endif()

target_include_directories(core_browserengine INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/..)
