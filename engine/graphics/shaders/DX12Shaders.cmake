# Build the DX12 shader set beside the default shader set.  The .vso/.pso
# suffixes are retained because ShaderLibrary uses those names for all
# backends; the DX12 directory contains DXIL bytecode.

if(WIN32 AND TARGET slang-bootstrap AND DEFINED GRAPHICS_SHADER_ASSET_DIRECTORY)
    set(GRAPHICS_DX12_SHADER_ASSET_DIRECTORY "${GRAPHICS_SHADER_ASSET_DIRECTORY}/dx12")
    set(_GRAPHICS_DX12_SHADER_SOURCE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/source")

    set(GRAPHICS_DX12_DXC_DIRECTORY "" CACHE PATH
        "Directory containing dxcompiler.dll and dxil.dll for DXIL shader compilation")
    if(NOT GRAPHICS_DX12_DXC_DIRECTORY)
        find_package(directx-dxc CONFIG REQUIRED)
        get_filename_component(GRAPHICS_DX12_DXC_DIRECTORY
            "${DIRECTX_DXC_TOOL}" DIRECTORY)
    endif()
    set(_GRAPHICS_DX12_DXC_RUNTIME_FILES
        "${GRAPHICS_DX12_DXC_DIRECTORY}/dxcompiler.dll"
        "${GRAPHICS_DX12_DXC_DIRECTORY}/dxil.dll")
    foreach(_graphics_dx12_dxc_runtime IN LISTS _GRAPHICS_DX12_DXC_RUNTIME_FILES)
        if(NOT EXISTS "${_graphics_dx12_dxc_runtime}")
            message(FATAL_ERROR
                "DX12 shader compilation requires '${_graphics_dx12_dxc_runtime}'. "
                "Set GRAPHICS_DX12_DXC_DIRECTORY to a matching Slang DXC runtime directory.")
        endif()
    endforeach()

    set(_GRAPHICS_DX12_SHADER_COMMON_DEPENDENCIES
        "${_GRAPHICS_DX12_SHADER_SOURCE_DIRECTORY}/resource_access.slangh"
        "${_GRAPHICS_DX12_SHADER_SOURCE_DIRECTORY}/environment_lighting.slangh"
        "${_GRAPHICS_DX12_SHADER_SOURCE_DIRECTORY}/prop_surface.slangh")
    set(_GRAPHICS_DX12_SHADER_OUTPUTS)

    function(_graphics_add_dx12_shader_stage output_name stage entry source_name)
        set(output_path "${GRAPHICS_DX12_SHADER_ASSET_DIRECTORY}/${output_name}")
        set(stage_dependencies)
        if(source_name STREQUAL "ocean.slang" OR source_name STREQUAL "water_underwater.slang")
            list(APPEND stage_dependencies "${_GRAPHICS_DX12_SHADER_SOURCE_DIRECTORY}/water_constants.slangh")
        endif()
        add_custom_command(
            OUTPUT "${output_path}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${GRAPHICS_DX12_SHADER_ASSET_DIRECTORY}"
            COMMAND ${CMAKE_COMMAND} -E env
                --modify "PATH=path_list_prepend:$<TARGET_FILE_DIR:slang-bootstrap>"
                --modify "PATH=path_list_prepend:${GRAPHICS_DX12_DXC_DIRECTORY}"
                $<TARGET_FILE:slang-bootstrap>
                -target dxil
                -profile sm_6_6
                -O3
                -DGRAPHICS_DX12=1
                -entry "${entry}"
                -stage "${stage}"
                -matrix-layout-row-major
                -o "${output_path}"
                "${_GRAPHICS_DX12_SHADER_SOURCE_DIRECTORY}/${source_name}"
            DEPENDS
                "${_GRAPHICS_DX12_SHADER_SOURCE_DIRECTORY}/${source_name}"
                ${_GRAPHICS_DX12_SHADER_COMMON_DEPENDENCIES}
                ${stage_dependencies}
                ${_GRAPHICS_DX12_DXC_RUNTIME_FILES}
                slang-bootstrap
            VERBATIM)
        set(outputs "${_GRAPHICS_DX12_SHADER_OUTPUTS}")
        list(APPEND outputs "${output_path}")
        set(_GRAPHICS_DX12_SHADER_OUTPUTS "${outputs}" PARENT_SCOPE)
    endfunction()

    _graphics_add_dx12_shader_stage(basic_opaque.vso vertex basic_opaque_vertex basic_opaque.slang)
    _graphics_add_dx12_shader_stage(basic_opaque.pso pixel basic_opaque_pixel basic_opaque.slang)
    _graphics_add_dx12_shader_stage(basic_opaque_skinned.vso vertex basic_opaque_skinned_vertex basic_opaque.slang)
    _graphics_add_dx12_shader_stage(particle_billboard.vso vertex particle_billboard_vertex particle_billboard.slang)
    _graphics_add_dx12_shader_stage(particle_billboard.pso pixel particle_billboard_pixel particle_billboard.slang)
    _graphics_add_dx12_shader_stage(screen_distortion.vso vertex screen_distortion_vertex screen_distortion.slang)
    _graphics_add_dx12_shader_stage(screen_distortion.pso pixel screen_distortion_pixel screen_distortion.slang)
    _graphics_add_dx12_shader_stage(beam.vso vertex beam_vertex beam.slang)
    _graphics_add_dx12_shader_stage(beam.pso pixel beam_pixel beam.slang)
    _graphics_add_dx12_shader_stage(laser_core.vso vertex laser_core_vertex laser.slang)
    _graphics_add_dx12_shader_stage(laser_core.pso pixel laser_core_pixel laser.slang)
    _graphics_add_dx12_shader_stage(laser_distortion.vso vertex laser_distortion_vertex laser.slang)
    _graphics_add_dx12_shader_stage(laser_distortion.pso pixel laser_distortion_pixel laser.slang)
    _graphics_add_dx12_shader_stage(video.vso vertex video_vertex video.slang)
    _graphics_add_dx12_shader_stage(video.pso pixel video_pixel video.slang)
    _graphics_add_dx12_shader_stage(ring.vso vertex ring_vertex ring.slang)
    _graphics_add_dx12_shader_stage(ring.pso pixel ring_pixel ring.slang)
    _graphics_add_dx12_shader_stage(fullscreen_overlay.vso vertex fullscreen_overlay_vertex fullscreen_overlay.slang)
    _graphics_add_dx12_shader_stage(fullscreen_overlay.pso pixel fullscreen_overlay_pixel fullscreen_overlay.slang)
    _graphics_add_dx12_shader_stage(world_quad.vso vertex world_quad_vertex world_quad.slang)
    _graphics_add_dx12_shader_stage(world_quad.pso pixel world_quad_pixel world_quad.slang)
    _graphics_add_dx12_shader_stage(ui_2d.vso vertex ui_2d_vertex ui_2d.slang)
    _graphics_add_dx12_shader_stage(ui_2d.pso pixel ui_2d_pixel ui_2d.slang)
    _graphics_add_dx12_shader_stage(terrain.vso vertex terrain_vertex terrain.slang)
    _graphics_add_dx12_shader_stage(terrain.pso pixel terrain_pixel terrain.slang)
    _graphics_add_dx12_shader_stage(surface.vso vertex surface_vertex surface.slang)
    _graphics_add_dx12_shader_stage(surface.pso pixel surface_pixel surface.slang)
    _graphics_add_dx12_shader_stage(screen_filter.vso vertex screen_filter_vertex screen_filter.slang)
    _graphics_add_dx12_shader_stage(screen_filter.pso pixel screen_filter_pixel screen_filter.slang)
    _graphics_add_dx12_shader_stage(ssao.vso vertex ssao_vertex ssao.slang)
    _graphics_add_dx12_shader_stage(ssao.pso pixel ssao_pixel ssao.slang)
    _graphics_add_dx12_shader_stage(light_rays.vso vertex light_rays_vertex light_rays.slang)
    _graphics_add_dx12_shader_stage(light_rays.pso pixel light_rays_pixel light_rays.slang)
    _graphics_add_dx12_shader_stage(bloom.vso vertex bloom_vertex bloom.slang)
    _graphics_add_dx12_shader_stage(bloom.pso pixel bloom_pixel bloom.slang)
    _graphics_add_dx12_shader_stage(tree.vso vertex tree_vertex tree.slang)
    _graphics_add_dx12_shader_stage(tree.pso pixel tree_pixel tree.slang)
    _graphics_add_dx12_shader_stage(ocean.vso vertex OceanVertexMain ocean.slang)
    _graphics_add_dx12_shader_stage(ocean_instanced.vso vertex OceanVertexInstanced ocean.slang)
    _graphics_add_dx12_shader_stage(ocean.pso pixel OceanPixelMain ocean.slang)
    _graphics_add_dx12_shader_stage(water_underwater.vso vertex UnderwaterVertexMain water_underwater.slang)
    _graphics_add_dx12_shader_stage(water_underwater_instanced.vso vertex UnderwaterVertexInstanced water_underwater.slang)
    _graphics_add_dx12_shader_stage(water_underwater.pso pixel UnderwaterPixelMain water_underwater.slang)
    _graphics_add_dx12_shader_stage(water_surface.vso vertex WaterSurfaceVertexMain water_surface.slang)
    _graphics_add_dx12_shader_stage(water_surface.pso pixel WaterSurfacePixelMain water_surface.slang)
    _graphics_add_dx12_shader_stage(water_displacement.vso vertex WaterDisplacementVertexMain water_displacement.slang)
    _graphics_add_dx12_shader_stage(water_displacement.pso pixel WaterDisplacementPixelMain water_displacement.slang)
    _graphics_add_dx12_shader_stage(water_sky.vso vertex WaterSkyVertexMain water_sky.slang)
    _graphics_add_dx12_shader_stage(water_sky.pso pixel WaterSkyPixelMain water_sky.slang)
    _graphics_add_dx12_shader_stage(water_track.vso vertex WaterTrackVertexMain water_track.slang)
    _graphics_add_dx12_shader_stage(water_track.pso pixel WaterTrackPixelMain water_track.slang)
    _graphics_add_dx12_shader_stage(prop.vso vertex prop_vertex prop.slang)
    _graphics_add_dx12_shader_stage(prop_instanced.vso vertex prop_vertex_instanced prop.slang)
    _graphics_add_dx12_shader_stage(prop.pso pixel prop_pixel prop.slang)
    _graphics_add_dx12_shader_stage(prop_records.vso vertex prop_vertex_records prop.slang)
    _graphics_add_dx12_shader_stage(prop_records.pso pixel prop_pixel_records prop.slang)

    add_custom_target(generals_graphics_dx12_shaders
        DEPENDS ${_GRAPHICS_DX12_SHADER_OUTPUTS})
    if(TARGET generals_graphics)
        add_dependencies(generals_graphics generals_graphics_dx12_shaders)
    endif()
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_shaders generals_graphics_dx12_shaders)
    endif()
    if(TARGET generals_graphics)
        set_property(TARGET generals_graphics PROPERTY GRAPHICS_DX12_SHADER_ASSET_DIRECTORY
            "${GRAPHICS_DX12_SHADER_ASSET_DIRECTORY}")
    endif()
endif()
