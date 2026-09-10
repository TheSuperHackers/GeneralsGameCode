cmake_minimum_required(VERSION 3.28)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(generals_graphics LANGUAGES CXX)
endif()

add_library(generals_graphics STATIC)

target_sources(generals_graphics
    PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES
            diagnostics/RenderDiagnostics.cppm
            cursors/CursorLoad.cppm
            cursors/Cursor.cppm
            presentation/DisplayModes.cppm
            resources/handles/ResourceHandle.cppm
            memory/AlignedAllocator.cppm
            resources/pools/ResourcePool.cppm
            resources/meshes/Mesh.cppm
            resources/textures/Texture.cppm
            resources/textures/TextureCachePolicy.cppm
            resources/textures/TextureResidency.cppm
            resources/loading/ResourceLoadQueue.cppm
            resources/textures/TextureUpload.cppm
            resources/textures/TextureLoad.cppm
            resources/textures/TextureAtlas.cppm
            resources/ResourceRecreation.cppm
            resources/textures/TextureResource.cppm
            resources/textures/TextureEdit.cppm
            resources/textures/TextureSnapshot.cppm
            resources/textures/TextureReferences.cppm
            resources/textures/TextureSampling.cppm
            resources/textures/TextureStorage.cppm
            resources/textures/TextureQuality.cppm
            materials/TextureCoordinates.cppm
            materials/TextureProjection.cppm
            materials/TextureProjector.cppm
            materials/TextureMapping.cppm
            materials/MeshTextureMapping.cppm
            materials/MeshMaterial.cppm
            materials/W3DMeshMaterial.cppm
            scene/views/CameraMatrices.cppm
            scene/views/CameraProjection.cppm
            scene/views/CameraFrustum.cppm
            scene/views/CameraState.cppm
            scene/RenderObjectState.cppm
            scene/RenderObjectBounds.cppm
            scene/RenderObjectHierarchy.cppm
            scene/RenderObjectLOD.cppm
            scene/RenderObjectDrawing.cppm
            scene/SceneDrawParameters.cppm
            scene/DrawContext.cppm
            materials/MaterialState.cppm
            materials/MaterialOrdering.cppm
            materials/W3DMaterialState.cppm
            materials/ProceduralPass.cppm
            scene/props/PropDrawState.cppm
            materials/Fog.cppm
            resources/samplers/Sampler.cppm
            resources/materials/Material.cppm
            resources/residency/GPUResourceResidency.cppm
            resources/bindless/BindlessResourceTable.cppm
            shaders/ParameterLayout.cppm
            shaders/Pipeline.cppm
            shaders/ShaderLibrary.cppm
            scene/terrain/TerrainVisibility.cppm
            scene/terrain/TerrainGeometry.cppm
            scene/terrain/TerrainRenderer.cppm
            scene/surfaces/SurfaceGeometry.cppm
            scene/primitives/PrimitiveGeometry.cppm
            scene/surfaces/SurfaceRenderer.cppm
            scene/lines/LineDrawing.cppm
            scene/lines/Tracer.cppm
            scene/bibs/BibRenderer.cppm
            scene/roads/RoadRenderer.cppm
            scene/shadows/StencilVolumeRenderer.cppm
            scene/screen/ScreenFilterRenderer.cppm
            scene/debug/DebugRenderer.cppm
            scene/debug/CollisionBox.cppm
            scene/shadows/ProjectedShadowRenderer.cppm
            scene/shadows/ProjectedShadowCapture.cppm
            scene/props/PropLighting.cppm
            scene/props/PropGeometry.cppm
            scene/props/PropExtraction.cppm
            scene/muzzle_flash/MuzzleFlash.cppm
            scene/props/PropMaterialSubmission.cppm
            scene/props/PropMaterial.cppm
            scene/props/PropSurface.cppm
            scene/props/PropAssetGeometry.cppm
            scene/props/PropSkinning.cppm
            scene/props/PropAssetBinding.cppm
            scene/props/PropConstants.cppm
            scene/props/PropInstances.cppm
            scene/props/PropSkinPalettes.cppm
            scene/props/PropSkinBounds.cppm
            scene/props/PropBatchBounds.cppm
            scene/props/PropRenderer.cppm
            scene/props/PropMeshSet.cppm
            scene/props/PropSubmission.cppm
            scene/props/TransparentGeometry.cppm
            scene/props/MaterialPassQueue.cppm
            scene/water/WaterGeometry.cppm
            scene/water/OceanPatches.cppm
            scene/water/WaterView.cppm
            scene/water/WaterRenderer.cppm
            scene/water/OceanDisplacement.cppm
            scene/water/WaterWaves.cppm
            scene/trees/TreeGeometry.cppm
            scene/trees/TreeRenderer.cppm
            scene/tracks/TrackGeometry.cppm
            scene/scorches/ScorchGeometry.cppm
            scene/bridges/BridgeRenderer.cppm
            scene/RenderScene.cppm
            scene/attachments/Attachments.cppm
            scene/meshes/StaticMeshRenderer.cppm
            scene/lighting/LocalLighting.cppm
            scene/props/PropLightingParameters.cppm
            scene/lighting/Lighting.cppm
            scene/lighting/LightState.cppm
            scene/lighting/EnvironmentLighting.cppm
            scene/lighting/LightRenderer.cppm
            scene/environment/Environment.cppm
            scene/shadows/Shadows.cppm
            scene/shroud/ShroudImage.cppm
            scene/shadows/DirectionalShadowRenderer.cppm
            scene/decals/Decals.cppm
            scene/decals/DecalVisibility.cppm
            scene/decals/DecalDraw.cppm
            scene/world/WorldQuads.cppm
            scene/particles/Particles.cppm
            scene/particles/SpriteGeometry.cppm
            scene/particles/EmitterKinematics.cppm
            scene/particles/EmitterVisualState.cppm
            scene/particles/EmitterEmission.cppm
            scene/particles/EmitterDetail.cppm
            scene/particles/EmitterRenderer.cppm
            scene/particles/LineGroupGeometry.cppm
            scene/beams/RibbonSubdivision.cppm
            scene/beams/RibbonMath.cppm
            scene/beams/RibbonIntersections.cppm
            scene/beams/RibbonGeometry.cppm
            scene/beams/RibbonEdges.cppm
            scene/beams/RibbonTextureCoordinates.cppm
            scene/beams/RibbonPipeline.cppm
            scene/beams/SegmentedLine.cppm
            scene/particles/ParticleVisibility.cppm
            scene/particles/ParticleDraw.cppm
            scene/particles/ParticleRenderer.cppm
            scene/weather/WeatherParticles.cppm
            scene/screen/ScreenDistortion.cppm
            scene/screen/FullscreenOverlay.cppm
            scene/ring/Ring.cppm
            scene/ring/RingRuntime.cppm
            scene/ring/AuthoredRingRenderer.cppm
            scene/sphere/Sphere.cppm
            scene/beams/Beam.cppm
            scene/beams/Laser.cppm
            scene/models/ModelVisibility.cppm
            scene/models/Skeleton.cppm
            scene/models/ModelInstance.cppm
            scene/models/Animation.cppm
            scene/models/AnimationBlend.cppm
            scene/models/ModelAssetPose.cppm
            scene/models/AnimationChannels.cppm
            scene/models/ClipSampling.cppm
            scene/models/ModelHierarchy.cppm
            scene/models/ModelPlayback.cppm
            scene/models/ModelGeometrySource.cppm
            scene/models/ModelDeformation.cppm
            scene/models/ModelGeometryMath.cppm
            scene/models/ModelGeometryQueries.cppm
            scene/models/ModelMeshDrawing.cppm
            scene/dazzles/DazzleState.cppm
            scene/dazzles/DazzleLayer.cppm
            scene/dazzles/DazzleResources.cppm
            scene/dazzles/DazzleDrawing.cppm
            scene/models/ModelObjectDrawing.cppm
            scene/models/ModelChildren.cppm
            scene/models/ModelDetailLevels.cppm
            scene/AffineTransform.cppm
            scene/models/AnimationRotation.cppm
            scene/models/Skinning.cppm
            scene/models/ModelAssetGeometry.cppm
            scene/models/ModelAssetBinding.cppm
            scene/transparency/Transparency.cppm
            scene/views/View.cppm
            scene/visibility/Visibility.cppm
            scene/lod/LOD.cppm
            scene/models/ModelFactoryStore.cppm
            scene/models/VertexChannels.cppm
            scene/models/SourceRevision.cppm
            scene/models/MaterialSlots.cppm
            scene/models/ModelMaterials.cppm
            scene/models/MeshMaterialBindings.cppm
            scene/models/MeshMaterialPreparation.cppm
            scene/models/W3DMeshMaterialLoading.cppm
            scene/models/ModelBoundsTree.cppm
            scene/models/ModelFactory.cppm
            scene/gpu/GPUScene.cppm
            scene/draw/DrawGeneration.cppm
            render-graph/RenderGraph.cppm
            render-graph/RenderGraphExecution.cppm
            text/GlyphAtlas.cppm
            renderer2d/Renderer2D.cppm
            rhi/RHI.cppm
            frame/FrameTargets.cppm
            capture/FrameCapture.cppm
            capture/FramePreview.cppm
            capture/MovieCapture.cppm
            frame/RenderClock.cppm
            frame/RenderSettings.cppm
            frame/FrameOwner.cppm
            frame/SubmissionStatistics.cppm
            frame/FrameResourceLifecycle.cppm
            frame/AttachmentBindings.cppm
            scene/SceneObjectList.cppm
            scene/ScenePass.cppm
            scene/Traversal.cppm
            scene/OrderedDraws.cppm
            frame/ToolFrame.cppm
            frame/SceneRenderers.cppm
            render-graph/Frame.cppm
            passes/opaque/OpaquePass.cppm
            passes/shadow/ShadowPass.cppm
            passes/transparent/TransparentPass.cppm
            passes/decal/DecalPass.cppm
            passes/particles/ParticlePass.cppm
            passes/post-process/PostProcess.cppm
            passes/post-process/Bloom.cppm
            passes/post-process/SSAO.cppm
            passes/post-process/LightRays.cppm
            passes/sky/SkyPass.cppm
            passes/video/VideoPass.cppm
            video/VideoRenderer.cppm
)

