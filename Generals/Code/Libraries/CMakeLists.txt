# WW common libraries decended from earliest C&C games
add_subdirectory(Source/WWVegas)

target_include_directories(g_ww3d2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/WWVegas/WW3D2
)

if(RTS_BUILD_DIRECTX9)
    target_include_directories(g_ww3d2 PRIVATE ${DIRECTX_INCLUDE_DIR})
endif()
