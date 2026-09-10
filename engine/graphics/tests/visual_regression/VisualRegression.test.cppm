module;

#define BOOST_TEST_MODULE GraphicsVisualRegressionTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <vector>

export module Graphics.Testing.VisualRegression.Tests;

import Graphics.Passes.Opaque;
import Graphics.Passes.Shadow;
import Graphics.Resources.Bindless.BindlessResourceTable;
import Graphics.Resources.Materials.Material;
import Graphics.Resources.Residency.GPUResourceResidency;
import Graphics.Scene.DrawGeneration;
import Graphics.Scene.GPUScene;
import Graphics.Scene.LOD;
import Graphics.Scene.Visibility;
import Graphics.Scene.Beams;
import Graphics.Scene.Lighting.Renderer;
import Graphics.Shaders.Library;
import Graphics.Testing.VisualRegression;
import Graphics.Tests.Device;
import Graphics.Renderer2D;

using namespace Graphics;

#ifndef GRAPHICS_VISUAL_REFERENCE_DIRECTORY
#define GRAPHICS_VISUAL_REFERENCE_DIRECTORY "."
#endif

#ifndef GRAPHICS_UI2D_REFERENCE_DIRECTORY
#define GRAPHICS_UI2D_REFERENCE_DIRECTORY GRAPHICS_VISUAL_REFERENCE_DIRECTORY
#endif

#ifndef GRAPHICS_VISUAL_FAILURE_DIRECTORY
#define GRAPHICS_VISUAL_FAILURE_DIRECTORY "."
#endif

#ifndef GRAPHICS_VISUAL_SHADER_DIRECTORY
#define GRAPHICS_VISUAL_SHADER_DIRECTORY "."
#endif

#ifndef GRAPHICS_RENDERER_SHADER_DIRECTORY
#define GRAPHICS_RENDERER_SHADER_DIRECTORY "."
#endif

namespace
{
struct Vertex final
{
	float position[3]{};
	float color[4]{};
	float uv[2]{};
};

static_assert(sizeof(Vertex) == 36);

struct MaterialConstants final
{
	float color[4]{};
	std::uint32_t texture_index = Invalid_GPU_Index;
	std::uint32_t shadow_index = Invalid_GPU_Index;
	std::uint32_t reserved[2]{};
};

static_assert(sizeof(MaterialConstants) == 32);

enum class SceneKind : std::uint8_t
{
	BasicTriangle,
	TexturedMesh,
	LitMeshWithShadow,
	DynamicLight,
	Beam,
	LaserAdapter,
	RopeAdapter,
	ProjectileStreamAdapter,
	UI2DHudAndMenu
};

struct VisualScene final
{
	SceneKind kind = SceneKind::BasicTriangle;
	RHIPipelineHandle pipeline{};
	RHIPipelineHandle shadow_pipeline{};
	std::array<RHIBufferHandle, 2> vertex_buffers{};
	std::array<RHIBufferHandle, 2> index_buffers{};
	std::array<OpaqueMeshBinding, 2> meshes{};
	std::array<DrawData, 1> main_draws{};
	std::array<DrawData, 1> shadow_draws{};
	std::array<GPUInstanceData, 1> shadow_instances{};
	std::array<Vertex, 3> basic_vertices{};
	std::array<std::uint16_t, 3> basic_indices{};
	MeshPool mesh_pool;
	TexturePool texture_pool;
	SamplerPool sampler_pool;
	MaterialPool material_pool;
	RenderScene render_scene;
	GPUScene gpu_scene;
	View view{};
	ShaderLibrary shader_library;
	std::unique_ptr<GPUResourceResidency> residency;
	RHIBufferHandle instance_buffer{};
	MeshHandle scene_mesh{};
	MaterialHandle scene_material{};
	InstanceHandle scene_instance{};
	RHITextureHandle material_texture{};
	RHITextureHandle shadow_texture{};
	RHIBufferHandle draw_buffer{};
	RHIBufferHandle material_constants{};
	RHIBufferHandle bone_buffer{};
	RHIBufferHandle material_buffer{};
	BindlessResourceTable bindless;
	BeamRenderer beam_renderer;
	LightRenderer light_renderer;
	Renderer2D ui_renderer;
	RHITextureHandle ui_texture{};
	Renderer2DTexture ui_binding{};
	RHIBufferHandle auxiliary_light_buffer{};
	RHIBufferHandle view_buffer{};