target_compile_features(generals_graphics PUBLIC cxx_std_20)

if(NOT TARGET SDL3::SDL3)
    find_package(SDL3 CONFIG REQUIRED)
endif()
target_link_libraries(generals_graphics PRIVATE SDL3::SDL3)

if(TARGET generals_assets)
    target_link_libraries(generals_graphics PUBLIC generals_assets)
endif()

if(TARGET generals_geometry)
    target_link_libraries(generals_graphics PUBLIC generals_geometry)
endif()

if(TARGET generals_video)
    target_link_libraries(generals_graphics PUBLIC generals_video)
endif()

if(TARGET core_profile_tracy)
    target_link_libraries(generals_graphics PUBLIC core_profile_tracy)
endif()

if(WIN32)
	set(GRAPHICS_SHADER_ASSET_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/shaders/precompiled/dx11")
	if(TARGET slang-bootstrap)
		set(GRAPHICS_SHADER_ASSET_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shaders/$<CONFIG>")
		set(GRAPHICS_BASIC_OPAQUE_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/basic_opaque.vso")
		set(GRAPHICS_SKINNED_BASIC_OPAQUE_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/basic_opaque_skinned.vso")
		set(GRAPHICS_BASIC_OPAQUE_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/basic_opaque.pso")
		set(GRAPHICS_PARTICLE_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/particle_billboard.vso")
		set(GRAPHICS_PARTICLE_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/particle_billboard.pso")
		set(GRAPHICS_SCREEN_DISTORTION_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/screen_distortion.vso")
		set(GRAPHICS_SCREEN_DISTORTION_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/screen_distortion.pso")
		set(GRAPHICS_BEAM_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/beam.vso")
		set(GRAPHICS_BEAM_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/beam.pso")
		set(GRAPHICS_LASER_CORE_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/laser_core.vso")
		set(GRAPHICS_LASER_CORE_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/laser_core.pso")
		set(GRAPHICS_LASER_DISTORTION_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/laser_distortion.vso")
		set(GRAPHICS_LASER_DISTORTION_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/laser_distortion.pso")
		set(GRAPHICS_VIDEO_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/video.vso")
		set(GRAPHICS_VIDEO_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/video.pso")
		set(GRAPHICS_RING_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ring.vso")
		set(GRAPHICS_RING_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ring.pso")
		set(GRAPHICS_FULLSCREEN_OVERLAY_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/fullscreen_overlay.vso")
		set(GRAPHICS_FULLSCREEN_OVERLAY_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/fullscreen_overlay.pso")
		set(GRAPHICS_WORLD_QUAD_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/world_quad.vso")
		set(GRAPHICS_WORLD_QUAD_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/world_quad.pso")
		set(GRAPHICS_SURFACE_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/surface.vso")
		set(GRAPHICS_SCREEN_FILTER_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/screen_filter.vso")
		set(GRAPHICS_SURFACE_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/surface.pso")
		set(GRAPHICS_SCREEN_FILTER_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/screen_filter.pso")
		set(GRAPHICS_TERRAIN_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/terrain.vso")
		set(GRAPHICS_TERRAIN_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/terrain.pso")
		set(GRAPHICS_UI_2D_VERTEX_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ui_2d.vso")
		set(GRAPHICS_UI_2D_FRAGMENT_SHADER "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ui_2d.pso")
		add_custom_command(
			OUTPUT
				"${GRAPHICS_BASIC_OPAQUE_VERTEX_SHADER}"
				"${GRAPHICS_SKINNED_BASIC_OPAQUE_VERTEX_SHADER}"
				"${GRAPHICS_BASIC_OPAQUE_FRAGMENT_SHADER}"
				"${GRAPHICS_PARTICLE_VERTEX_SHADER}"
				"${GRAPHICS_PARTICLE_FRAGMENT_SHADER}"
				"${GRAPHICS_SCREEN_DISTORTION_VERTEX_SHADER}"
				"${GRAPHICS_SCREEN_DISTORTION_FRAGMENT_SHADER}"
				"${GRAPHICS_BEAM_VERTEX_SHADER}"
				"${GRAPHICS_BEAM_FRAGMENT_SHADER}"
				"${GRAPHICS_LASER_CORE_VERTEX_SHADER}"
				"${GRAPHICS_LASER_CORE_FRAGMENT_SHADER}"
				"${GRAPHICS_LASER_DISTORTION_VERTEX_SHADER}"
				"${GRAPHICS_LASER_DISTORTION_FRAGMENT_SHADER}"
				"${GRAPHICS_VIDEO_VERTEX_SHADER}"
				"${GRAPHICS_VIDEO_FRAGMENT_SHADER}"
				"${GRAPHICS_RING_VERTEX_SHADER}"
				"${GRAPHICS_RING_FRAGMENT_SHADER}"
				"${GRAPHICS_FULLSCREEN_OVERLAY_VERTEX_SHADER}"
				"${GRAPHICS_FULLSCREEN_OVERLAY_FRAGMENT_SHADER}"
				"${GRAPHICS_WORLD_QUAD_VERTEX_SHADER}"
				"${GRAPHICS_WORLD_QUAD_FRAGMENT_SHADER}"
				"${GRAPHICS_TERRAIN_VERTEX_SHADER}"
				"${GRAPHICS_SURFACE_VERTEX_SHADER}"
				"${GRAPHICS_SCREEN_FILTER_VERTEX_SHADER}"
				"${GRAPHICS_SURFACE_FRAGMENT_SHADER}"
				"${GRAPHICS_SCREEN_FILTER_FRAGMENT_SHADER}"
				"${GRAPHICS_TERRAIN_FRAGMENT_SHADER}"
				"${GRAPHICS_UI_2D_VERTEX_SHADER}"
				"${GRAPHICS_UI_2D_FRAGMENT_SHADER}"
			COMMAND ${CMAKE_COMMAND} -E make_directory "${GRAPHICS_SHADER_ASSET_DIRECTORY}"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry basic_opaque_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_BASIC_OPAQUE_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/basic_opaque.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry basic_opaque_pixel
				-stage pixel
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_BASIC_OPAQUE_FRAGMENT_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/basic_opaque.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry basic_opaque_skinned_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_SKINNED_BASIC_OPAQUE_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/basic_opaque.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry particle_billboard_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_PARTICLE_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/particle_billboard.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry particle_billboard_pixel
				-stage pixel
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_PARTICLE_FRAGMENT_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/particle_billboard.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry screen_distortion_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_SCREEN_DISTORTION_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/screen_distortion.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry screen_distortion_pixel
				-stage pixel
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_SCREEN_DISTORTION_FRAGMENT_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/screen_distortion.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry beam_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_BEAM_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/beam.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry beam_pixel
				-stage pixel
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_BEAM_FRAGMENT_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/beam.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry laser_core_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_LASER_CORE_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/laser.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry laser_core_pixel
				-stage pixel
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_LASER_CORE_FRAGMENT_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/laser.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry laser_distortion_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_LASER_DISTORTION_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/laser.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry laser_distortion_pixel
				-stage pixel
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_LASER_DISTORTION_FRAGMENT_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/laser.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry video_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_VIDEO_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/video.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry video_pixel
				-stage pixel
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_VIDEO_FRAGMENT_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/video.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry ring_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_RING_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ring.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry ring_pixel
				-stage pixel
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_RING_FRAGMENT_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ring.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry fullscreen_overlay_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_FULLSCREEN_OVERLAY_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/fullscreen_overlay.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry fullscreen_overlay_pixel
				-stage pixel
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_FULLSCREEN_OVERLAY_FRAGMENT_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/fullscreen_overlay.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry world_quad_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_WORLD_QUAD_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/world_quad.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry world_quad_pixel
				-stage pixel
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_WORLD_QUAD_FRAGMENT_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/world_quad.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry ui_2d_vertex
				-stage vertex
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_UI_2D_VERTEX_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ui_2d.slang"
			COMMAND $<TARGET_FILE:slang-bootstrap>
				-target dxbc
				-profile sm_5_0
				-entry ui_2d_pixel
				-stage pixel
				-O3 -matrix-layout-row-major
				-o "${GRAPHICS_UI_2D_FRAGMENT_SHADER}"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ui_2d.slang"
            COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0
                -entry terrain_vertex -stage vertex -O3 -matrix-layout-row-major
                -o "${GRAPHICS_TERRAIN_VERTEX_SHADER}" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/terrain.slang"
            COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0
                -entry terrain_pixel -stage pixel -O3 -matrix-layout-row-major
                -o "${GRAPHICS_TERRAIN_FRAGMENT_SHADER}" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/terrain.slang"

            COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0
                -entry surface_vertex -stage vertex -O3 -matrix-layout-row-major
                -o "${GRAPHICS_SURFACE_VERTEX_SHADER}" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/surface.slang"
            COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0
                -entry surface_pixel -stage pixel -O3 -matrix-layout-row-major
                -o "${GRAPHICS_SURFACE_FRAGMENT_SHADER}" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/surface.slang"
            COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0
                -entry screen_filter_vertex -stage vertex -O3 -matrix-layout-row-major
                -o "${GRAPHICS_SCREEN_FILTER_VERTEX_SHADER}" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/screen_filter.slang"
            COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0
                -entry screen_filter_pixel -stage pixel -O3 -matrix-layout-row-major
                -o "${GRAPHICS_SCREEN_FILTER_FRAGMENT_SHADER}" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/screen_filter.slang"
            DEPENDS
                "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh"
                "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/environment_lighting.slangh"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/basic_opaque.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/particle_billboard.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/screen_distortion.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/beam.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/laser.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/video.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ring.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/fullscreen_overlay.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/world_quad.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ui_2d.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/terrain.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/surface.slang"
				"${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/screen_filter.slang"
				slang-bootstrap
			VERBATIM
		)
		add_custom_target(generals_graphics_shaders
			DEPENDS
				"${GRAPHICS_BASIC_OPAQUE_VERTEX_SHADER}"
				"${GRAPHICS_SKINNED_BASIC_OPAQUE_VERTEX_SHADER}"
				"${GRAPHICS_BASIC_OPAQUE_FRAGMENT_SHADER}"
				"${GRAPHICS_PARTICLE_VERTEX_SHADER}"
				"${GRAPHICS_PARTICLE_FRAGMENT_SHADER}"
				"${GRAPHICS_SCREEN_DISTORTION_VERTEX_SHADER}"
				"${GRAPHICS_SCREEN_DISTORTION_FRAGMENT_SHADER}"
				"${GRAPHICS_BEAM_VERTEX_SHADER}"
				"${GRAPHICS_BEAM_FRAGMENT_SHADER}"
				"${GRAPHICS_LASER_CORE_VERTEX_SHADER}"
				"${GRAPHICS_LASER_CORE_FRAGMENT_SHADER}"
				"${GRAPHICS_LASER_DISTORTION_VERTEX_SHADER}"
				"${GRAPHICS_LASER_DISTORTION_FRAGMENT_SHADER}"
				"${GRAPHICS_VIDEO_VERTEX_SHADER}"
				"${GRAPHICS_VIDEO_FRAGMENT_SHADER}"
				"${GRAPHICS_RING_VERTEX_SHADER}"
				"${GRAPHICS_RING_FRAGMENT_SHADER}"
				"${GRAPHICS_FULLSCREEN_OVERLAY_VERTEX_SHADER}"
				"${GRAPHICS_FULLSCREEN_OVERLAY_FRAGMENT_SHADER}"
				"${GRAPHICS_WORLD_QUAD_VERTEX_SHADER}"
				"${GRAPHICS_WORLD_QUAD_FRAGMENT_SHADER}"
				"${GRAPHICS_TERRAIN_VERTEX_SHADER}"
				"${GRAPHICS_SURFACE_VERTEX_SHADER}"
				"${GRAPHICS_SCREEN_FILTER_VERTEX_SHADER}"
				"${GRAPHICS_SURFACE_FRAGMENT_SHADER}"
				"${GRAPHICS_SCREEN_FILTER_FRAGMENT_SHADER}"
				"${GRAPHICS_TERRAIN_FRAGMENT_SHADER}"
				"${GRAPHICS_UI_2D_VERTEX_SHADER}"
				"${GRAPHICS_UI_2D_FRAGMENT_SHADER}"
		)
		add_dependencies(generals_graphics generals_graphics_shaders)
	endif()

    target_sources(generals_graphics
        PUBLIC
            FILE_SET CXX_MODULES
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
            FILES
                backends/dx11/DX11RHI.cppm
                backends/dx12/DX12RHI.cppm
                frame/FrameRuntime.cppm
                frame/RenderServices.cppm
    )

    target_sources(generals_graphics PRIVATE backends/dx11/DX11GPUProfiling.cpp)
    target_link_libraries(generals_graphics PRIVATE d3d11 d3d12 dxgi d3dcompiler)
    set_property(TARGET generals_graphics PROPERTY GRAPHICS_SHADER_ASSET_DIRECTORY "${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    include("${CMAKE_CURRENT_SOURCE_DIR}/shaders/DX12Shaders.cmake")
endif()

option(RTS_BUILD_GRAPHICS_TESTS "Build generic graphics unit tests." OFF)

if(RTS_BUILD_GRAPHICS_TESTS)
    include(CTest)
    find_package(Boost CONFIG REQUIRED)

    if(WIN32)
        add_library(generals_graphics_test_support STATIC)
        target_sources(generals_graphics_test_support
            PUBLIC
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    tests/GraphicsTestDevice.cppm
        )
        target_compile_features(generals_graphics_test_support PUBLIC cxx_std_20)
        target_link_libraries(generals_graphics_test_support PUBLIC generals_graphics)
        include("${CMAKE_CURRENT_SOURCE_DIR}/shaders/TestShaders.cmake")
    endif()

    function(add_graphics_test test_name test_source)
        add_executable(${test_name})

        target_sources(${test_name}
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    ${test_source}
        )

        target_compile_features(${test_name} PRIVATE cxx_std_20)
        target_include_directories(${test_name} PRIVATE ${Boost_INCLUDE_DIRS})
        target_link_libraries(${test_name} PRIVATE generals_graphics)
        if(TARGET generals_graphics_test_support)
            target_link_libraries(${test_name} PRIVATE generals_graphics_test_support)
        endif()

        set_property(GLOBAL APPEND PROPERTY GRAPHICS_TEST_TARGETS ${test_name})
        if(WIN32)
            foreach(graphics_test_backend IN ITEMS dx11 dx12)
                foreach(graphics_test_driver IN ITEMS hardware warp)
                    set(graphics_test_instance
                        "${test_name}.${graphics_test_backend}.${graphics_test_driver}")
                    add_test(NAME ${graphics_test_instance} COMMAND ${test_name})
                    set_tests_properties(${graphics_test_instance} PROPERTIES
                        LABELS "graphics;${graphics_test_backend};${graphics_test_driver}"
                        ENVIRONMENT
                            "GRAPHICS_BACKEND=${graphics_test_backend};GRAPHICS_TEST_BACKEND=${graphics_test_backend};GRAPHICS_TEST_DRIVER=${graphics_test_driver};GRAPHICS_TEST_FAILURE_SUFFIX=${graphics_test_backend}-${graphics_test_driver}")
                endforeach()
            endforeach()
        else()
            add_test(NAME ${test_name} COMMAND ${test_name})
        endif()
    endfunction()

    add_graphics_test(generals_graphics_render_clock_test frame/RenderClock.test.cppm)
    add_graphics_test(generals_graphics_render_settings_test frame/RenderSettings.test.cppm)
    if(WIN32)
        add_graphics_test(generals_graphics_render_services_test frame/RenderServices.test.cppm)
        target_compile_definitions(generals_graphics_render_services_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_render_services_test generals_graphics_shaders)
        endif()
    endif()

    add_graphics_test(generals_graphics_scene_draw_context_test scene/DrawContext.test.cppm)
    add_graphics_test(generals_graphics_scene_pass_test scene/ScenePass.test.cppm)
    add_graphics_test(generals_graphics_scene_traversal_test scene/Traversal.test.cppm)
    add_graphics_test(generals_graphics_scene_adapter_test scene/SceneAdapter.test.cppm)
    add_graphics_test(generals_graphics_procedural_pass_test materials/ProceduralPass.test.cppm)
    add_graphics_test(generals_graphics_texture_projector_test materials/TextureProjector.test.cppm)
    if(TARGET corei_always_no_pch AND TARGET core_wwmath)
        target_link_libraries(generals_graphics_texture_projector_test PRIVATE
            corei_always_no_pch corei_libraries_source_wwvegas core_wwmath core_wwlib)
        if(WIN32)
            target_link_libraries(generals_graphics_texture_projector_test PRIVATE winmm)
        endif()
        target_compile_definitions(generals_graphics_texture_projector_test PRIVATE
            GRAPHICS_COMPARE_GAME_MATH=1)
    endif()
    if(WIN32)
        add_graphics_test(generals_graphics_scene_traversal_drawing_test scene/TraversalDrawing.test.cppm)
        target_compile_definitions(generals_graphics_scene_traversal_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_scene_traversal_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_texture_mapping_test materials/TextureMapping.test.cppm)
        target_compile_definitions(generals_graphics_texture_mapping_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        add_graphics_test(generals_graphics_material_state_test materials/MaterialState.test.cppm)
        target_compile_definitions(generals_graphics_material_state_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    endif()
    add_graphics_test(generals_graphics_cursor_test cursors/Cursor.test.cppm)
    add_graphics_test(generals_graphics_display_modes_test presentation/DisplayModes.test.cppm)
    target_link_libraries(generals_graphics_display_modes_test PRIVATE SDL3::SDL3)
    target_link_libraries(generals_graphics_cursor_test PRIVATE SDL3::SDL3)
    add_graphics_test(generals_graphics_cursor_load_test cursors/CursorLoad.test.cppm)
    target_link_libraries(generals_graphics_cursor_load_test PRIVATE SDL3::SDL3)
    if(WIN32)
        foreach(cursor_test generals_graphics_cursor_test generals_graphics_cursor_load_test generals_graphics_display_modes_test)
            add_custom_command(TARGET ${cursor_test} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:${cursor_test}>)
        endforeach()
    endif()
    add_graphics_test(generals_graphics_terrain_visibility_test scene/terrain/TerrainVisibility.test.cppm)
    add_graphics_test(generals_graphics_terrain_geometry_test scene/terrain/TerrainGeometry.test.cppm)
    if(WIN32)
        add_graphics_test(generals_graphics_road_drawing_test scene/roads/RoadDrawing.test.cppm)
        add_graphics_test(generals_graphics_movie_capture_test capture/MovieCapture.test.cppm)
        add_graphics_test(generals_graphics_scene_renderers_test frame/SceneRenderers.test.cppm)
        target_compile_definitions(generals_graphics_scene_renderers_test PRIVATE
            GRAPHICS_SCENE_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_scene_renderers_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_scorch_drawing_test scene/scorches/ScorchDrawing.test.cppm)
        target_compile_definitions(generals_graphics_scorch_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_scorch_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_stencil_volume_drawing_test scene/shadows/StencilVolumeDrawing.test.cppm)
        add_graphics_test(generals_graphics_projected_shadow_drawing_test scene/shadows/ProjectedShadowDrawing.test.cppm)
        add_graphics_test(generals_graphics_projected_shadow_capture_test scene/shadows/ProjectedShadowCapture.test.cppm)
        target_compile_definitions(generals_graphics_projected_shadow_capture_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_projected_shadow_capture_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_directional_shadow_drawing_test scene/shadows/DirectionalShadowDrawing.test.cppm)
        target_compile_definitions(generals_graphics_directional_shadow_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_directional_shadow_drawing_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_projected_shadow_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_projected_shadow_drawing_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_stencil_volume_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_stencil_volume_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_bib_drawing_test scene/bibs/BibDrawing.test.cppm)
        target_compile_definitions(generals_graphics_bib_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_bib_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_line_drawing_test scene/lines/LineDrawing.test.cppm)
        target_compile_definitions(generals_graphics_line_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_line_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_tracer_drawing_test scene/lines/Tracer.test.cppm)
        target_compile_definitions(generals_graphics_tracer_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_tracer_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_screen_filter_drawing_test scene/screen/ScreenFilterDrawing.test.cppm)
        target_compile_definitions(generals_graphics_screen_filter_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_screen_filter_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_debug_drawing_test scene/debug/DebugDrawing.test.cppm)
        target_compile_definitions(generals_graphics_debug_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_debug_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_render_object_test scene/RenderObject.test.cppm)
        target_compile_definitions(generals_graphics_render_object_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_render_object_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_collision_box_test scene/debug/CollisionBox.test.cppm)
        target_compile_definitions(generals_graphics_collision_box_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_collision_box_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_track_drawing_test scene/tracks/TrackDrawing.test.cppm)
        target_compile_definitions(generals_graphics_track_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_track_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_tree_drawing_test scene/trees/TreeDrawing.test.cppm)
        target_compile_definitions(generals_graphics_tree_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_tree_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_texture_snapshot_test resources/textures/TextureSnapshot.test.cppm)
        add_graphics_test(generals_graphics_texture_upload_test resources/textures/TextureUpload.test.cppm)
        add_graphics_test(generals_graphics_texture_load_test resources/textures/TextureLoad.test.cppm)
        add_graphics_test(generals_graphics_texture_atlas_test resources/textures/TextureAtlas.test.cppm)
        target_compile_definitions(generals_graphics_texture_atlas_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_texture_atlas_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_frame_resource_lifecycle_test frame/FrameResourceLifecycle.test.cppm)
        add_graphics_test(generals_graphics_frame_drawing_test frame/FrameDrawing.test.cppm)
        add_graphics_test(generals_graphics_frame_device_test frame/FrameDevice.test.cppm)
        add_graphics_test(generals_graphics_tool_frame_test frame/ToolFrame.test.cppm)
        target_compile_definitions(generals_graphics_tool_frame_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_tool_frame_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_frame_device_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_frame_device_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_frame_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_frame_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_scene_draw_parameters_test scene/SceneDrawParameters.test.cppm)
        add_graphics_test(generals_graphics_camera_matrices_test scene/views/CameraMatrices.test.cppm)
        add_graphics_test(generals_graphics_camera_state_test scene/views/CameraState.test.cppm)
        target_compile_definitions(generals_graphics_camera_matrices_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_camera_matrices_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_camera_state_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_camera_state_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_scene_draw_parameters_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_scene_draw_parameters_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_attachment_bindings_test frame/AttachmentBindings.test.cppm)
        target_compile_definitions(generals_graphics_attachment_bindings_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_attachment_bindings_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_resource_recreation_test resources/ResourceRecreation.test.cppm)
        add_graphics_test(generals_graphics_scene_object_list_test scene/SceneObjectList.test.cppm)
        add_graphics_test(generals_graphics_ordered_draws_test scene/OrderedDraws.test.cppm)
        target_compile_definitions(generals_graphics_ordered_draws_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_ordered_draws_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_scene_object_list_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_scene_object_list_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_texture_load_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_texture_load_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_texture_resource_test resources/textures/TextureResource.test.cppm)
        add_graphics_test(generals_graphics_texture_edit_test resources/textures/TextureEdit.test.cppm)
        target_compile_definitions(generals_graphics_texture_edit_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_texture_edit_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_texture_resource_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_texture_resource_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_resource_load_queue_test resources/loading/ResourceLoadQueue.test.cppm)
        target_compile_definitions(generals_graphics_resource_load_queue_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_resource_load_queue_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_texture_upload_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_texture_upload_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_texture_references_test resources/textures/TextureReferences.test.cppm)
        add_graphics_test(generals_graphics_texture_sampling_test resources/textures/TextureSampling.test.cppm)
        add_graphics_test(generals_graphics_texture_storage_test resources/textures/TextureStorage.test.cppm)
        add_graphics_test(generals_graphics_texture_residency_test resources/textures/TextureResidency.test.cppm)
        target_compile_definitions(generals_graphics_texture_residency_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_texture_residency_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_material_fog_test materials/Fog.test.cppm)
        add_graphics_test(generals_graphics_local_lighting_test scene/lighting/LocalLighting.test.cppm)
        target_compile_definitions(generals_graphics_local_lighting_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_local_lighting_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_light_state_test scene/lighting/LightState.test.cppm)
        target_compile_definitions(generals_graphics_light_state_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_light_state_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_material_fog_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_material_fog_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_texture_sampling_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_texture_sampling_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_texture_references_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_texture_references_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_texture_snapshot_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_texture_snapshot_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_renderer2d_drawing_test renderer2d/Renderer2DDrawing.test.cppm)
        target_compile_definitions(generals_graphics_renderer2d_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_renderer2d_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_bindless_drawing_test rhi/BindlessDrawing.test.cppm)
        target_compile_definitions(generals_graphics_bindless_drawing_test PRIVATE
            GRAPHICS_BINDLESS_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_bindless_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_frame_preview_test capture/FramePreview.test.cppm)
        add_graphics_test(generals_graphics_bloom_test passes/post-process/Bloom.test.cppm)
        add_graphics_test(generals_graphics_ssao_test passes/post-process/SSAO.test.cppm)
        target_compile_definitions(generals_graphics_ssao_test PRIVATE
            GRAPHICS_SSAO_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_ssao_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_bloom_test PRIVATE
            GRAPHICS_BLOOM_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        add_graphics_test(generals_graphics_light_rays_test passes/post-process/LightRays.test.cppm)
        target_compile_definitions(generals_graphics_light_rays_test PRIVATE
            GRAPHICS_LIGHT_RAYS_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        target_compile_definitions(generals_graphics_frame_preview_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_frame_preview_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_prop_drawing_test scene/props/PropDrawing.test.cppm)
        add_graphics_test(generals_graphics_model_materials_test scene/models/ModelMaterials.test.cppm)
        add_graphics_test(generals_graphics_mesh_material_bindings_test scene/models/MeshMaterialBindings.test.cppm)
        target_compile_definitions(generals_graphics_model_materials_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_model_materials_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_muzzle_flash_test scene/muzzle_flash/MuzzleFlash.test.cppm)
        target_compile_definitions(generals_graphics_muzzle_flash_test PRIVATE
            GRAPHICS_MUZZLE_FLASH_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_muzzle_flash_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_prop_surface_test scene/props/PropSurface.test.cppm)
        add_graphics_test(generals_graphics_prop_asset_geometry_test scene/props/PropAssetGeometry.test.cppm)
        add_graphics_test(generals_graphics_prop_skinning_test scene/props/PropSkinning.test.cppm)
        add_graphics_test(generals_graphics_prop_asset_binding_test scene/props/PropAssetBinding.test.cppm)
        target_compile_definitions(generals_graphics_prop_asset_binding_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        target_compile_definitions(generals_graphics_prop_surface_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_prop_surface_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_prop_submission_test scene/props/PropSubmission.test.cppm)
        target_compile_definitions(generals_graphics_prop_submission_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_prop_submission_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_shroud_drawing_test scene/shroud/ShroudDrawing.test.cppm)
        target_compile_definitions(generals_graphics_shroud_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_shroud_drawing_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_prop_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_prop_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_water_drawing_test scene/water/WaterDrawing.test.cppm)
        target_compile_definitions(generals_graphics_water_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}"
            GRAPHICS_WATER_TEXTURE_DIRECTORY="${PROJECT_SOURCE_DIR}/GeneralsMD/Run/Art/Textures")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_water_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_bridge_drawing_test scene/bridges/BridgeDrawing.test.cppm)
        target_compile_definitions(generals_graphics_bridge_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_bridge_drawing_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_road_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_road_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_terrain_renderer_test scene/terrain/TerrainRenderer.test.cppm)
        target_compile_definitions(generals_graphics_terrain_renderer_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_terrain_renderer_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_transparent_drawing_test scene/props/TransparentDrawing.test.cppm)
        target_compile_definitions(generals_graphics_transparent_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_transparent_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_material_pass_drawing_test scene/props/MaterialPassDrawing.test.cppm)
        add_graphics_test(generals_graphics_primitive_geometry_test scene/primitives/PrimitiveGeometry.test.cppm)
        target_compile_definitions(generals_graphics_primitive_geometry_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_primitive_geometry_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_screen_distortion_drawing_test scene/screen/ScreenDistortionDrawing.test.cppm)
        target_compile_definitions(generals_graphics_screen_distortion_drawing_test PRIVATE
            GRAPHICS_SCREEN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_screen_distortion_drawing_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_material_pass_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_material_pass_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_model_texture_drawing_test scene/models/ModelTextureDrawing.test.cppm)
        target_compile_definitions(generals_graphics_model_texture_drawing_test PRIVATE
            GRAPHICS_MODEL_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_model_texture_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_particle_texture_drawing_test scene/particles/ParticleTextureDrawing.test.cppm)
        add_graphics_test(generals_graphics_sprite_geometry_drawing_test scene/particles/SpriteGeometryDrawing.test.cppm)
        add_graphics_test(generals_graphics_line_group_drawing_test scene/particles/LineGroupDrawing.test.cppm)
        target_compile_definitions(generals_graphics_line_group_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_line_group_drawing_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_sprite_geometry_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_sprite_geometry_drawing_test generals_graphics_shaders)
        endif()
        target_compile_definitions(generals_graphics_particle_texture_drawing_test PRIVATE
            GRAPHICS_PARTICLE_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_particle_texture_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_point_light_drawing_test scene/props/PointLightDrawing.test.cppm)
        target_compile_definitions(generals_graphics_point_light_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_point_light_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_line_material_drawing_test scene/lines/LineMaterialDrawing.test.cppm)
        target_compile_definitions(generals_graphics_line_material_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_line_material_drawing_test generals_graphics_shaders)
        endif()
        add_graphics_test(generals_graphics_glyph_atlas_drawing_test text/GlyphAtlasDrawing.test.cppm)
        target_compile_definitions(generals_graphics_glyph_atlas_drawing_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_glyph_atlas_drawing_test generals_graphics_shaders)
        endif()
    endif()



    add_graphics_test(
        generals_graphics_resource_handle_test
        resources/handles/ResourceHandle.test.cppm
    )

    add_graphics_test(
        generals_graphics_aligned_allocator_test
        memory/AlignedAllocator.test.cppm
    )

    add_graphics_test(
        generals_graphics_resource_pool_test
        resources/pools/ResourcePool.test.cppm
    )

    add_graphics_test(
        generals_graphics_mesh_test
        resources/meshes/Mesh.test.cppm
    )

    add_graphics_test(
        generals_graphics_texture_test
        resources/textures/Texture.test.cppm
    )

    add_graphics_test(
        generals_graphics_sampler_test
        resources/samplers/Sampler.test.cppm
    )

    add_graphics_test(
        generals_graphics_material_test
        resources/materials/Material.test.cppm
    )

    add_graphics_test(
        generals_graphics_gpu_resource_residency_test
        resources/residency/GPUResourceResidency.test.cppm
    )

    add_graphics_test(
        generals_graphics_bindless_resource_table_test
        resources/bindless/bindlessResourceTable.test.cppm
    )

    add_graphics_test(
        generals_graphics_parameter_layout_test
        shaders/ParameterLayout.test.cppm
    )

    add_graphics_test(
        generals_graphics_pipeline_test
        shaders/Pipeline.test.cppm
    )

    add_graphics_test(
        generals_graphics_shader_library_test
        shaders/ShaderLibrary.test.cppm
    )

    add_graphics_test(
        generals_graphics_lighting_test
        scene/lighting/Lighting.test.cppm
    )

    add_graphics_test(
        generals_graphics_light_renderer_test
        scene/lighting/LightRenderer.test.cppm
    )

    add_graphics_test(
        generals_graphics_environment_test
        scene/environment/Environment.test.cppm
    )

    add_graphics_test(
        generals_graphics_shadows_test
        scene/shadows/Shadows.test.cppm
    )

    add_graphics_test(
        generals_graphics_decals_test
        scene/decals/Decals.test.cppm
    )

    add_graphics_test(
        generals_graphics_decal_visibility_test
        scene/decals/DecalVisibility.test.cppm
    )

    add_graphics_test(
        generals_graphics_decal_draw_test
        scene/decals/DecalDraw.test.cppm
    )

    add_graphics_test(
        generals_graphics_transparency_test
        scene/transparency/Transparency.test.cppm
    )

    add_graphics_test(
        generals_graphics_render_scene_test
        scene/RenderScene.test.cppm
    )

    add_graphics_test(
        generals_graphics_attachment_graph_test
        scene/attachments/Attachments.test.cppm
    )
    if(WIN32)
        target_sources(generals_graphics_attachment_graph_test
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    tests/visual_regression/VisualRegression.cppm
        )
        target_compile_definitions(generals_graphics_attachment_graph_test PRIVATE
            GRAPHICS_ATTACHMENT_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/scene/attachments/__snapshots__"
            GRAPHICS_ATTACHMENT_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/attachment_graph_failures"
            GRAPHICS_ATTACHMENT_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}"
        )
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_attachment_graph_test generals_graphics_shaders)
        endif()
    endif()

    add_graphics_test(
        generals_graphics_model_visibility_test
        scene/models/ModelVisibility.test.cppm
    )

    add_graphics_test(
        generals_graphics_skeleton_test
        scene/models/Skeleton.test.cppm
    )

    add_graphics_test(
        generals_graphics_model_instance_test
        scene/models/ModelInstance.test.cppm
    )

    add_graphics_test(
        generals_graphics_animation_test
        scene/models/Animation.test.cppm
    )

    add_graphics_test(
        generals_graphics_animation_blend_test
        scene/models/AnimationBlend.test.cppm
    )
    add_graphics_test(generals_graphics_animation_channels_test scene/models/AnimationChannels.test.cppm)
    add_graphics_test(generals_graphics_clip_sampling_test scene/models/ClipSampling.test.cppm)
    target_compile_definitions(generals_graphics_clip_sampling_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_clip_sampling_test generals_graphics_shaders)
    endif()
    add_graphics_test(generals_graphics_affine_transform_test scene/AffineTransform.test.cppm)
    add_graphics_test(generals_graphics_animation_rotation_test scene/models/AnimationRotation.test.cppm)
    target_compile_definitions(generals_graphics_animation_rotation_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_animation_rotation_test generals_graphics_shaders)
    endif()
    if(TARGET corei_always_no_pch)
        target_sources(generals_graphics_animation_rotation_test PRIVATE scene/models/AnimationRotation.test-reference.cpp)
        target_link_libraries(generals_graphics_animation_rotation_test PRIVATE corei_always_no_pch corei_libraries_source_wwvegas)
        target_compile_definitions(generals_graphics_animation_rotation_test PRIVATE GRAPHICS_COMPARE_GAME_MATH=1)
        target_link_libraries(generals_graphics_affine_transform_test PRIVATE corei_always_no_pch corei_libraries_source_wwvegas)
        target_compile_definitions(generals_graphics_affine_transform_test PRIVATE GRAPHICS_COMPARE_GAME_MATH=1)
    endif()
    add_graphics_test(generals_graphics_model_hierarchy_test scene/models/ModelHierarchy.test.cppm)
    add_graphics_test(generals_graphics_model_playback_test scene/models/ModelPlayback.test.cppm)
    target_compile_definitions(generals_graphics_model_playback_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_model_playback_test generals_graphics_shaders)
    endif()
    add_graphics_test(generals_graphics_model_children_test scene/models/ModelChildren.test.cppm)
    target_compile_definitions(generals_graphics_model_children_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_model_children_test generals_graphics_shaders)
    endif()
    add_graphics_test(generals_graphics_model_assembly_drawing_test scene/models/ModelAssemblyDrawing.test.cppm)
    add_graphics_test(generals_graphics_material_slots_test scene/models/MaterialSlots.test.cppm)
    add_graphics_test(generals_graphics_model_geometry_source_test scene/models/ModelGeometrySource.test.cppm)
    add_graphics_test(generals_graphics_dazzle_state_test scene/dazzles/DazzleState.test.cppm)
    add_graphics_test(generals_graphics_dazzle_layer_test scene/dazzles/DazzleLayer.test.cppm)
    add_graphics_test(generals_graphics_dazzle_resources_test scene/dazzles/DazzleResources.test.cppm)
    add_graphics_test(generals_graphics_dazzle_drawing_test scene/dazzles/DazzleDrawing.test.cppm)
    target_compile_definitions(generals_graphics_dazzle_drawing_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_dazzle_drawing_test generals_graphics_shaders)
    endif()
    add_graphics_test(generals_graphics_mesh_texture_mapping_test materials/MeshTextureMapping.test.cppm)
    add_graphics_test(generals_graphics_model_object_drawing_test scene/models/ModelObjectDrawing.test.cppm)
    target_compile_definitions(generals_graphics_model_object_drawing_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_model_object_drawing_test generals_graphics_shaders)
    endif()
    add_graphics_test(generals_graphics_model_mesh_drawing_test scene/models/ModelMeshDrawing.test.cppm)
    target_compile_definitions(generals_graphics_model_mesh_drawing_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_model_mesh_drawing_test generals_graphics_shaders)
    endif()
    add_graphics_test(generals_graphics_w3d_mesh_material_loading_test scene/models/W3DMeshMaterialLoading.test.cppm)
    add_graphics_test(generals_graphics_submission_statistics_test frame/SubmissionStatistics.test.cppm)
    add_graphics_test(generals_graphics_sprite_geometry_test scene/particles/SpriteGeometry.test.cppm)
    add_graphics_test(generals_graphics_emitter_test scene/particles/Emitter.test.cppm)
    target_compile_definitions(generals_graphics_emitter_test PRIVATE
        GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_emitter_test generals_graphics_shaders)
    endif()
    add_graphics_test(generals_graphics_line_group_geometry_test scene/particles/LineGroupGeometry.test.cppm)
    add_graphics_test(generals_graphics_ribbon_subdivision_test scene/beams/RibbonSubdivision.test.cppm)
    add_graphics_test(generals_graphics_ribbon_edges_test scene/beams/RibbonEdges.test.cppm)
    add_graphics_test(generals_graphics_ribbon_intersections_test scene/beams/RibbonIntersections.test.cppm)
    add_graphics_test(generals_graphics_ribbon_geometry_test scene/beams/RibbonGeometry.test.cppm)
    add_graphics_test(generals_graphics_ribbon_texture_coordinates_test scene/beams/RibbonTextureCoordinates.test.cppm)
    target_compile_definitions(generals_graphics_ribbon_texture_coordinates_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_ribbon_texture_coordinates_test generals_graphics_shaders)
    endif()
    add_graphics_test(generals_graphics_ribbon_pipeline_test scene/beams/RibbonPipeline.test.cppm)
    target_compile_definitions(generals_graphics_ribbon_pipeline_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_ribbon_pipeline_test generals_graphics_shaders)
    endif()
    add_graphics_test(generals_graphics_segmented_line_test scene/beams/SegmentedLine.test.cppm)
    target_compile_definitions(generals_graphics_segmented_line_test PRIVATE
        GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_segmented_line_test generals_graphics_shaders)
    endif()
    if(TARGET corei_always_no_pch)
        add_graphics_test(generals_graphics_vertex_channels_test scene/models/VertexChannels.test.cppm)
        target_link_libraries(generals_graphics_vertex_channels_test PRIVATE corei_always_no_pch corei_libraries_source_wwvegas)
    endif()
    add_graphics_test(generals_graphics_model_factory_store_test scene/models/ModelFactoryStore.test.cppm)
    add_graphics_test(generals_graphics_model_factory_test scene/models/ModelFactory.test.cppm)
    target_compile_definitions(generals_graphics_model_assembly_drawing_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_model_assembly_drawing_test generals_graphics_shaders)
    endif()
    target_compile_definitions(generals_graphics_model_hierarchy_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_model_hierarchy_test generals_graphics_shaders)
    endif()
    target_compile_definitions(generals_graphics_animation_channels_test PRIVATE GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
    if(TARGET generals_graphics_shaders)
        add_dependencies(generals_graphics_animation_channels_test generals_graphics_shaders)
    endif()
    add_graphics_test(
        generals_graphics_model_asset_pose_test
        scene/models/ModelAssetPose.test.cppm
    )

    add_graphics_test(
        generals_graphics_skinning_test
        scene/models/Skinning.test.cppm
    )

    add_graphics_test(
        generals_graphics_static_mesh_renderer_test
        scene/meshes/StaticMeshRenderer.test.cppm
    )

    add_graphics_test(
        generals_graphics_model_asset_binding_test
        scene/models/ModelAssetBinding.test.cppm
    )

    add_graphics_test(
        generals_graphics_model_asset_geometry_test
        scene/models/ModelAssetGeometry.test.cppm
    )
    if(WIN32)
        target_sources(generals_graphics_static_mesh_renderer_test
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    tests/visual_regression/VisualRegression.cppm
        )
        target_compile_definitions(generals_graphics_static_mesh_renderer_test PRIVATE
            GRAPHICS_STATIC_MESH_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/scene/meshes/__snapshots__"
            GRAPHICS_STATIC_MESH_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/static_mesh_renderer_failures"
            GRAPHICS_STATIC_MESH_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}"
        )
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_static_mesh_renderer_test generals_graphics_shaders)
        endif()
    endif()

    add_graphics_test(
        generals_graphics_view_test
        scene/views/View.test.cppm
    )

    add_graphics_test(
        generals_graphics_visibility_test
        scene/visibility/Visibility.test.cppm
    )

    add_graphics_test(
        generals_graphics_lod_test
        scene/lod/LOD.test.cppm
    )

    add_graphics_test(
        generals_graphics_gpu_scene_test
        scene/gpu/GPUScene.test.cppm
    )

    add_graphics_test(
        generals_graphics_draw_generation_test
        scene/draw/DrawGeneration.test.cppm
    )

    add_graphics_test(
        generals_graphics_render_graph_test
        render-graph/RenderGraph.test.cppm
    )

    add_graphics_test(
        generals_graphics_render_graph_execution_test
        render-graph/RenderGraphExecution.test.cppm
    )

    add_graphics_test(
        generals_graphics_renderer2d_test
        renderer2d/Renderer2D.test.cppm
    )

    add_graphics_test(
        generals_graphics_opaque_pass_test
        passes/opaque/OpaquePass.test.cppm
    )
    if(WIN32)
        target_compile_definitions(generals_graphics_opaque_pass_test PRIVATE
            GRAPHICS_TEST_SHADER_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/tests/visual_regression/shaders/precompiled/dx11"
        )
    endif()

    add_graphics_test(
        generals_graphics_shadow_pass_test
        passes/shadow/ShadowPass.test.cppm
    )

    add_graphics_test(
        generals_graphics_transparent_pass_test
        passes/transparent/TransparentPass.test.cppm
    )

    add_graphics_test(
        generals_graphics_decal_pass_test
        passes/decal/DecalPass.test.cppm
    )

    add_graphics_test(
        generals_graphics_particles_test
        scene/particles/Particles.test.cppm
    )

    add_graphics_test(
        generals_graphics_particle_visibility_test
        scene/particles/ParticleVisibility.test.cppm
    )

    add_graphics_test(
        generals_graphics_particle_draw_test
        scene/particles/ParticleDraw.test.cppm
    )

    add_graphics_test(
        generals_graphics_particle_pass_test
        passes/particles/ParticlePass.test.cppm
    )

    add_graphics_test(
        generals_graphics_weather_particles_test
        scene/weather/WeatherParticles.test.cppm
    )

    add_graphics_test(
        generals_graphics_world_quads_test
        scene/world/WorldQuads.test.cppm
    )

    if(WIN32)
        add_graphics_test(
            generals_graphics_particle_renderer_test
            scene/particles/ParticleRenderer.test.cppm
        )
        target_sources(generals_graphics_particle_renderer_test
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    tests/visual_regression/VisualRegression.cppm
        )
        target_compile_definitions(generals_graphics_particle_renderer_test PRIVATE
            GRAPHICS_PARTICLE_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/scene/particles/__snapshots__"
            GRAPHICS_PARTICLE_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/particle_renderer_failures"
            GRAPHICS_PARTICLE_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}"
        )
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_particle_renderer_test generals_graphics_shaders)
        endif()
    endif()


    add_graphics_test(
        generals_graphics_beam_test
        scene/beams/beam.test.cppm
    )

    add_graphics_test(
        generals_graphics_laser_test
        scene/beams/Laser.test.cppm
    )

    if(WIN32)
        add_graphics_test(
            generals_graphics_laser_drawing_test
            scene/beams/LaserDrawing.test.cppm
        )
        target_compile_definitions(generals_graphics_laser_drawing_test PRIVATE
            GRAPHICS_LASER_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_laser_drawing_test generals_graphics_shaders)
        endif()
    endif()

    add_graphics_test(
        generals_graphics_ring_test
        scene/ring/Ring.test.cppm
    )

    add_graphics_test(
        generals_graphics_ring_runtime_test
        scene/ring/RingRuntime.test.cppm
    )

    add_graphics_test(
        generals_graphics_ring_renderer_test
        scene/ring/AuthoredRingRenderer.test.cppm
    )

    if(WIN32)
        target_compile_definitions(generals_graphics_ring_renderer_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_ring_renderer_test generals_graphics_shaders)
        endif()
    endif()

    if(WIN32)
        add_graphics_test(
            generals_graphics_sphere_test
            scene/sphere/Sphere.test.cppm
        )
        target_compile_definitions(generals_graphics_sphere_test PRIVATE
            GRAPHICS_TERRAIN_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}")
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_sphere_test generals_graphics_shaders)
        endif()
    endif()

    add_graphics_test(
        generals_graphics_fullscreen_overlay_test
        scene/screen/FullscreenOverlay.test.cppm
    )

    add_graphics_test(
        generals_graphics_post_process_test
        passes/post-process/PostProcess.test.cppm
    )

    add_graphics_test(
        generals_graphics_sky_pass_test
        passes/sky/SkyPass.test.cppm
    )

    if(WIN32)
        add_graphics_test(
            generals_graphics_video_renderer_test
            video/VideoRenderer.test.cppm
        )
        target_sources(generals_graphics_video_renderer_test
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    tests/visual_regression/VisualRegression.cppm
        )
        target_compile_definitions(generals_graphics_video_renderer_test PRIVATE
            GRAPHICS_VIDEO_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/video/__snapshots__"
            GRAPHICS_VIDEO_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/video_renderer_failures"
            GRAPHICS_VIDEO_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}"
        )
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_video_renderer_test generals_graphics_shaders)
        endif()
    endif()

    if(WIN32)
        add_graphics_test(
            generals_graphics_w3d_e2e_test
            test_scenarios/w3d/W3DE2E.test.cppm
        )
        target_sources(generals_graphics_w3d_e2e_test
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    tests/visual_regression/VisualRegression.cppm
        )
        file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}/GeneralsMD/Run/Art/W3D" GRAPHICS_W3D_E2E_MODEL_DIRECTORY_CMAKE)
        file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}/GeneralsMD/Run/Art/Textures" GRAPHICS_W3D_E2E_TEXTURE_DIRECTORY_CMAKE)
        target_compile_definitions(generals_graphics_w3d_e2e_test PRIVATE
            GRAPHICS_W3D_E2E_MODEL_DIRECTORY="${GRAPHICS_W3D_E2E_MODEL_DIRECTORY_CMAKE}"
            GRAPHICS_W3D_E2E_TEXTURE_DIRECTORY="${GRAPHICS_W3D_E2E_TEXTURE_DIRECTORY_CMAKE}"
            GRAPHICS_W3D_E2E_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/test_scenarios/w3d/__snapshots__"
            GRAPHICS_W3D_E2E_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/w3d_e2e_failures"
            GRAPHICS_W3D_E2E_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}"
        )

        add_graphics_test(
            generals_graphics_visual_regression_test
            tests/visual_regression/VisualRegression.test.cppm
        )
        target_sources(generals_graphics_visual_regression_test
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    tests/visual_regression/VisualRegression.cppm
        )
        target_compile_definitions(generals_graphics_visual_regression_test PRIVATE
            GRAPHICS_VISUAL_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/tests/visual_regression/references"
            GRAPHICS_UI2D_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/renderer2d/__snapshots__"
            GRAPHICS_VISUAL_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/visual_regression_failures"
            GRAPHICS_VISUAL_SHADER_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/tests/visual_regression/shaders/precompiled/dx11"
            GRAPHICS_RENDERER_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}"
        )
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_visual_regression_test generals_graphics_shaders)
        endif()

        add_graphics_test(
            generals_graphics_weather_particles_snapshot_test
            scene/weather/WeatherParticles.snapshot.test.cppm
        )
        target_sources(generals_graphics_weather_particles_snapshot_test
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    tests/visual_regression/VisualRegression.cppm
        )
        target_compile_definitions(generals_graphics_weather_particles_snapshot_test PRIVATE
            GRAPHICS_WEATHER_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/scene/weather/__snapshots__"
            GRAPHICS_WEATHER_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/weather_particle_snapshot_failures"
            GRAPHICS_WEATHER_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}"
        )
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_weather_particles_snapshot_test generals_graphics_shaders)
        endif()

        add_graphics_test(
            generals_graphics_ring_snapshot_test
            scene/ring/Ring.snapshot.test.cppm
        )
        target_sources(generals_graphics_ring_snapshot_test
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    tests/visual_regression/VisualRegression.cppm
        )
        target_compile_definitions(generals_graphics_ring_snapshot_test PRIVATE
            GRAPHICS_RING_SNAPSHOT_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/scene/ring/__snapshots__"
            GRAPHICS_RING_SNAPSHOT_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/ring_snapshot_failures"
            GRAPHICS_RING_SNAPSHOT_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}"
        )
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_ring_snapshot_test generals_graphics_shaders)
        endif()

        add_graphics_test(
            generals_graphics_world_quads_snapshot_test
            scene/world/WorldQuads.snapshot.test.cppm
        )
        target_sources(generals_graphics_world_quads_snapshot_test
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                FILES
                    tests/visual_regression/VisualRegression.cppm
        )
        target_compile_definitions(generals_graphics_world_quads_snapshot_test PRIVATE
            GRAPHICS_WORLD_QUAD_REFERENCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/scene/world/__snapshots__"
            GRAPHICS_WORLD_QUAD_FAILURE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/world_quad_snapshot_failures"
            GRAPHICS_WORLD_QUAD_SHADER_DIRECTORY="${GRAPHICS_SHADER_ASSET_DIRECTORY}"
        )
        if(TARGET generals_graphics_shaders)
            add_dependencies(generals_graphics_world_quads_snapshot_test generals_graphics_shaders)
        endif()
    endif()

    add_graphics_test(
        generals_graphics_rhi_test
        rhi/RHI.test.cppm
    )

    add_graphics_test(
        generals_graphics_frame_test
        frame/FrameOwner.test.cppm
    )

    if(WIN32)
        add_graphics_test(
            generals_graphics_backend_rhi_test
            backends/BackendRHI.test.cppm
        )
        target_compile_definitions(generals_graphics_backend_rhi_test PRIVATE
            GRAPHICS_TEST_SHADER_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/tests/visual_regression/shaders/precompiled/dx11"
        )
    endif()

    get_property(graphics_test_targets GLOBAL PROPERTY GRAPHICS_TEST_TARGETS)
    if(graphics_test_targets AND NOT TARGET generals_graphics_tests)
        add_custom_target(generals_graphics_tests DEPENDS ${graphics_test_targets})
    endif()
endif()

if(WIN32 AND TARGET slang-bootstrap)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ssao.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ssao.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry ssao_vertex -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ssao.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ssao.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry ssao_pixel -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ssao.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ssao.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ssao.slang" slang-bootstrap VERBATIM)
    add_custom_target(generals_graphics_ssao_shaders DEPENDS "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ssao.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ssao.pso")
    add_dependencies(generals_graphics generals_graphics_ssao_shaders)
    if(TARGET generals_graphics_ssao_test)
        add_dependencies(generals_graphics_ssao_test generals_graphics_ssao_shaders)
    endif()
endif()

if(WIN32 AND TARGET slang-bootstrap)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/light_rays.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/light_rays.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry light_rays_vertex -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/light_rays.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/light_rays.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry light_rays_pixel -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/light_rays.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/light_rays.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/light_rays.slang" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/environment_lighting.slangh" slang-bootstrap VERBATIM)
    add_custom_target(generals_graphics_light_rays_shaders DEPENDS "${GRAPHICS_SHADER_ASSET_DIRECTORY}/light_rays.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/light_rays.pso")
    add_dependencies(generals_graphics generals_graphics_light_rays_shaders)
    if(TARGET generals_graphics_light_rays_test)
        add_dependencies(generals_graphics_light_rays_test generals_graphics_light_rays_shaders)
    endif()
endif()

if(WIN32 AND TARGET slang-bootstrap)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/bloom.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/bloom.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry bloom_vertex -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/bloom.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/bloom.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry bloom_pixel -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/bloom.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/bloom.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/bloom.slang" slang-bootstrap VERBATIM)
    add_custom_target(generals_graphics_bloom_shaders DEPENDS "${GRAPHICS_SHADER_ASSET_DIRECTORY}/bloom.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/bloom.pso")
    add_dependencies(generals_graphics generals_graphics_bloom_shaders)
    if(TARGET generals_graphics_bloom_test)
        add_dependencies(generals_graphics_bloom_test generals_graphics_bloom_shaders)
    endif()
endif()

if(WIN32 AND TARGET slang-bootstrap)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/tree.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/tree.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry tree_vertex -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/tree.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/tree.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry tree_pixel -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/tree.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/tree.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/environment_lighting.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/tree.slang" slang-bootstrap VERBATIM)
    add_custom_target(generals_graphics_tree_shaders DEPENDS "${GRAPHICS_SHADER_ASSET_DIRECTORY}/tree.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/tree.pso")
    add_dependencies(generals_graphics generals_graphics_tree_shaders)
endif()

if(TARGET generals_graphics_tree_drawing_test AND TARGET generals_graphics_tree_shaders)
    add_dependencies(generals_graphics_tree_drawing_test generals_graphics_tree_shaders)
endif()


if(WIN32 AND TARGET slang-bootstrap)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ocean.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ocean.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry OceanVertexMain -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ocean.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ocean.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry OceanPixelMain -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ocean.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ocean.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/environment_lighting.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ocean.slang" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_constants.slangh" slang-bootstrap VERBATIM)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ocean_instanced.vso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry OceanVertexInstanced -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ocean_instanced.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ocean.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/environment_lighting.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/ocean.slang" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_constants.slangh" slang-bootstrap VERBATIM)
    add_custom_target(generals_graphics_ocean_shaders DEPENDS "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ocean_instanced.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ocean.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/ocean.pso")
    add_dependencies(generals_graphics generals_graphics_ocean_shaders)
endif()

if(WIN32 AND TARGET slang-bootstrap)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_underwater.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_underwater_instanced.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_underwater.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry UnderwaterVertexMain -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_underwater.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_underwater.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry UnderwaterVertexInstanced -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_underwater_instanced.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_underwater.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry UnderwaterPixelMain -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_underwater.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_underwater.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_constants.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_underwater.slang" slang-bootstrap VERBATIM)
    add_custom_target(generals_graphics_water_underwater_shaders DEPENDS "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_underwater.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_underwater_instanced.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_underwater.pso")
    add_dependencies(generals_graphics generals_graphics_water_underwater_shaders)
endif()

if(WIN32 AND TARGET slang-bootstrap)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_surface.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_surface.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry WaterSurfaceVertexMain -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_surface.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_surface.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry WaterSurfacePixelMain -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_surface.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_surface.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/environment_lighting.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_surface.slang" slang-bootstrap VERBATIM)
    add_custom_target(generals_graphics_water_surface_shaders DEPENDS "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_surface.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_surface.pso")
    add_dependencies(generals_graphics generals_graphics_water_surface_shaders)
endif()

if(WIN32 AND TARGET slang-bootstrap)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_displacement.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_displacement.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry WaterDisplacementVertexMain -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_displacement.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_displacement.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry WaterDisplacementPixelMain -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_displacement.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_displacement.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_displacement.slang" slang-bootstrap VERBATIM)
    add_custom_target(generals_graphics_water_displacement_shaders DEPENDS "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_displacement.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_displacement.pso")
    add_dependencies(generals_graphics generals_graphics_water_displacement_shaders)
endif()

if(WIN32 AND TARGET slang-bootstrap)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_sky.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_sky.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry WaterSkyVertexMain -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_sky.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_sky.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry WaterSkyPixelMain -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_sky.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_sky.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_sky.slang" slang-bootstrap VERBATIM)
    add_custom_target(generals_graphics_water_sky_shaders DEPENDS "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_sky.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_sky.pso")
    add_dependencies(generals_graphics generals_graphics_water_sky_shaders)
endif()

if(WIN32 AND TARGET slang-bootstrap)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_track.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_track.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry WaterTrackVertexMain -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_track.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_track.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry WaterTrackPixelMain -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_track.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_track.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/water_track.slang" slang-bootstrap VERBATIM)
    add_custom_target(generals_graphics_water_track_shaders DEPENDS "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_track.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/water_track.pso")
    add_dependencies(generals_graphics generals_graphics_water_track_shaders)
endif()

if(WIN32 AND TARGET slang-bootstrap)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry prop_vertex -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/prop.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry prop_pixel -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/prop.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/environment_lighting.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/prop_surface.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/prop.slang" slang-bootstrap VERBATIM)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop_instanced.vso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry prop_vertex_instanced -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop_instanced.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/prop.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/environment_lighting.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/prop_surface.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/prop.slang" slang-bootstrap VERBATIM)
    add_custom_command(OUTPUT "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop_records.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop_records.pso"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry prop_vertex_records -stage vertex -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop_records.vso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/prop.slang"
        COMMAND $<TARGET_FILE:slang-bootstrap> -target dxbc -profile sm_5_0 -entry prop_pixel_records -stage pixel -O3 -matrix-layout-row-major -o "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop_records.pso" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/prop.slang"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/resource_access.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/environment_lighting.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/prop_surface.slangh" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/source/prop.slang" slang-bootstrap VERBATIM)
    add_custom_target(generals_graphics_prop_shaders DEPENDS "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop_records.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop_records.pso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop_instanced.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop.vso" "${GRAPHICS_SHADER_ASSET_DIRECTORY}/prop.pso")
    add_dependencies(generals_graphics generals_graphics_prop_shaders)
endif()
