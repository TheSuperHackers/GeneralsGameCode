cmake_minimum_required(VERSION 3.28)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(generals_assets LANGUAGES CXX)
endif()

add_library(generals_assets STATIC)

# Keep each module in the responsibility group that owns its public contract.
target_sources(generals_assets
    PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES
            core/AssetHandle.cppm
            core/AssetState.cppm
            core/AssetIdentity.cppm
            core/AssetMath.cppm
            image/PixelEncoding.cppm
            image/ImageBuffer.cppm
            image/ImageColor.cppm
            image/ImagePreparation.cppm
            material/MaterialAsset.cppm
            material/TextureMapping.cppm
            adapters/w3d/materials/W3DTextureMapping.cppm
            model/ModelAsset.cppm
            model/ModelRig.cppm
            model/MeshBoundsTree.cppm
            particle/EmitterAsset.cppm
            ring/RingAsset.cppm
            dazzle/DazzleAsset.cppm
            adapters/w3d/dazzle/W3DDazzle.cppm
            light/LightAsset.cppm
            adapters/w3d/light/W3DLight.cppm
            sphere/SphereAsset.cppm
            model/ModelAssembly.cppm
            model/AnimationAsset.cppm
            texture/TextureAsset.cppm
            font/FontAsset.cppm
            import/ModelImporter.cppm
            cache/ModelLoadTask.cppm
            cache/MaterialLoadTask.cppm
            cache/TextureLoadTask.cppm
            cache/FontLoadTask.cppm
            cache/AssetCache.cppm
            cache/AssetReport.cppm
            cache/SkeletonCache.cppm
            cache/AnimationCache.cppm
            runtime/AssetRuntime.cppm
            adapters/dds/DDSImage.cppm
            adapters/tga/TGAPixelEncoding.cppm
            adapters/tga/TGAImage.cppm
            adapters/w3d/chunks/W3DChunkReader.cppm
            adapters/w3d/materials/W3DMaterialParser.cppm
            adapters/w3d/materials/W3DShaderMaterials.cppm
            adapters/w3d/materials/W3DSurfaceMaterial.cppm
            adapters/w3d/materials/W3DPassBindings.cppm
            adapters/w3d/mesh/W3DMeshParser.cppm
            adapters/w3d/mesh/W3DGeometry.cppm
            adapters/w3d/mesh/W3DMeshData.cppm
            adapters/w3d/model/W3DModelBuilder.cppm
            adapters/w3d/model/W3DBox.cppm
            adapters/w3d/null/W3DNull.cppm
            adapters/w3d/model/W3DRig.cppm
            adapters/w3d/model/W3DAssembly.cppm
            adapters/w3d/particles/W3DEmitter.cppm
            adapters/w3d/ring/W3DRing.cppm
            adapters/w3d/sphere/W3DSphere.cppm
            adapters/w3d/model/W3DCollection.cppm
            adapters/w3d/model/W3DLevelSet.cppm
            adapters/w3d/model/W3DAggregate.cppm
            adapters/w3d/model/W3DRetention.cppm
            adapters/w3d/model/W3DAnimationChannels.cppm
            adapters/w3d/W3DAdapter.cppm
            adapters/w3d/W3DRequestPolicy.cppm
)

target_compile_features(generals_assets PUBLIC cxx_std_23)

if(TARGET stb)
    target_link_libraries(generals_assets PUBLIC stb)
endif()

option(RTS_BUILD_ASSET_TESTS "Build asset system unit and integration tests." ${RTS_BUILD_GRAPHICS_TESTS})