	void Release(Device &device) noexcept
	{
		ui_renderer.Shutdown();
		if (ui_texture.Is_Valid())
			device.Destroy_Texture(ui_texture);
		ui_texture = {};
		ui_binding = {};
		light_renderer.Shutdown();
		beam_renderer.Shutdown();
		bindless.Clear();
		if (instance_buffer.Is_Valid())
			device.Destroy_Buffer(instance_buffer);
		instance_buffer = {};
		if (draw_buffer.Is_Valid())
			device.Destroy_Buffer(draw_buffer);
		draw_buffer = {};
		if (auxiliary_light_buffer.Is_Valid())
			device.Destroy_Buffer(auxiliary_light_buffer);
		auxiliary_light_buffer = {};
		if (view_buffer.Is_Valid())
			device.Destroy_Buffer(view_buffer);
		view_buffer = {};
		if (bone_buffer.Is_Valid())
			device.Destroy_Buffer(bone_buffer);
		bone_buffer = {};
		if (material_buffer.Is_Valid())
			device.Destroy_Buffer(material_buffer);
		material_buffer = {};
		residency.reset();
		if (shadow_texture.Is_Valid())
			device.Destroy_Texture(shadow_texture);
		if (material_texture.Is_Valid())
			device.Destroy_Texture(material_texture);
		if (material_constants.Is_Valid())
			device.Destroy_Buffer(material_constants);
		for (RHIBufferHandle buffer : index_buffers) {
			if (buffer.Is_Valid())
				device.Destroy_Buffer(buffer);
		}
		for (RHIBufferHandle buffer : vertex_buffers) {
			if (buffer.Is_Valid())
				device.Destroy_Buffer(buffer);
		}
		if (shadow_pipeline.Is_Valid())
			device.Destroy_Pipeline(shadow_pipeline);
		if (pipeline.Is_Valid())
			device.Destroy_Pipeline(pipeline);
	}

