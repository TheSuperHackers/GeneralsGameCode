set(GRAPHICS_TEST_FIXTURE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test-shaders/$<CONFIG>")
set(_graphics_test_shader_source "${CMAKE_CURRENT_SOURCE_DIR}/tests/visual_regression/shaders")
set(_graphics_test_shader_outputs)

function(_graphics_add_test_shader name stage entry source)
    foreach(backend IN ITEMS dx11 dx12)
        if(backend STREQUAL dx12)
            set(compiler_options -target dxil -profile sm_6_6 -DGRAPHICS_DX12=1)
        else()
            set(compiler_options -target dxbc -profile sm_5_0)
        endif()
        set(output "${GRAPHICS_TEST_FIXTURE_DIRECTORY}/${backend}/${name}")
        add_custom_command(OUTPUT "${output}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${GRAPHICS_TEST_FIXTURE_DIRECTORY}/${backend}"
            COMMAND ${CMAKE_COMMAND} -E env
                --modify "PATH=path_list_prepend:$<TARGET_FILE_DIR:slang-bootstrap>"
                --modify "PATH=path_list_prepend:${GRAPHICS_DX12_DXC_DIRECTORY}"
                $<TARGET_FILE:slang-bootstrap> ${compiler_options} -O3
                -entry "${entry}" -stage "${stage}" -matrix-layout-row-major
                -o "${output}" "${_graphics_test_shader_source}/${source}"
            DEPENDS "${_graphics_test_shader_source}/${source}"
                "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh"
                slang-bootstrap
            VERBATIM)
        list(APPEND _graphics_test_shader_outputs "${output}")
    endforeach()
    set(_graphics_test_shader_outputs "${_graphics_test_shader_outputs}" PARENT_SCOPE)
endfunction()

_graphics_add_test_shader(visual_basic.vso vertex BasicVertex VisualRegression.slang)
_graphics_add_test_shader(visual_basic.pso pixel BasicPixel VisualRegression.slang)
_graphics_add_test_shader(visual_textured.pso pixel TexturedPixel VisualRegression.slang)
_graphics_add_test_shader(visual_shadow.vso vertex ShadowVertex VisualRegression.slang)
_graphics_add_test_shader(visual_shadow.pso pixel ShadowPixel VisualRegression.slang)
_graphics_add_test_shader(visual_lit_shadow.pso pixel LitShadowPixel VisualRegression.slang)
_graphics_add_test_shader(draw_constants.vso vertex DrawConstantsVertex DrawConstants.slang)
_graphics_add_test_shader(draw_constants.pso pixel DrawConstantsPixel DrawConstants.slang)

add_custom_target(generals_graphics_test_shaders DEPENDS ${_graphics_test_shader_outputs})
add_dependencies(generals_graphics_test_support generals_graphics_test_shaders)
target_compile_definitions(generals_graphics_test_support PRIVATE
    GRAPHICS_TEST_FIXTURE_DIRECTORY="${GRAPHICS_TEST_FIXTURE_DIRECTORY}")
