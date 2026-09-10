module;

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <span>
#include <vector>

#if defined(RTS_PROFILE_TRACY)
#include <tracy/Tracy.hpp>
#define GRAPHICS_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#endif

export module Graphics.Scene.Particles.Renderer;

export import Graphics.Passes.Particles;
export import Graphics.Resources.Bindless.BindlessResourceTable;
export import Graphics.Resources.Materials.Material;
export import Graphics.Resources.Residency.GPUResourceResidency;
export import Graphics.Scene.GPUScene;
export import Graphics.Scene.Views.View;
export import Graphics.Shaders.Library;

import Graphics.Memory.AlignedAllocator;

namespace Graphics
{

export class ParticleRenderer final
{
public:
	bool Initialize(Device &device, const std::filesystem::path &shader_directory, std::size_t max_emitters = 1024, std::size_t max_particles = 32768)
	{
		if (m_device != nullptr || !device.Is_Valid() || max_emitters == 0 || max_particles == 0 || max_particles > std::numeric_limits<std::uint32_t>::max() / sizeof(GPUParticleData))
			return false;

		m_device = &device;
		m_particles.Reserve(max_emitters, max_particles);
		m_gpu_particles.resize(max_particles);
		m_visible_storage.resize(max_particles);
		m_draw_storage.resize(max_particles);
		m_graph = std::make_unique<RenderGraph>();
		m_graph->Reserve(2, 1, 2);
		m_color_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		m_depth_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		m_pass = ParticlePass::Add_To_Graph(*m_graph, m_color_resource, m_depth_resource, 40);
		if (!m_color_resource.Is_Valid() || !m_depth_resource.Is_Valid() || !m_pass.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_shader = m_shaders.Load_Particle_Billboard(shader_directory);
		if (!m_shader.Is_Valid()) {
			Shutdown();
			return false;
		}

		Material material;
		material.shader = m_shader;
		material.parameters.values[0] = 1.0f;
		material.parameters.values[1] = 1.0f;
		material.parameters.values[2] = 1.0f;
		material.parameters.values[3] = 1.0f;
		m_material = m_materials.Create(material);
		if (!m_material.Is_Valid()) {
			Shutdown();
			return false;
		}

		const PipelineDesc pipeline_description = m_shaders.Make_Pipeline_Description(m_shader, Make_Particle_Billboard_Pipeline());
		PipelineDesc alpha_test_description = pipeline_description;
		alpha_test_description.depth_write = true;
		alpha_test_description.blend_mode = RHIBlendMode::Disabled;
		PipelineDesc additive_description = pipeline_description;
		additive_description.blend_mode = RHIBlendMode::Additive;
		PipelineDesc multiply_description = pipeline_description;
		multiply_description.blend_mode = RHIBlendMode::Multiply;
		std::array<PipelineDesc, 4> point_sprite_descriptions = {
			pipeline_description,
			additive_description,
			multiply_description,
			alpha_test_description
		};
		m_pipelines[0] = m_shaders.Create_Pipeline(device, m_shader, pipeline_description);
		m_pipelines[1] = m_shaders.Create_Pipeline(device, m_shader, additive_description);
		m_pipelines[2] = m_shaders.Create_Pipeline(device, m_shader, multiply_description);
		m_pipelines[3] = m_shaders.Create_Pipeline(device, m_shader, alpha_test_description);
		for (std::size_t index = 0; index < point_sprite_descriptions.size(); ++index)
			m_pipelines[index + 4] = m_shaders.Create_Pipeline(device, m_shader, point_sprite_descriptions[index]);
		m_pipeline = m_pipelines[0];
		bool pipelines_valid = true;
		for (const PipelineHandle pipeline : m_pipelines)
			pipelines_valid = pipelines_valid && pipeline.Is_Valid();
		if (!pipelines_valid) {
			Shutdown();
			return false;
		}

		constexpr std::array<ParticleVertex, 6> billboard = {{
			{{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
			{{-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
			{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
			{{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
			{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
			{{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
		}};
		m_billboard_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(billboard)), RHIBufferUsage::Vertex, sizeof(ParticleVertex)},
			std::as_bytes(std::span<const ParticleVertex>(billboard)));
		m_particle_buffer = device.Create_Buffer({
			static_cast<std::uint32_t>(max_particles * sizeof(GPUParticleData)),
			RHIBufferUsage::Storage,
			static_cast<std::uint32_t>(sizeof(GPUParticleData))
		});
		m_material_constants = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(material.parameters.Bytes().size()), RHIBufferUsage::Constant, 16},
			material.parameters.Bytes());
		if (!m_billboard_buffer.Is_Valid() || !m_particle_buffer.Is_Valid() || !m_material_constants.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_bindless.Reserve(2, 1, 126, 0, 1);
		if (!m_bindless.Register_Buffer(m_particle_buffer).Is_Valid() || !m_bindless.Register_Material(m_material, m_material_constants).Is_Valid()) {
			Shutdown();
			return false;
		}

		m_gpu_scene.Reserve(0, 0, 1);
		if (!m_gpu_scene.Build(m_empty_scene, m_meshes, m_textures, m_samplers, m_materials)) {
			Shutdown();
			return false;
		}

		m_residency = std::make_unique<GPUResourceResidency>(device);
		m_max_particles = max_particles;
		return true;
	}

	void Shutdown() noexcept
	{
		m_residency.reset();
		if (m_device != nullptr) {
			if (m_billboard_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_billboard_buffer);
			if (m_material_constants.Is_Valid())
				m_device->Destroy_Buffer(m_material_constants);
			if (m_particle_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_particle_buffer);
			for (const PipelineHandle pipeline : m_pipelines)
				if (pipeline.Is_Valid())
					m_device->Destroy_Pipeline(pipeline);
		}

		m_bindless.Clear();
		if (m_material.Is_Valid())
			m_materials.Destroy(m_material);
		m_textures.Clear();
		m_materials.Clear();
		m_samplers.Clear();
		m_shaders.Destroy(m_shader);
		m_shader = {};
		m_material = {};
		m_pipeline = {};
		m_pipelines.fill({});
		m_billboard_buffer = {};
		m_material_constants = {};
		m_particle_buffer = {};
		m_gpu_particles.clear();
		m_visible_storage.clear();
		m_draw_storage.clear();
		m_particles.Clear();
		m_gpu_scene = {};
		m_plan = {};
		m_graph.reset();
		m_color_resource = {};
		m_depth_resource = {};
		m_pass = {};
		m_max_particles = 0;
		m_device = nullptr;
	}

	bool Is_Initialized() const noexcept
	{
		return m_device != nullptr && m_pipeline.Is_Valid() && m_particle_buffer.Is_Valid();
	}

	ParticleEmitterHandle Create_Emitter(const ParticleEmitter &emitter = {})
	{
		return Is_Initialized() ? m_particles.Create_Emitter(emitter) : ParticleEmitterHandle{};
	}

	bool Update_Emitter(ParticleEmitterHandle handle, const ParticleEmitter &emitter) noexcept
	{
		return Is_Initialized() && m_particles.Update_Emitter(handle, emitter);
	}

	bool Destroy_Emitter(ParticleEmitterHandle handle) noexcept
	{
		return Is_Initialized() && m_particles.Destroy_Emitter(handle);
	}

	void Reset_Particles() noexcept
	{
		m_particles.Clear_Particles();
	}

	bool Append_Particles(ParticleEmitterHandle handle, const ParticleData &data) noexcept
	{
		return Is_Initialized() && m_particles.Append_Particles(handle, data);
	}

	bool Spawn(ParticleEmitterHandle handle, std::uint32_t count) noexcept
	{
		return Is_Initialized() && m_particles.Spawn(handle, count);
	}

	bool Update(float delta_seconds) noexcept
	{
		return Is_Initialized() && m_particles.Update(delta_seconds);
	}

	std::size_t Particle_Count() const noexcept
	{
		return m_particles.Particle_Count();
	}

	std::size_t Visible_Particle_Count() const noexcept
	{
		return m_visible_set.Size();
	}

	std::size_t Draw_Count() const noexcept
	{
		return m_draw_set.Size();
	}

	MaterialHandle Default_Material() const noexcept
	{
		return m_material;
	}

	ShaderHandle Particle_Shader() const noexcept
	{
		return m_shader;
	}

	PipelineHandle Pipeline_For_Flags(ParticleEmitterFlags flags) const noexcept
	{
		std::size_t index = 0;
		if (Has_Particle_Emitter_Flag(flags, ParticleEmitterFlags::Additive))
			index = 1;
		else if (Has_Particle_Emitter_Flag(flags, ParticleEmitterFlags::Multiply))
			index = 2;
		else if (Has_Particle_Emitter_Flag(flags, ParticleEmitterFlags::AlphaTest))
			index = 3;
		if (Has_Particle_Emitter_Flag(flags, ParticleEmitterFlags::PointSprite))
			index += 4;
		return m_pipelines[index];
	}

	TextureHandle Create_Texture(const Texture &description, std::span<const std::byte> initial_data)
	{
		if (!Is_Initialized() || initial_data.empty() || !Has_Texture_Usage(description.usage, TextureUsage::Sampled))
			return {};

		Texture stored = description;
		stored.pixel_data = {};
		const TextureHandle handle = m_textures.Create(stored);
		if (!handle.Is_Valid())
			return {};

		Texture upload = description;
		upload.pixel_data = initial_data;
		if (m_residency == nullptr || !m_residency->Upload_Texture(handle, upload)) {
			m_textures.Destroy(handle);
			return {};
		}

		const GPUResidentTexture resident = m_residency->Texture_Info(handle);
		if (!resident.texture.Is_Valid()) {
			m_residency->Destroy_Texture(handle);
			m_textures.Destroy(handle);
			return {};
		}
		return handle;
	}

	MaterialHandle Create_Material(const Material &description)
	{
		if (!Is_Initialized() || m_residency == nullptr)
			return {};

		const MaterialHandle handle = m_materials.Create(description);
		if (!handle.Is_Valid() || !m_residency->Upload_Material(handle, description)) {
			m_materials.Destroy(handle);
			return {};
		}

		const GPUResidentMaterial resident = m_residency->Material_Info(handle);
		if (!resident.constants.Is_Valid() || !Rebuild_GPU_Scene()) {
			m_residency->Destroy_Material(handle);
			m_materials.Destroy(handle);
			return {};
		}
		return handle;
	}

	bool Destroy_Texture(TextureHandle handle) noexcept
	{
		if (m_textures.Resolve(handle) == nullptr || m_residency == nullptr)
			return false;
		m_residency->Destroy_Texture(handle);
		return m_textures.Destroy(handle);
	}

	bool Destroy_Material(MaterialHandle handle) noexcept
	{
		if (handle == m_material || m_materials.Resolve(handle) == nullptr || m_residency == nullptr)
			return false;
		m_residency->Destroy_Material(handle);
		return m_materials.Destroy(handle) && Rebuild_GPU_Scene();
	}

	bool Set_View(const View &view) noexcept
	{
		if (!Is_Initialized())
			return false;
		m_view = view;
		return true;
	}

	bool Render(CommandList &commands, RHITextureHandle color_target, RHITextureHandle depth_target, RHIViewport viewport) noexcept
	{
		GRAPHICS_PROFILE_SCOPE("Graphics::ParticleRenderer::Render");
		if (!Is_Initialized() || !color_target.Is_Valid() || !depth_target.Is_Valid() || viewport.width == 0 || viewport.height == 0)
			return false;

		const ParticleData particles = m_particles.Particles();
		m_visible_set = VisibleParticleSet(m_visible_storage);
		if (!Build_Visible_Particles(m_particles, m_view, m_visible_set))
			return false;

		m_draw_set = ParticleDrawSet(m_draw_storage);
		if (!Build_Particle_Draw_Data(m_visible_set, m_particles, m_view, m_gpu_scene, {40, m_pipeline, 0}, m_draw_set))
			return false;

		if (m_draw_set.Size() > m_max_particles)
			return false;
		const std::span<const ParticleDrawData> draws = m_draw_set.Records();
        const ParticleFrameParameters frame_constants{m_view.view_matrix.values,m_view.projection_matrix.values};
		std::array<float, MaterialParameterBlock::ValueCount> material_values{};
		material_values[0] = 1.0f;
		material_values[1] = 1.0f;
		material_values[2] = 1.0f;
		material_values[3] = 1.0f;
		material_values[4] = static_cast<float>(viewport.width);
		material_values[5] = static_cast<float>(viewport.height);
        material_values[6] = static_cast<float>(viewport.x);
        material_values[7] = static_cast<float>(viewport.y);
		if (!m_device->Update_Buffer(m_material_constants, 0,
			std::as_bytes(std::span<const float>(material_values))))
			return false;

		m_bindings[0] = GraphResourceBinding::Texture(m_color_resource, color_target);
		m_bindings[1] = GraphResourceBinding::Texture(m_depth_resource, depth_target);
		if (!m_plan.Is_Valid() && !m_plan.Compile(*m_graph, m_bindings))
			return false;

		// Texture residency is independent of the number of slots in a draw.
        // Split only at page boundaries, retaining the sorted particle order.
        for (std::size_t first = 0; first < draws.size();) {
            m_bindless.Clear();
            if (!m_bindless.Register_Buffer(m_particle_buffer).Is_Valid()
                || !m_bindless.Register_Material(m_material, m_material_constants).Is_Valid()) return false;
            std::size_t count = 0;
            std::size_t texture_count = 0;
            while (first + count < draws.size()) {
                const auto& draw = draws[first + count];
                const Material* material = m_materials.Resolve(particles.materials[draw.particle_index]);
                if (material == nullptr) return false;
                const TextureHandle texture = material->textures[0];
                if (texture.Is_Valid() && !m_bindless.Texture_Index(texture).Is_Valid()) {
                    if (texture_count == 126) break;
                    const auto resident = m_residency->Texture_Info(texture);
                    if (!resident.texture.Is_Valid()
                        || !m_bindless.Register_Texture(texture, resident.texture).Is_Valid()) return false;
                    ++texture_count;
                }
                m_gpu_particles[count] = Pack_Draw_Particle(particles, draw.particle_index, draw.material_index);
                ++count;
            }
            if (!m_device->Update_Buffer(m_particle_buffer, 0,
                std::as_bytes(std::span<const GPUParticleData>(m_gpu_particles.data(), count)))) return false;
            const ParticlePassInput input{
                draws.subspan(first, count),
                {m_billboard_buffer, sizeof(ParticleVertex), 6},
                m_bindless.Resources(), m_color_resource, m_depth_resource, viewport, frame_constants
            };
            if (!m_plan.Execute(*m_graph, commands,
                [&](GraphPassHandle pass, CommandList& command_list, const PassResources& resources) noexcept {
                    return pass == m_pass
                        && ParticlePass::Execute(command_list, resources, input);
                })) return false;
            first += count;
        }
        return true;
	}

	bool Render(RHITextureHandle color_target, RHITextureHandle depth_target, RHIViewport viewport) noexcept
	{
		return m_device != nullptr && Render(m_device->Immediate_Command_List(), color_target, depth_target, viewport);
	}

private:
    GPUParticleData Pack_Draw_Particle(const ParticleData& particles, std::size_t index,
        std::uint32_t material_index) const noexcept
    {
        GPUParticleData data = Pack_GPU_Particle(particles,index,material_index);
        if (const Material* material = m_materials.Resolve(particles.materials[index]);
            material != nullptr && material->textures[0].Is_Valid()) {
            const ResourceIndex texture_index = m_bindless.Texture_Index(material->textures[0]);
            if (texture_index.Is_Valid()) data.texture_index = texture_index.Get_Index();
        }
        return data;
    }

	bool Rebuild_GPU_Scene() noexcept
	{
		return m_gpu_scene.Build(m_empty_scene, m_meshes, m_textures, m_samplers, m_materials);
	}

	Device *m_device = nullptr;
	std::size_t m_max_particles = 0;
	ParticleSystem m_particles;
	MeshPool m_meshes;
	TexturePool m_textures;
	SamplerPool m_samplers;
	MaterialPool m_materials;
	std::unique_ptr<GPUResourceResidency> m_residency;
	RenderScene m_empty_scene;
	GPUScene m_gpu_scene;
	View m_view{};
	ShaderLibrary m_shaders;
	ShaderHandle m_shader{};
	MaterialHandle m_material{};
	PipelineHandle m_pipeline{};
	std::array<PipelineHandle, 8> m_pipelines{};
	RHIBufferHandle m_billboard_buffer{};
	RHIBufferHandle m_particle_buffer{};
	RHIBufferHandle m_material_constants{};
	BindlessResourceTable m_bindless;
	AlignedVector<GPUParticleData> m_gpu_particles;
	std::vector<std::uint32_t> m_visible_storage;
	AlignedVector<ParticleDrawData> m_draw_storage;
	VisibleParticleSet m_visible_set{std::span<std::uint32_t>{}};
	ParticleDrawSet m_draw_set{std::span<ParticleDrawData>{}};
	std::unique_ptr<RenderGraph> m_graph;
	ExecutionPlan m_plan;
	std::array<GraphResourceBinding, 2> m_bindings{};
	GraphResourceHandle m_color_resource{};
	GraphResourceHandle m_depth_resource{};
	GraphPassHandle m_pass{};
};

namespace
{
ParticleRenderer g_particle_renderer;
}

export ParticleRenderer &GetParticleRenderer() noexcept
{
	return g_particle_renderer;
}

export ParticleEmitterHandle CreateParticleEmitter(const ParticleEmitter &emitter)
{
	return g_particle_renderer.Create_Emitter(emitter);
}

export bool UpdateParticleEmitter(ParticleEmitterHandle handle, const ParticleEmitter &emitter) noexcept
{
	return g_particle_renderer.Update_Emitter(handle, emitter);
}

export void DestroyParticleEmitter(ParticleEmitterHandle handle) noexcept
{
	g_particle_renderer.Destroy_Emitter(handle);
}

export bool InitializeParticles(Device &device, const std::filesystem::path &shader_directory, std::size_t max_emitters, std::size_t max_particles)
{
	return g_particle_renderer.Initialize(device, shader_directory, max_emitters, max_particles);
}

export void ShutdownParticles() noexcept
{
	g_particle_renderer.Shutdown();
}

}