	bool Initialize(Device &device)
	{
		if (kind == SceneKind::UI2DHudAndMenu)
			return Initialize_UI2D(device);
		if (kind == SceneKind::Beam || kind == SceneKind::LaserAdapter || kind == SceneKind::RopeAdapter || kind == SceneKind::ProjectileStreamAdapter)
			return Initialize_Beam(device);
		if (kind == SceneKind::BasicTriangle)
			return Initialize_Basic_Opaque(device);
		if (kind == SceneKind::DynamicLight)
			return Initialize_Dynamic_Light(device);

		std::vector<std::byte> vertex_shader;
		std::vector<std::byte> pixel_shader;
		if (!Load_Binary_File(Graphics::Test_Shader_Directory(GRAPHICS_VISUAL_SHADER_DIRECTORY) / "visual_basic.vso", vertex_shader))
			return false;

		const char *pixel_name = kind == SceneKind::TexturedMesh ? "visual_textured.pso" : kind == SceneKind::LitMeshWithShadow ? "visual_lit_shadow.pso" : "visual_basic.pso";
		if (!Load_Binary_File(Graphics::Test_Shader_Directory(GRAPHICS_VISUAL_SHADER_DIRECTORY) / pixel_name, pixel_shader))
			return false;

		const RHIPipeline pipeline_description{static_cast<std::uint64_t>(kind) + 1, true, true, RHIPrimitiveTopology::TriangleList, RHIVertexFormat::Position3Color4UV2, RHIBlendMode::Disabled};
		pipeline = device.Create_Pipeline(pipeline_description, {vertex_shader}, {pixel_shader});
		if (!pipeline.Is_Valid())
			return false;

		ResourceIndex texture_index{};
		if (kind == SceneKind::TexturedMesh) {
			const std::array<std::uint8_t, 16> texture_data = {
				255, 255, 255, 255, 20, 40, 220, 255,
				20, 40, 220, 255, 255, 255, 255, 255
			};
			material_texture = device.Create_Texture_Initialized(
				{2, 2, 1, RHITextureFormat::RGBA8_UNorm, static_cast<std::uint32_t>(RHITextureUsage::ShaderResource), 1},
				{std::as_bytes(std::span<const std::uint8_t>(texture_data)), 8});
			if (!material_texture.Is_Valid())
				return false;
			texture_index = bindless.Register_Texture(TextureHandle(0, 1), material_texture);
			if (!texture_index.Is_Valid())
				return false;
		}

		ResourceIndex shadow_index{};
		if (kind == SceneKind::LitMeshWithShadow) {
			if (!Load_Binary_File(Graphics::Test_Shader_Directory(GRAPHICS_VISUAL_SHADER_DIRECTORY) / "visual_shadow.vso", vertex_shader))
				return false;
			if (!Load_Binary_File(Graphics::Test_Shader_Directory(GRAPHICS_VISUAL_SHADER_DIRECTORY) / "visual_shadow.pso", pixel_shader))
				return false;
			shadow_pipeline = device.Create_Pipeline(pipeline_description, {vertex_shader}, {pixel_shader});
			if (!shadow_pipeline.Is_Valid())
				return false;

			shadow_texture = device.Create_Texture({
				32,
				32,
				1,
				RHITextureFormat::D32_Float,
				static_cast<std::uint32_t>(RHITextureUsage::ShaderResource) | static_cast<std::uint32_t>(RHITextureUsage::DepthStencil),
				1
			});
			if (!shadow_texture.Is_Valid())
				return false;
			shadow_index = bindless.Register_Texture(TextureHandle(1, 1), shadow_texture);
			if (!shadow_index.Is_Valid())
				return false;
		}

		const MaterialConstants material_data{
			{1.0f, 1.0f, 1.0f, 1.0f},
			texture_index.Is_Valid() ? texture_index.Get_Index() : Invalid_GPU_Index,
			shadow_index.Is_Valid() ? shadow_index.Get_Index() : Invalid_GPU_Index,
			{0, 0}
		};
		material_constants = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(material_data)), RHIBufferUsage::Constant, 16},
			std::as_bytes(std::span<const MaterialConstants>(&material_data, 1)));
		if (!material_constants.Is_Valid())
			return false;
		if (!bindless.Register_Material(MaterialHandle(0, 1), material_constants).Is_Valid())
			return false;

		if (!Create_Geometry(device))
			return false;

		main_draws[0] = {0, 0, 0, 1, pipeline, 0};
		shadow_draws[0] = {1, 0, 0, 1, shadow_pipeline, 0};
		shadow_instances[0].flags = static_cast<std::uint32_t>(RenderInstanceFlags::CastsShadow);
		return true;
	}

	bool Initialize_UI2D(Device &device)
	{
		if (!ui_renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_RENDERER_SHADER_DIRECTORY), 4096, 6144, 128))
			return false;

		constexpr std::uint32_t texture_width = 64;
		constexpr std::uint32_t texture_height = 32;
		std::array<std::uint8_t, texture_width * texture_height * 4> texture_data{};
		const auto set_pixel = [&](std::uint32_t x, std::uint32_t y, std::array<std::uint8_t, 4> color) {
			const std::size_t offset = (static_cast<std::size_t>(y) * texture_width + x) * 4;
			for (std::size_t channel = 0; channel < color.size(); ++channel)
				texture_data[offset + channel] = color[channel];
		};
		for (std::uint32_t y = 0; y < 24; ++y) {
			for (std::uint32_t x = 0; x < 24; ++x)
				set_pixel(x, y, ((x / 4 + y / 4) & 1) != 0 ? std::array<std::uint8_t, 4>{36, 145, 205, 255} : std::array<std::uint8_t, 4>{220, 185, 55, 255});
		}
		constexpr std::array<std::array<std::uint8_t, 8>, 4> glyph_patterns = {
			std::array<std::uint8_t, 8>{0x7e, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x00},
			std::array<std::uint8_t, 8>{0x7c, 0x42, 0x42, 0x7c, 0x42, 0x42, 0x7c, 0x00},
			std::array<std::uint8_t, 8>{0x3c, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3c, 0x00},
			std::array<std::uint8_t, 8>{0x7c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7c, 0x00}};
		for (std::size_t glyph = 0; glyph < glyph_patterns.size(); ++glyph) {
			for (std::uint32_t y = 0; y < 8; ++y) {
				for (std::uint32_t x = 0; x < 8; ++x) {
					if ((glyph_patterns[glyph][y] & (1u << (7 - x))) != 0)
						set_pixel(32 + static_cast<std::uint32_t>(glyph) * 8 + x, y, {245, 245, 245, 255});
				}
			}
		}
		ui_texture = device.Create_Texture_Initialized(
			{texture_width, texture_height, 1, RHITextureFormat::RGBA8_UNorm, static_cast<std::uint32_t>(RHITextureUsage::ShaderResource), 1},
			{std::as_bytes(std::span<const std::uint8_t>(texture_data)), texture_width * 4});
		if (!ui_texture.Is_Valid())
			return false;
		ui_binding = ui_renderer.Register_Texture(TextureHandle(1, 1), ui_texture);
		return ui_binding.index.Is_Valid();
	}

	bool Initialize_Beam(Device &device)
	{
		const std::size_t beam_capacity = kind == SceneKind::RopeAdapter || kind == SceneKind::ProjectileStreamAdapter ? 16 : 4;
		if (!beam_renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_RENDERER_SHADER_DIRECTORY), beam_capacity))
			return false;

		if (kind == SceneKind::Beam) {
			BeamDescription beam;
			beam.start = {-0.75f, -0.15f, 0.0f};
			beam.end = {0.75f, 0.15f, 0.0f};
			beam.width = 0.10f;
			beam.color = {1.0f, 0.25f, 0.05f, 1.0f};
			beam.opacity = 0.75f;
			return beam_renderer.Create(beam).Is_Valid();
		}

		if (kind == SceneKind::ProjectileStreamAdapter) {
			constexpr std::array<Vec3, 8> points = {{
				{-0.86f, -0.68f, 0.0f},
				{-0.62f, -0.43f, 0.0f},
				{-0.72f, -0.15f, 0.0f},
				{0.0f, 0.0f, 0.0f},
				{-0.35f, 0.16f, 0.0f},
				{-0.12f, 0.42f, 0.0f},
				{-0.24f, 0.68f, 0.0f},
				{0.02f, 0.88f, 0.0f}
			}};
			for (std::size_t index = 0; index + 1 < points.size(); ++index) {
				if ((points[index].x == 0.0f && points[index].y == 0.0f && points[index].z == 0.0f)
					|| (points[index + 1].x == 0.0f && points[index + 1].y == 0.0f && points[index + 1].z == 0.0f))
					continue;
				BeamDescription segment;
				segment.start = points[index];
				segment.end = points[index + 1];
				segment.width = 0.10f;
				segment.color = {1.0f, 0.38f, 0.06f, 1.0f};
				segment.opacity = 0.72f;
				segment.uv_scale = 2.5f;
				segment.uv_offset = 0.25f;
				if (!beam_renderer.Create(segment).Is_Valid())
					return false;
			}
			return true;
		}

		if (kind == SceneKind::RopeAdapter) {
			constexpr std::array<Vec3, 7> points = {{
				{-0.78f, -0.72f, 0.0f},
				{-0.58f, -0.46f, 0.0f},
				{-0.68f, -0.19f, 0.0f},
				{-0.48f, 0.08f, 0.0f},
				{-0.60f, 0.35f, 0.0f},
				{-0.38f, 0.60f, 0.0f},
				{-0.48f, 0.82f, 0.0f}
			}};
			for (std::size_t index = 0; index + 1 < points.size(); ++index) {
				BeamDescription soft;
				soft.start = points[index];
				soft.end = points[index + 1];
				soft.width = 0.13f;
				soft.color = {0.95f, 0.35f, 0.08f, 1.0f};
				soft.opacity = 0.35f;
				if (!beam_renderer.Create(soft).Is_Valid())
					return false;

				BeamDescription hard = soft;
				hard.width = 0.06f;
				hard.color = {1.0f, 0.80f, 0.30f, 1.0f};
				hard.opacity = 1.0f;
				if (!beam_renderer.Create(hard).Is_Valid())
					return false;
			}
			return true;
		}

		BeamDescription outer;
		outer.start = {-0.80f, 0.0f, 0.0f};
		outer.end = {0.80f, 0.0f, 0.0f};
		outer.width = 0.18f;
		outer.color = {1.0f, 0.08f, 0.01f, 1.0f};
		outer.opacity = 0.45f;
		if (!beam_renderer.Create(outer).Is_Valid())
			return false;

		BeamDescription inner = outer;
		inner.width = 0.07f;
		inner.color = {1.0f, 0.90f, 0.25f, 1.0f};
		inner.opacity = 0.90f;
		return beam_renderer.Create(inner).Is_Valid();
	}

	bool Initialize_Basic_Opaque(Device &device)
	{
		basic_vertices = {{
			{{0.0f, 0.78f, 0.35f}, {1.0f, 0.20f, 0.10f, 1.0f}, {0.5f, 0.0f}},
			{{-0.72f, -0.62f, 0.35f}, {0.10f, 0.85f, 0.20f, 1.0f}, {0.0f, 1.0f}},
			{{0.72f, -0.62f, 0.35f}, {0.10f, 0.20f, 0.95f, 1.0f}, {1.0f, 1.0f}}
		}};
	basic_indices = {0, 1, 2};

		const ShaderHandle basic_shader = shader_library.Load_Basic_Opaque(Graphics::Test_Shader_Directory(GRAPHICS_RENDERER_SHADER_DIRECTORY));
		if (!basic_shader.Is_Valid())
			return false;

		Mesh mesh;
		mesh.vertex_count = static_cast<std::uint32_t>(basic_vertices.size());
		mesh.index_count = static_cast<std::uint32_t>(basic_indices.size());
		mesh.vertex_stride = sizeof(Vertex);
		mesh.index_format = MeshIndexFormat::UInt16;
		mesh.vertex_data = std::as_bytes(std::span<const Vertex>(basic_vertices));
		mesh.index_data = std::as_bytes(std::span<const std::uint16_t>(basic_indices));
		scene_mesh = mesh_pool.Create(mesh);

		Material material;
		material.shader = basic_shader;
		material.parameters.values[0] = 1.0f;
		material.parameters.values[1] = 1.0f;
		material.parameters.values[2] = 1.0f;
		material.parameters.values[3] = 1.0f;
		const ShaderHandle selected_shader = shader_library.Select_Shader(material, shader_library.Basic_Opaque());
		const PipelineDesc pipeline_description = shader_library.Make_Pipeline_Description(selected_shader, Make_Basic_Opaque_Pipeline());
		pipeline = shader_library.Create_Pipeline(device, selected_shader, pipeline_description);
		if (!pipeline.Is_Valid())
			return false;
		scene_material = material_pool.Create(material);

		RenderInstance instance;
		instance.transform.matrix = Matrix4x4::Identity().values;
		instance.bounds = {{0.0f, 0.0f, 0.35f}, 1.0f};
		instance.mesh = scene_mesh;
		instance.material = scene_material;
		scene_instance = render_scene.Create(instance);
		view = View(Matrix4x4::Identity(), Matrix4x4::Identity(), {}, {0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f});

		gpu_scene.Reserve(1, 1, 1);
		if (!gpu_scene.Build(render_scene, mesh_pool, texture_pool, sampler_pool, material_pool))
			return false;

		residency = std::make_unique<GPUResourceResidency>(device);
		if (!residency->Upload_Mesh(scene_mesh, mesh_pool) || !residency->Upload_Material(scene_material, material_pool))
			return false;

		const std::span<const GPUInstanceData> instance_data = gpu_scene.Instances();
		instance_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(instance_data.size_bytes()), RHIBufferUsage::Storage, sizeof(GPUInstanceData)},
			std::as_bytes(instance_data));
		if (!instance_buffer.Is_Valid() || !bindless.Register_Buffer(instance_buffer).Is_Valid())
			return false;

		const std::array<float, 16> view_data = Matrix4x4::Identity().values;
		const GPULightData empty_light{};
		auxiliary_light_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(empty_light)), RHIBufferUsage::Storage, sizeof(empty_light)},
			std::as_bytes(std::span<const GPULightData>(&empty_light, 1)));
		view_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(view_data)), RHIBufferUsage::Storage, sizeof(float) * 16},
			std::as_bytes(std::span<const float>(view_data)));
		if (!auxiliary_light_buffer.Is_Valid() || !bindless.Register_Buffer(auxiliary_light_buffer).Is_Valid()
			|| !view_buffer.Is_Valid() || !bindless.Register_Buffer(view_buffer).Is_Valid())
			return false;

		const GPUBoneMatrixData empty_bone{};
		bone_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(empty_bone)), RHIBufferUsage::Storage, sizeof(empty_bone)},
			std::as_bytes(std::span<const GPUBoneMatrixData>(&empty_bone, 1)));
		const std::span<const GPUMaterialData> materials = gpu_scene.Materials();
		material_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(materials.size_bytes()), RHIBufferUsage::Storage, sizeof(GPUMaterialData)},
			std::as_bytes(materials));
		if (!bone_buffer.Is_Valid() || !bindless.Register_Buffer(bone_buffer).Is_Valid()
			|| !material_buffer.Is_Valid() || !bindless.Register_Buffer(material_buffer).Is_Valid())
			return false;
		const GPUDrawData draw_data{0, 0, 0, 0};
		draw_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(draw_data)), RHIBufferUsage::Storage, sizeof(draw_data)},
			std::as_bytes(std::span<const GPUDrawData>(&draw_data, 1)));
		if (!draw_buffer.Is_Valid() || !bindless.Register_Buffer(draw_buffer).Is_Valid())
			return false;

		const GPUResidentMaterial resident_material = residency->Material_Info(scene_material);
		if (!resident_material.constants.Is_Valid() || !bindless.Register_Material(scene_material, resident_material.constants).Is_Valid())
			return false;

		const GPUResidentMesh resident_mesh = residency->Mesh_Info(scene_mesh);
		meshes[0] = {
			resident_mesh.vertex_buffer,
			resident_mesh.index_buffer,
			RHIIndexFormat::UInt16,
			resident_mesh.vertex_stride,
			resident_mesh.index_count,
			0,
			0
		};
		return resident_mesh.vertex_buffer.Is_Valid() && resident_mesh.index_buffer.Is_Valid();
	}

	bool Initialize_Dynamic_Light(Device &device)
	{
		const ShaderHandle basic_shader = shader_library.Load_Basic_Opaque(Graphics::Test_Shader_Directory(GRAPHICS_RENDERER_SHADER_DIRECTORY));
		if (!basic_shader.Is_Valid())
			return false;

		const PipelineDesc pipeline_description = shader_library.Make_Pipeline_Description(basic_shader, Make_Basic_Opaque_Pipeline());
		pipeline = shader_library.Create_Pipeline(device, basic_shader, pipeline_description);
		if (!pipeline.Is_Valid() || !light_renderer.Initialize(device, 1))
			return false;

		RenderLight light;
		light.position = {0.0f, 0.0f, 0.35f};
		light.color = {1.0f, 0.35f, 0.08f};
		light.intensity = 1.5f;
		light.range = 1.5f;
		if (!light_renderer.Create_Point_Light(light).Is_Valid()
			|| !Create_Geometry(device))
			return false;

		GPUInstanceData instance;
		instance.transform = Matrix4x4::Identity().values;
		instance.bounds = {0.0f, 0.0f, 0.35f, 1.0f};
		instance.mesh_index = 0;
		instance.material_index = 0;
		instance_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(instance)), RHIBufferUsage::Storage, sizeof(instance)},
			std::as_bytes(std::span<const GPUInstanceData>(&instance, 1)));
		if (!instance_buffer.Is_Valid()
			|| !bindless.Register_Buffer(instance_buffer).Is_Valid()
			|| !bindless.Register_Buffer(light_renderer.Light_Buffer()).Is_Valid())
			return false;
		const std::array<float, 16> view_data = Matrix4x4::Identity().values;
		view_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(view_data)), RHIBufferUsage::Storage, sizeof(float) * 16},
			std::as_bytes(std::span<const float>(view_data)));
		if (!view_buffer.Is_Valid() || !bindless.Register_Buffer(view_buffer).Is_Valid())
			return false;
		const GPUBoneMatrixData empty_bone{};
		bone_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(empty_bone)), RHIBufferUsage::Storage, sizeof(empty_bone)},
			std::as_bytes(std::span<const GPUBoneMatrixData>(&empty_bone, 1)));
		GPUMaterialData material_gpu{};
		material_gpu.texture_indices.fill(Invalid_GPU_Index);
		material_gpu.sampler_indices.fill(Invalid_GPU_Index);
		material_gpu.parameters[0] = 1.0f;
		material_gpu.parameters[1] = 1.0f;
		material_gpu.parameters[2] = 1.0f;
		material_gpu.parameters[3] = 1.0f;
		material_gpu.parameters[4] = 1.0f;
		material_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(material_gpu)), RHIBufferUsage::Storage, sizeof(material_gpu)},
			std::as_bytes(std::span<const GPUMaterialData>(&material_gpu, 1)));
		if (!bone_buffer.Is_Valid() || !bindless.Register_Buffer(bone_buffer).Is_Valid()
			|| !material_buffer.Is_Valid() || !bindless.Register_Buffer(material_buffer).Is_Valid())
			return false;
		const GPUDrawData draw_data{0, 0, 0, 0};
		draw_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(draw_data)), RHIBufferUsage::Storage, sizeof(draw_data)},
			std::as_bytes(std::span<const GPUDrawData>(&draw_data, 1)));
		if (!draw_buffer.Is_Valid() || !bindless.Register_Buffer(draw_buffer).Is_Valid())
			return false;

		MaterialParameterBlock material_data;
		material_data.values[0] = 1.0f;
		material_data.values[1] = 1.0f;
		material_data.values[2] = 1.0f;
		material_data.values[3] = 1.0f;
		material_data.values[4] = 1.0f;
		material_constants = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(material_data)), RHIBufferUsage::Constant, 16},
			material_data.Bytes());
		if (!material_constants.Is_Valid()
			|| !bindless.Register_Material(MaterialHandle(0, 1), material_constants).Is_Valid()
			|| !light_renderer.Sync())
			return false;

		main_draws[0] = {0, 0, 0, 1, pipeline, 0};
		return true;
	}

