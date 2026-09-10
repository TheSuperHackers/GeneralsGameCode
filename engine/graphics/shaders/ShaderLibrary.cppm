module;

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

export module Graphics.Shaders.Library;

export import Graphics.Resources.Materials.Material;
export import Graphics.Resources.Pools.ResourcePool;
export import Graphics.Shaders.Pipeline;

namespace Graphics
{

export enum class ShaderStage : std::uint8_t
{
	Vertex,
	Pixel,
	Compute
};

export enum class ShaderStageMask : std::uint8_t
{
	None = 0,
	Vertex = 1u << 0,
	Pixel = 1u << 1,
	Compute = 1u << 2
};

export constexpr ShaderStageMask operator|(ShaderStageMask left, ShaderStageMask right) noexcept
{
	return static_cast<ShaderStageMask>(static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
}

export constexpr bool Has_Shader_Stage(ShaderStageMask stages, ShaderStageMask stage) noexcept
{
	return (static_cast<std::uint8_t>(stages) & static_cast<std::uint8_t>(stage)) != 0;
}

export struct ShaderProgramDesc final
{
	std::uint16_t vertex_shader = 0;
	std::uint16_t fragment_shader = 0;
	std::uint16_t compute_shader = 0;
	ShaderStageMask stages = ShaderStageMask::None;
	ShaderInterfaceLayout interface_layout{};
	std::uint64_t source_key = 0;

	constexpr bool Has_Stage(ShaderStage stage) const noexcept
	{
		switch (stage) {
		case ShaderStage::Vertex:
			return Has_Shader_Stage(stages, ShaderStageMask::Vertex);
		case ShaderStage::Pixel:
			return Has_Shader_Stage(stages, ShaderStageMask::Pixel);
		case ShaderStage::Compute:
			return Has_Shader_Stage(stages, ShaderStageMask::Compute);
		}

		return false;
	}
};

export struct ShaderPrecompiledDesc final
{
	ShaderProgramDesc program{};
	std::filesystem::path vertex_path;
	std::filesystem::path fragment_path;
	std::filesystem::path compute_path;
};

export struct ShaderProgram final
{
	ShaderProgramDesc description{};
	std::vector<std::byte> vertex_bytecode;
	std::vector<std::byte> fragment_bytecode;
	std::vector<std::byte> compute_bytecode;

	bool Is_Graphics_Program() const noexcept
	{
		return description.Has_Stage(ShaderStage::Vertex) && description.Has_Stage(ShaderStage::Pixel) && !vertex_bytecode.empty() && !fragment_bytecode.empty();
	}
};

static_assert(std::is_nothrow_move_constructible_v<ShaderProgram>);
static_assert(std::is_nothrow_move_assignable_v<ShaderProgram>);

export ShaderInterfaceLayout Make_Basic_Opaque_Interface() noexcept
{
	ShaderInterfaceLayout layout;
	for (std::size_t index = 0; index < 4; ++index)
		layout.material.Add_Constant(ShaderValueType::Float4);
	return layout;
}

export ShaderInterfaceLayout Make_Particle_Billboard_Interface() noexcept
{
	ShaderInterfaceLayout layout;
	for (std::size_t index = 0; index < 4; ++index)
		layout.material.Add_Constant(ShaderValueType::Float4);
	return layout;
}

export ShaderInterfaceLayout Make_Screen_Distortion_Interface() noexcept
{
	return {};
}

export ShaderInterfaceLayout Make_Beam_Interface() noexcept
{
	return {};
}

export ShaderInterfaceLayout Make_Video_Interface() noexcept
{
	ShaderInterfaceLayout layout;
	for (std::size_t index = 0; index < 4; ++index)
		layout.material.Add_Constant(ShaderValueType::Float4);
	return layout;
}

export ShaderInterfaceLayout Make_Ring_Interface() noexcept
{
	ShaderInterfaceLayout layout;
	for (std::size_t index = 0; index < 4; ++index)
		layout.material.Add_Constant(ShaderValueType::Float4);
	return layout;
}

export ShaderInterfaceLayout Make_Fullscreen_Overlay_Interface() noexcept
{
	ShaderInterfaceLayout layout;
	for (std::size_t index = 0; index < 4; ++index)
		layout.material.Add_Constant(ShaderValueType::Float4);
	return layout;
}

export ShaderInterfaceLayout Make_World_Quad_Interface() noexcept
{
	ShaderInterfaceLayout layout;
	for (std::size_t index = 0; index < 4; ++index)
		layout.material.Add_Constant(ShaderValueType::Float4);
	return layout;
}

export ShaderProgramDesc Make_Basic_Opaque_Description() noexcept
{
	ShaderProgramDesc description;
	description.vertex_shader = 1;
	description.fragment_shader = 1;
	description.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
	description.interface_layout = Make_Basic_Opaque_Interface();
	description.source_key = 0x42415349434f5041ull;
	return description;
}

export ShaderProgramDesc Make_Skinned_Basic_Opaque_Description() noexcept
{
	ShaderProgramDesc description;
	description.vertex_shader = 5;
	description.fragment_shader = 1;
	description.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
	description.interface_layout = Make_Basic_Opaque_Interface();
	description.source_key = 0x534b494e4f504151ull;
	return description;
}

export ShaderProgramDesc Make_Particle_Billboard_Description() noexcept
{
	ShaderProgramDesc description;
	description.vertex_shader = 2;
	description.fragment_shader = 2;
	description.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
	description.interface_layout = Make_Particle_Billboard_Interface();
	description.source_key = 0x5041525449434c45ull;
	return description;
}

export ShaderProgramDesc Make_Screen_Distortion_Description() noexcept
{
	ShaderProgramDesc description;
	description.vertex_shader = 3;
	description.fragment_shader = 3;
	description.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
	description.interface_layout = Make_Screen_Distortion_Interface();
	description.source_key = 0x5343524e44495354ull;
	return description;
}

export ShaderProgramDesc Make_Beam_Description() noexcept
{
	ShaderProgramDesc description;
	description.vertex_shader = 4;
	description.fragment_shader = 4;
	description.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
	description.interface_layout = Make_Beam_Interface();
	description.source_key = 0x4245414d5052494dull;
	return description;
}

export ShaderProgramDesc Make_Video_Description() noexcept
{
	ShaderProgramDesc description;
	description.vertex_shader = 6;
	description.fragment_shader = 6;
	description.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
	description.interface_layout = Make_Video_Interface();
	description.source_key = 0x564944454f505249ull;
	return description;
}

export ShaderProgramDesc Make_Ring_Description() noexcept
{
	ShaderProgramDesc description;
	description.vertex_shader = 8;
	description.fragment_shader = 8;
	description.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
	description.interface_layout = Make_Ring_Interface();
	description.source_key = 0x52494e475052494dull;
	return description;
}

export ShaderProgramDesc Make_Fullscreen_Overlay_Description() noexcept
{
	ShaderProgramDesc description;
	description.vertex_shader = 9;
	description.fragment_shader = 9;
	description.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
	description.interface_layout = Make_Fullscreen_Overlay_Interface();
	description.source_key = 0x46534f5645524c59ull;
	return description;
}

export ShaderProgramDesc Make_World_Quad_Description() noexcept
{
	ShaderProgramDesc description;
	description.vertex_shader = 10;
	description.fragment_shader = 10;
	description.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
	description.interface_layout = Make_World_Quad_Interface();
	description.source_key = 0x574f524c44515544ull;
	return description;
}

export PipelineDesc Make_Basic_Opaque_Pipeline() noexcept
{
	const ShaderProgramDesc shader = Make_Basic_Opaque_Description();
	PipelineDesc description;
	description.vertex_shader = shader.vertex_shader;
	description.fragment_shader = shader.fragment_shader;
	description.Set_Parameter_Layout(shader.interface_layout);
	return description;
}

export PipelineDesc Make_Skinned_Basic_Opaque_Pipeline() noexcept
{
	PipelineDesc description = Make_Basic_Opaque_Pipeline();
	description.vertex_shader = 5;
	description.vertex_format = RHIVertexFormat::Position3Color4UV2Skinned;
	return description;
}

export PipelineDesc Make_Particle_Billboard_Pipeline() noexcept
{
	const ShaderProgramDesc shader = Make_Particle_Billboard_Description();
	PipelineDesc description;
	description.vertex_shader = shader.vertex_shader;
	description.fragment_shader = shader.fragment_shader;
	description.depth_test = true;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Alpha;
	description.cull_mode = RHICullMode::None;
	description.Set_Parameter_Layout(shader.interface_layout);
	return description;
}

export PipelineDesc Make_Screen_Distortion_Pipeline() noexcept
{
	const ShaderProgramDesc shader = Make_Screen_Distortion_Description();
	PipelineDesc description;
	description.vertex_shader = shader.vertex_shader;
	description.fragment_shader = shader.fragment_shader;
	description.depth_test = true;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Alpha;
	description.cull_mode = RHICullMode::None;
	description.Set_Parameter_Layout(shader.interface_layout);
	return description;
}

export PipelineDesc Make_Beam_Pipeline() noexcept
{
	const ShaderProgramDesc shader = Make_Beam_Description();
	PipelineDesc description;
	description.vertex_shader = shader.vertex_shader;
	description.fragment_shader = shader.fragment_shader;
	description.vertex_format = RHIVertexFormat::Position3Color4UV2ResourceIndex;
	description.depth_test = true;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Alpha;
	description.cull_mode = RHICullMode::None;
	description.Set_Parameter_Layout(shader.interface_layout);
	return description;
}

export PipelineDesc Make_Video_Pipeline() noexcept
{
	const ShaderProgramDesc shader = Make_Video_Description();
	PipelineDesc description;
	description.vertex_shader = shader.vertex_shader;
	description.fragment_shader = shader.fragment_shader;
	description.depth_test = false;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Alpha;
	description.cull_mode = RHICullMode::None;
	description.Set_Parameter_Layout(shader.interface_layout);
	return description;
}

export PipelineDesc Make_Ring_Pipeline() noexcept
{
	const ShaderProgramDesc shader = Make_Ring_Description();
	PipelineDesc description;
	description.vertex_shader = shader.vertex_shader;
	description.fragment_shader = shader.fragment_shader;
	description.depth_test = false;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Alpha;
	description.cull_mode = RHICullMode::None;
	description.Set_Parameter_Layout(shader.interface_layout);
	return description;
}

export PipelineDesc Make_Fullscreen_Overlay_Pipeline() noexcept
{
	const ShaderProgramDesc shader = Make_Fullscreen_Overlay_Description();
	PipelineDesc description;
	description.vertex_shader = shader.vertex_shader;
	description.fragment_shader = shader.fragment_shader;
	description.vertex_format = RHIVertexFormat::Position3Color4UV2;
	description.depth_test = false;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Additive;
	description.cull_mode = RHICullMode::None;
	description.Set_Parameter_Layout(shader.interface_layout);
	return description;
}

export PipelineDesc Make_World_Quad_Pipeline() noexcept
{
	const ShaderProgramDesc shader = Make_World_Quad_Description();
	PipelineDesc description;
	description.vertex_shader = shader.vertex_shader;
	description.fragment_shader = shader.fragment_shader;
	description.vertex_format = RHIVertexFormat::Position3Color4UV2;
	description.depth_test = true;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Alpha;
	description.cull_mode = RHICullMode::None;
	description.Set_Parameter_Layout(shader.interface_layout);
	return description;
}

export class ShaderLibrary final
{
public:
	ShaderHandle Load_Precompiled(const ShaderPrecompiledDesc &description)
	{
		if (!Is_Valid_Graphics_Description(description) || Find_By_Source_Key(description.program.source_key) != nullptr)
			return {};

		std::vector<std::byte> vertex_bytecode;
		std::vector<std::byte> fragment_bytecode;
		if (!Load_Binary(description.vertex_path, vertex_bytecode) || !Load_Binary(description.fragment_path, fragment_bytecode))
			return {};

		ShaderProgram program;
		program.description = description.program;
		program.vertex_bytecode = std::move(vertex_bytecode);
		program.fragment_bytecode = std::move(fragment_bytecode);
		return m_programs.Create(std::move(program));
	}

	ShaderHandle Load_Basic_Opaque(const std::filesystem::path &directory)
	{
		if (m_basic_opaque.Is_Valid())
			return m_basic_opaque;

		ShaderPrecompiledDesc description;
		description.program = Make_Basic_Opaque_Description();
		description.vertex_path = directory / "basic_opaque.vso";
		description.fragment_path = directory / "basic_opaque.pso";
		m_basic_opaque = Load_Precompiled(description);
		return m_basic_opaque;
	}

	ShaderHandle Load_Skinned_Basic_Opaque(const std::filesystem::path &directory)
	{
		if (m_skinned_basic_opaque.Is_Valid())
			return m_skinned_basic_opaque;

		ShaderPrecompiledDesc description;
		description.program = Make_Skinned_Basic_Opaque_Description();
		description.vertex_path = directory / "basic_opaque_skinned.vso";
		description.fragment_path = directory / "basic_opaque.pso";
		m_skinned_basic_opaque = Load_Precompiled(description);
		return m_skinned_basic_opaque;
	}

	ShaderHandle Load_Particle_Billboard(const std::filesystem::path &directory)
	{
		if (m_particle_billboard.Is_Valid())
			return m_particle_billboard;

		ShaderPrecompiledDesc description;
		description.program = Make_Particle_Billboard_Description();
		description.vertex_path = directory / "particle_billboard.vso";
		description.fragment_path = directory / "particle_billboard.pso";
		m_particle_billboard = Load_Precompiled(description);
		return m_particle_billboard;
	}

	ShaderHandle Load_Screen_Distortion(const std::filesystem::path &directory)
	{
		if (m_screen_distortion.Is_Valid())
			return m_screen_distortion;

		ShaderPrecompiledDesc description;
		description.program = Make_Screen_Distortion_Description();
		description.vertex_path = directory / "screen_distortion.vso";
		description.fragment_path = directory / "screen_distortion.pso";
		m_screen_distortion = Load_Precompiled(description);
		return m_screen_distortion;
	}

	ShaderHandle Load_Beam(const std::filesystem::path &directory)
	{
		if (m_beam.Is_Valid())
			return m_beam;

		ShaderPrecompiledDesc description;
		description.program = Make_Beam_Description();
		description.vertex_path = directory / "beam.vso";
		description.fragment_path = directory / "beam.pso";
		m_beam = Load_Precompiled(description);
		return m_beam;
	}

	ShaderHandle Load_Video(const std::filesystem::path &directory)
	{
		if (m_video.Is_Valid())
			return m_video;

		ShaderPrecompiledDesc description;
		description.program = Make_Video_Description();
		description.vertex_path = directory / "video.vso";
		description.fragment_path = directory / "video.pso";
		m_video = Load_Precompiled(description);
		return m_video;
	}

	ShaderHandle Load_Ring(const std::filesystem::path &directory)
	{
		if (m_ring.Is_Valid())
			return m_ring;

		ShaderPrecompiledDesc description;
		description.program = Make_Ring_Description();
		description.vertex_path = directory / "ring.vso";
		description.fragment_path = directory / "ring.pso";
		m_ring = Load_Precompiled(description);
		return m_ring;
	}

	ShaderHandle Load_Fullscreen_Overlay(const std::filesystem::path &directory)
	{
		if (m_fullscreen_overlay.Is_Valid())
			return m_fullscreen_overlay;

		ShaderPrecompiledDesc description;
		description.program = Make_Fullscreen_Overlay_Description();
		description.vertex_path = directory / "fullscreen_overlay.vso";
		description.fragment_path = directory / "fullscreen_overlay.pso";
		m_fullscreen_overlay = Load_Precompiled(description);
		return m_fullscreen_overlay;
	}

	ShaderHandle Load_World_Quad(const std::filesystem::path &directory)
	{
		if (m_world_quad.Is_Valid())
			return m_world_quad;

		ShaderPrecompiledDesc description;
		description.program = Make_World_Quad_Description();
		description.vertex_path = directory / "world_quad.vso";
		description.fragment_path = directory / "world_quad.pso";
		m_world_quad = Load_Precompiled(description);
		return m_world_quad;
	}

	bool Destroy(ShaderHandle handle) noexcept
	{
		if (handle == m_basic_opaque)
			m_basic_opaque = {};
		if (handle == m_skinned_basic_opaque)
			m_skinned_basic_opaque = {};
		if (handle == m_particle_billboard)
			m_particle_billboard = {};
		if (handle == m_screen_distortion)
			m_screen_distortion = {};
		if (handle == m_beam)
			m_beam = {};
		if (handle == m_video)
			m_video = {};
		if (handle == m_ring)
			m_ring = {};
		if (handle == m_fullscreen_overlay)
			m_fullscreen_overlay = {};
		if (handle == m_world_quad)
			m_world_quad = {};
		return m_programs.Destroy(handle);
	}

	bool Is_Loaded(ShaderHandle handle) const noexcept
	{
		return m_programs.Resolve(handle) != nullptr;
	}

	ShaderHandle Basic_Opaque() const noexcept
	{
		return m_basic_opaque;
	}

	ShaderHandle Skinned_Basic_Opaque() const noexcept
	{
		return m_skinned_basic_opaque;
	}

	ShaderHandle Screen_Distortion() const noexcept
	{
		return m_screen_distortion;
	}

	ShaderHandle Beam() const noexcept
	{
		return m_beam;
	}

	ShaderHandle Video() const noexcept
	{
		return m_video;
	}

	ShaderHandle Ring() const noexcept
	{
		return m_ring;
	}

	ShaderHandle Fullscreen_Overlay() const noexcept
	{
		return m_fullscreen_overlay;
	}

	ShaderHandle World_Quad() const noexcept
	{
		return m_world_quad;
	}

	ShaderHandle Select_Shader(const Material &material, ShaderHandle default_shader) const noexcept
	{
		if (Is_Loaded(material.shader))
			return material.shader;

		return Is_Loaded(default_shader) ? default_shader : ShaderHandle{};
	}

    std::span<const std::byte> Bytecode(ShaderHandle handle, ShaderStage stage) const noexcept
    {
        const ShaderProgram *program = m_programs.Resolve(handle);
        if (program == nullptr) return {};
        switch (stage) {
        case ShaderStage::Vertex: return program->vertex_bytecode;
        case ShaderStage::Pixel: return program->fragment_bytecode;
        case ShaderStage::Compute: return program->compute_bytecode;
        }
        return {};
    }

	ShaderProgramDesc Description(ShaderHandle handle) const noexcept
	{
		const ShaderProgram *program = m_programs.Resolve(handle);
		return program != nullptr ? program->description : ShaderProgramDesc{};
	}

	ShaderInterfaceLayout Interface(ShaderHandle handle) const noexcept
	{
		return Description(handle).interface_layout;
	}

	PipelineDesc Make_Pipeline_Description(ShaderHandle shader, PipelineDesc state) const noexcept
	{
		const ShaderProgramDesc program = Description(shader);
		if (!Is_Loaded(shader) || !program.Has_Stage(ShaderStage::Vertex) || !program.Has_Stage(ShaderStage::Pixel))
			return {};

		state.vertex_shader = program.vertex_shader;
		state.fragment_shader = program.fragment_shader;
		state.Set_Parameter_Layout(program.interface_layout);
		return state;
	}

	PipelineHandle Create_Pipeline(Device &device, ShaderHandle shader, const PipelineDesc &description) const
	{
		const ShaderProgram *program = m_programs.Resolve(shader);
		if (program == nullptr || !program->Is_Graphics_Program() || description.vertex_shader != program->description.vertex_shader || description.fragment_shader != program->description.fragment_shader || description.parameter_layout_key != program->description.interface_layout.Key())
			return {};

		const PipelineKey key = description.Key();
		const RHIPipeline rhi_description{
			key.value,
			description.depth_test,
			description.depth_write,
			 description.topology,
				description.vertex_format,
				description.blend_mode,
				description.cull_mode,
				description.blend_operation
		};
		return device.Create_Pipeline(rhi_description, {program->vertex_bytecode}, {program->fragment_bytecode});
	}

	PipelineHandle Create_Pipeline(Device &device, const Material &material, ShaderHandle default_shader, PipelineDesc state) const
	{
		const ShaderHandle shader = Select_Shader(material, default_shader);
		const PipelineDesc description = Make_Pipeline_Description(shader, state);
		return Create_Pipeline(device, shader, description);
	}

	std::size_t Size() const noexcept
	{
		return m_programs.Size();
	}

private:
	static bool Is_Valid_Graphics_Description(const ShaderPrecompiledDesc &description) noexcept
	{
		const ShaderProgramDesc &program = description.program;
		return program.Has_Stage(ShaderStage::Vertex) && program.Has_Stage(ShaderStage::Pixel) && program.vertex_shader != 0 && program.fragment_shader != 0 && program.source_key != 0 && !description.vertex_path.empty() && !description.fragment_path.empty();
	}

	static bool Load_Binary(const std::filesystem::path &path, std::vector<std::byte> &data)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)
			return false;

		const std::streampos end = file.tellg();
		if (end <= 0)
			return false;

		std::vector<std::byte> loaded(static_cast<std::size_t>(end));
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char *>(loaded.data()), static_cast<std::streamsize>(loaded.size()));
		if (!file.good() && !file.eof())
			return false;

		data = std::move(loaded);
		return true;
	}

	const ShaderProgram *Find_By_Source_Key(std::uint64_t source_key) const noexcept
	{
		if (source_key == 0)
			return nullptr;

		const ShaderProgram *found = nullptr;
		m_programs.For_Each([&found, source_key](ShaderHandle, const ShaderProgram &program) noexcept {
			if (program.description.source_key == source_key)
				found = &program;
		});
		return found;
	}

	ResourcePool<ShaderProgram, ShaderHandle> m_programs;
	ShaderHandle m_basic_opaque{};
	ShaderHandle m_skinned_basic_opaque{};
	ShaderHandle m_particle_billboard{};
	ShaderHandle m_screen_distortion{};
	ShaderHandle m_beam{};
	ShaderHandle m_video{};
	ShaderHandle m_ring{};
	ShaderHandle m_fullscreen_overlay{};
	ShaderHandle m_world_quad{};
};

}