if(RTS_BUILD_ASSET_TESTS)
    include(CTest)
    find_package(Boost CONFIG REQUIRED)

    function(add_assets_test target_name source_file)
        add_executable(${target_name})
        target_sources(${target_name}
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    "${source_file}"
        )
        target_compile_features(${target_name} PRIVATE cxx_std_23)
        target_include_directories(${target_name} PRIVATE ${Boost_INCLUDE_DIRS})
        target_link_libraries(${target_name} PRIVATE generals_assets)
        add_test(NAME ${target_name} COMMAND ${target_name})
    endfunction()

    # Every production module has one colocated test module. Keeping these as
    # separate executables prevents a test driver from becoming another god
    # file and makes responsibility failures easy to identify in CTest.
    add_assets_test(generals_assets_dds_image_test adapters/dds/DDSImage.test.cppm)
    add_assets_test(generals_assets_report_test cache/AssetReport.test.cppm)
    add_assets_test(generals_assets_w3d_collection_test adapters/w3d/model/W3DCollection.test.cppm)
    add_assets_test(generals_assets_w3d_level_set_test adapters/w3d/model/W3DLevelSet.test.cppm)
    add_assets_test(generals_assets_w3d_aggregate_test adapters/w3d/model/W3DAggregate.test.cppm)
    add_assets_test(generals_assets_w3d_retention_test adapters/w3d/model/W3DRetention.test.cppm)
    add_assets_test(generals_assets_pixel_encoding_test image/PixelEncoding.test.cppm)
    add_assets_test(generals_assets_tga_pixel_encoding_test adapters/tga/TGAPixelEncoding.test.cppm)
    add_assets_test(generals_assets_tga_image_test adapters/tga/TGAImage.test.cppm)
    add_assets_test(generals_assets_image_preparation_test image/ImagePreparation.test.cppm)
    add_assets_test(generals_assets_image_buffer_test image/ImageBuffer.test.cppm)
    add_assets_test(generals_assets_image_color_test image/ImageColor.test.cppm)
    add_assets_test(generals_assets_handle_test core/AssetHandle.test.cppm)
    add_assets_test(generals_assets_w3d_pass_bindings_test adapters/w3d/materials/W3DPassBindings.test.cppm)
    add_assets_test(generals_assets_w3d_surface_material_test adapters/w3d/materials/W3DSurfaceMaterial.test.cppm)
    add_assets_test(generals_assets_state_test core/AssetState.test.cppm)
    add_assets_test(generals_assets_identity_test core/AssetIdentity.test.cppm)
    add_assets_test(generals_assets_math_test core/AssetMath.test.cppm)
    add_assets_test(generals_assets_material_test material/MaterialAsset.test.cppm)
    add_assets_test(generals_assets_model_test model/ModelAsset.test.cppm)
    add_assets_test(generals_assets_model_rig_test model/ModelRig.test.cppm)
    add_assets_test(generals_assets_mesh_bounds_tree_test model/MeshBoundsTree.test.cppm)
    add_assets_test(generals_assets_sphere_test sphere/SphereAsset.test.cppm)
    add_assets_test(generals_assets_model_assembly_test model/ModelAssembly.test.cppm)
    add_assets_test(generals_assets_animation_test model/AnimationAsset.test.cppm)
    add_assets_test(generals_assets_texture_test texture/TextureAsset.test.cppm)
    add_assets_test(generals_assets_font_test font/FontAsset.test.cppm)
    add_assets_test(generals_assets_font_load_test cache/FontLoadTask.test.cppm)
    target_compile_definitions(generals_assets_font_load_test PRIVATE
        ASSETS_FONT_METRICS_FIXTURE="${CMAKE_CURRENT_SOURCE_DIR}/font/fixtures/metrics.ttf")
    add_assets_test(generals_assets_importer_test import/ModelImporter.test.cppm)
    add_assets_test(generals_assets_load_task_test cache/ModelLoadTask.test.cppm)
    add_assets_test(generals_assets_material_load_task_test cache/MaterialLoadTask.test.cppm)
    add_assets_test(generals_assets_texture_load_task_test cache/TextureLoadTask.test.cppm)
    add_assets_test(generals_assets_cache_test cache/AssetCache.test.cppm)
    add_assets_test(generals_assets_runtime_test runtime/AssetRuntime.test.cppm)
    add_assets_test(generals_assets_w3d_chunks_test adapters/w3d/chunks/W3DChunkReader.test.cppm)
    add_assets_test(generals_assets_w3d_materials_test adapters/w3d/materials/W3DMaterialParser.test.cppm)
    add_assets_test(generals_assets_w3d_shader_materials_test adapters/w3d/materials/W3DShaderMaterials.test.cppm)
    add_assets_test(generals_assets_w3d_mesh_test adapters/w3d/mesh/W3DMeshParser.test.cppm)
    add_assets_test(generals_assets_w3d_geometry_test adapters/w3d/mesh/W3DGeometry.test.cppm)
    add_assets_test(generals_assets_w3d_mesh_data_test adapters/w3d/mesh/W3DMeshData.test.cppm)
    add_assets_test(generals_assets_w3d_model_test adapters/w3d/model/W3DModelBuilder.test.cppm)
    add_assets_test(generals_assets_w3d_box_test adapters/w3d/model/W3DBox.test.cppm)
    add_assets_test(generals_assets_w3d_null_test adapters/w3d/null/W3DNull.test.cppm)
    add_assets_test(generals_assets_w3d_rig_test adapters/w3d/model/W3DRig.test.cppm)
    add_assets_test(generals_assets_w3d_assembly_test adapters/w3d/model/W3DAssembly.test.cppm)
    add_assets_test(generals_assets_w3d_emitter_test adapters/w3d/particles/W3DEmitter.test.cppm)
    add_assets_test(generals_assets_w3d_ring_test adapters/w3d/ring/W3DRing.test.cppm)
    add_assets_test(generals_assets_dazzle_test dazzle/DazzleAsset.test.cppm)
    add_assets_test(generals_assets_w3d_dazzle_test adapters/w3d/dazzle/W3DDazzle.test.cppm)
    add_assets_test(generals_assets_light_test light/LightAsset.test.cppm)
    add_assets_test(generals_assets_w3d_light_test adapters/w3d/light/W3DLight.test.cppm)
    add_assets_test(generals_assets_w3d_sphere_test adapters/w3d/sphere/W3DSphere.test.cppm)
    add_assets_test(generals_assets_skeleton_cache_test cache/SkeletonCache.test.cppm)
    add_assets_test(generals_assets_animation_cache_test cache/AnimationCache.test.cppm)
    add_assets_test(generals_assets_w3d_animation_channels_test adapters/w3d/model/W3DAnimationChannels.test.cppm)
    add_assets_test(generals_assets_w3d_adapter_test adapters/w3d/W3DAdapter.test.cppm)
    add_assets_test(generals_assets_w3d_request_policy_test adapters/w3d/W3DRequestPolicy.test.cppm)

    # This is intentionally a path to a real, externally supplied game asset.
    # No proprietary W3D data is committed to the repository.
    set(RTS_W3D_INTEGRATION_ASSET "" CACHE FILEPATH
        "Loose real W3D model used by the asset integration test (for example ABArFrcCmd_A2.W3D).")
    if(RTS_W3D_INTEGRATION_ASSET)
        file(TO_CMAKE_PATH "${RTS_W3D_INTEGRATION_ASSET}" RTS_W3D_INTEGRATION_ASSET_CMAKE)
        target_compile_definitions(generals_assets_w3d_adapter_test PRIVATE
            "ASSETS_W3D_INTEGRATION_ASSET=\"${RTS_W3D_INTEGRATION_ASSET_CMAKE}\"")
    endif()

    # The normal Zero Hour build stages loose Art/W3D files next to the
    # executable. This generator expression selects the active configuration.
    target_compile_definitions(generals_assets_w3d_adapter_test PRIVATE
        "ASSETS_W3D_STAGED_ASSET=\"${CMAKE_BINARY_DIR}/GeneralsMD/$<CONFIG>/Art/W3D/ABArFrcCmd_A2.W3D\"")
endif()