private:
	bool Create_Geometry(Device &device)
	{
		constexpr std::array<Vertex, 3> triangle = {{
			{{0.0f, 0.78f, 0.35f}, {1.0f, 0.20f, 0.10f, 1.0f}, {0.5f, 0.0f}},
			{{-0.72f, -0.62f, 0.35f}, {0.10f, 0.85f, 0.20f, 1.0f}, {0.0f, 1.0f}},
			{{0.72f, -0.62f, 0.35f}, {0.10f, 0.20f, 0.95f, 1.0f}, {1.0f, 1.0f}}
		}};
		constexpr std::array<std::uint16_t, 3> triangle_indices = {0, 1, 2};

		std::array<Vertex, 4> receiver = {{
			{{-0.86f, 0.82f, 0.80f}, {0.80f, 0.80f, 0.80f, 1.0f}, {0.0f, 0.0f}},
			{{0.86f, 0.82f, 0.80f}, {0.80f, 0.80f, 0.80f, 1.0f}, {1.0f, 0.0f}},
			{{0.86f, -0.82f, 0.80f}, {0.80f, 0.80f, 0.80f, 1.0f}, {1.0f, 1.0f}},
			{{-0.86f, -0.82f, 0.80f}, {0.80f, 0.80f, 0.80f, 1.0f}, {0.0f, 1.0f}}
		}};
		constexpr std::array<std::uint16_t, 6> receiver_indices = {0, 1, 2, 0, 2, 3};
		constexpr std::array<Vertex, 3> occluder = {{
			{{-0.30f, 0.30f, 0.20f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
			{{0.30f, 0.30f, 0.20f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
			{{0.0f, -0.30f, 0.20f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 1.0f}}
		}};

		const auto create_mesh = [&](std::size_t index, std::span<const Vertex> vertices, std::span<const std::uint16_t> indices) {
			vertex_buffers[index] = device.Create_Buffer_Initialized(
				{static_cast<std::uint32_t>(vertices.size_bytes()), RHIBufferUsage::Vertex, sizeof(Vertex)},
				std::as_bytes(vertices));
			index_buffers[index] = device.Create_Buffer_Initialized(
				{static_cast<std::uint32_t>(indices.size_bytes()), RHIBufferUsage::Index, 0},
				std::as_bytes(indices));
			meshes[index] = {vertex_buffers[index], index_buffers[index], RHIIndexFormat::UInt16, sizeof(Vertex), static_cast<std::uint32_t>(indices.size()), 0, 0};
			return vertex_buffers[index].Is_Valid() && index_buffers[index].Is_Valid();
		};

		if (kind == SceneKind::LitMeshWithShadow)
			return create_mesh(0, receiver, receiver_indices) && create_mesh(1, occluder, triangle_indices);
		if (kind == SceneKind::DynamicLight)
			return create_mesh(0, receiver, receiver_indices);
		return create_mesh(0, triangle, triangle_indices);
	}
};

static bool Render_Scene(Device &device, CommandList &commands, RHITextureHandle color_target, RHITextureHandle depth_target, RHIViewport viewport, void *context) noexcept
{
	VisualScene &scene = *static_cast<VisualScene *>(context);
	if (scene.kind == SceneKind::UI2DHudAndMenu) {
		if (!commands.Set_Render_Targets(color_target, depth_target)
			|| !commands.Clear({0.015f, 0.025f, 0.045f, 1.0f}, 1.0f))
			return false;

		scene.ui_renderer.Begin(viewport.width, viewport.height);
		const Color2D panel_color{0.035f, 0.075f, 0.13f, 0.97f};
		const Color2D border_color{0.20f, 0.70f, 0.92f, 0.95f};
		if (!scene.ui_renderer.Add_Rect({8, 7, 120, 39}, panel_color)
			|| !scene.ui_renderer.Add_Outline({8, 7, 120, 39}, 1, border_color)
			|| !scene.ui_renderer.Add_Quad(Rect2D{14, 13, 38, 37}, Rect2D{0, 0, 24.0f / 64.0f, 24.0f / 32.0f}, scene.ui_binding, Color2D{1, 1, 1, 1}, Renderer2DBlendMode::Alpha)
			|| !scene.ui_renderer.Add_Rect({43, 12, 114, 21}, {0.08f, 0.14f, 0.22f, 0.95f})
			|| !scene.ui_renderer.Add_Rect({43, 24, 106, 31}, {0.05f, 0.10f, 0.17f, 0.95f})
			|| !scene.ui_renderer.Add_Outline({43, 12, 114, 21}, 1, {0.18f, 0.42f, 0.58f, 0.9f}))
			return false;

		std::array<Renderer2DGlyph, 12> label{};
		for (std::size_t index = 0; index < label.size(); ++index) {
			const float left = 48.0f + static_cast<float>(index) * 5.0f;
			const float atlas_left = static_cast<float>(32 + (index % 4) * 8) / 64.0f;
			label[index] = {{left, 15, left + 4, 22}, {atlas_left, 0, atlas_left + 8.0f / 64.0f, 8.0f / 32.0f}, {0.88f, 0.92f, 0.96f, 1.0f}};
		}
		std::array<Renderer2DGlyph, 12> label_shadow = label;
		for (Renderer2DGlyph &glyph : label_shadow) {
			glyph.screen.left += 1;
			glyph.screen.right += 1;
			glyph.screen.top += 1;
			glyph.screen.bottom += 1;
			glyph.color = {0, 0, 0, 0.8f};
		}
		if (!scene.ui_renderer.Add_Text_Glyphs(label_shadow, scene.ui_binding)
			|| !scene.ui_renderer.Add_Text_Glyphs(label, scene.ui_binding))
			return false;
		label[4].color = {1.0f, 0.80f, 0.22f, 1.0f};
		if (!scene.ui_renderer.Add_Text_Glyphs(std::span<const Renderer2DGlyph>(label.data() + 4, 1), scene.ui_binding))
			return false;

		scene.ui_renderer.Set_Clip(true, {64, 48, 124, 68});
		const std::array<Color2D, 4> health_gradient = {{{0.10f, 0.85f, 0.35f, 1.0f}, {0.10f, 0.85f, 0.35f, 1.0f}, {0.90f, 0.72f, 0.16f, 1.0f}, {0.90f, 0.72f, 0.16f, 1.0f}}};
		if (!scene.ui_renderer.Add_Rect({61, 49, 124, 68}, {0.04f, 0.08f, 0.12f, 0.96f})
			|| !scene.ui_renderer.Add_Line({61, 58}, {123, 58}, 5, health_gradient)
			|| !scene.ui_renderer.Add_Quad(Rect2D{52, 47, 75, 66}, Rect2D{0, 0, 24.0f / 64.0f, 24.0f / 32.0f}, scene.ui_binding, Color2D{1, 1, 1, 1}, Renderer2DBlendMode::Alpha, true))
			return false;
		scene.ui_renderer.Set_Clip(false, {});

		if (!scene.ui_renderer.Add_Triangle({97, 56}, {108, 56}, {108, 65}, {0.12f, 0.52f, 0.88f, 0.9f})
			|| !scene.ui_renderer.Add_Outline({64, 48, 124, 68}, 1, border_color))
			return false;
		return scene.ui_renderer.Execute(device, commands, color_target, depth_target, viewport);
	}
	if (scene.kind == SceneKind::Beam || scene.kind == SceneKind::LaserAdapter || scene.kind == SceneKind::RopeAdapter || scene.kind == SceneKind::ProjectileStreamAdapter) {
		return commands.Set_Render_Targets(color_target, depth_target)
			&& commands.Clear({0.02f, 0.02f, 0.03f, 1.0f}, 1.0f)
			&& scene.beam_renderer.Render(commands, color_target, depth_target, viewport);
	}

	std::array<InstanceHandle, 1> visible_storage{};
	VisibleSet visible_set(visible_storage);
	std::array<LODSelection, 1> lod_storage{};
	LODSet lod_set(lod_storage);
	std::array<DrawData, 1> generated_draw_storage{};
	DrawSet generated_draw_set(generated_draw_storage);
	std::span<const DrawData> main_draws = scene.main_draws;
	if (scene.kind == SceneKind::DynamicLight && !scene.light_renderer.Sync())
		return false;
	if (scene.kind == SceneKind::BasicTriangle) {
		if (!Build_Visible_Set(scene.render_scene, scene.view, visible_set)
			|| !Build_LOD_Set(scene.render_scene, scene.mesh_pool, visible_set, scene.view, lod_set)
			|| !Build_Draw_Data(lod_set, scene.gpu_scene, {20, scene.pipeline, 0}, generated_draw_set))
			return false;
		main_draws = generated_draw_set.Records();
	}

	RenderGraph graph;
	graph.Reserve(3, 2, 5, 1);
	const GraphResourceHandle color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle shadow = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle shadow_pass = scene.kind == SceneKind::LitMeshWithShadow ? ShadowPass::Add_To_Graph(graph, shadow, 10) : GraphPassHandle{};
	const GraphPassHandle opaque_pass = OpaquePass::Add_To_Graph(graph, color, depth, 20);
	if (!color.Is_Valid() || !depth.Is_Valid() || !opaque_pass.Is_Valid())
		return false;
	if (scene.kind == SceneKind::LitMeshWithShadow && (!shadow_pass.Is_Valid() || !graph.Add_Dependency(opaque_pass, shadow_pass)))
		return false;

	std::array<GraphResourceBinding, 3> bindings = {
		GraphResourceBinding::Texture(color, color_target),
		GraphResourceBinding::Texture(depth, depth_target),
		GraphResourceBinding::Texture(shadow, scene.shadow_texture)
	};
	const std::size_t binding_count = scene.kind == SceneKind::LitMeshWithShadow ? 3 : 2;
	ExecutionPlan plan;
	if (!plan.Compile(graph, std::span<const GraphResourceBinding>(bindings.data(), binding_count)))
		return false;

	const std::span<const RHIBindlessResource> bindless_resources = scene.bindless.Resources();
	const OpaquePassInput opaque_input{
		main_draws,
		scene.meshes,
		bindless_resources,
		color,
		depth,
		viewport,
		{0.035f, 0.045f, 0.075f, 1.0f},
		1.0f,
		true
	};
	const ShadowPassInput shadow_input{
		scene.shadow_draws,
		scene.meshes,
		scene.shadow_instances,
		scene.shadow_pipeline,
		shadow,
		{0, 0, 32, 32, 0.0f, 1.0f},
		1.0f
	};
	return plan.Execute(graph, commands, [&](GraphPassHandle pass, CommandList &command_list, const PassResources &resources) noexcept {
		if (pass == opaque_pass)
			return OpaquePass::Execute(command_list, resources, opaque_input);
		return pass == shadow_pass && ShadowPass::Execute(command_list, resources, shadow_input);
	});
}

static VisualRegressionConfig Make_Config() noexcept
{
	return {
		128,
		72,
		2,
		std::filesystem::path(GRAPHICS_VISUAL_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_VISUAL_FAILURE_DIRECTORY)
	};
}

static void Run_Scene(SceneKind kind, const char *name)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());
	VisualScene scene;
	scene.kind = kind;
	BOOST_REQUIRE(scene.Initialize(device));
	VisualRegressionConfig config = Make_Config();
	if (kind == SceneKind::UI2DHudAndMenu)
		config.reference_directory = std::filesystem::path(GRAPHICS_UI2D_REFERENCE_DIRECTORY);
	VisualRegressionHarness harness(config);
	const VisualComparisonResult result = harness.Run(device, name, Render_Scene, &scene);
	BOOST_CHECK_MESSAGE(result.expected_loaded, "missing visual reference image");
	BOOST_CHECK_MESSAGE(result.matched, "visual regression mismatch");
	scene.Release(device);
}
}

BOOST_AUTO_TEST_CASE(basic_triangle_matches_golden_image)
{
	Run_Scene(SceneKind::BasicTriangle, "basic_triangle");
}

BOOST_AUTO_TEST_CASE(textured_mesh_matches_golden_image)
{
	Run_Scene(SceneKind::TexturedMesh, "textured_mesh");
}

BOOST_AUTO_TEST_CASE(lit_mesh_with_shadow_matches_golden_image)
{
	Run_Scene(SceneKind::LitMeshWithShadow, "lit_mesh_with_shadow");
}

BOOST_AUTO_TEST_CASE(dynamic_light_matches_golden_image)
{
	Run_Scene(SceneKind::DynamicLight, "dynamic_light");
}

BOOST_AUTO_TEST_CASE(generic_beam_matches_golden_image)
{
	Run_Scene(SceneKind::Beam, "generic_beam");
}

BOOST_AUTO_TEST_CASE(laser_adapter_matches_golden_image)
{
	Run_Scene(SceneKind::LaserAdapter, "laser_adapter");
}

BOOST_AUTO_TEST_CASE(rope_adapter_matches_golden_image)
{
	Run_Scene(SceneKind::RopeAdapter, "rope_adapter");
}

BOOST_AUTO_TEST_CASE(projectile_stream_adapter_matches_golden_image)
{
	Run_Scene(SceneKind::ProjectileStreamAdapter, "projectile_stream_adapter");
}

BOOST_AUTO_TEST_CASE(ui_2d_hud_and_menu_matches_golden_image)
{
	Run_Scene(SceneKind::UI2DHudAndMenu, "ui_2d_hud_and_menu");
}
