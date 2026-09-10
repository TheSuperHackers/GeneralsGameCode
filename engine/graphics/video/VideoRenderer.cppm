module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <limits>
#include <memory>
#include <span>
#include <vector>

export module Graphics.Video.Renderer;

export import Graphics.Passes.Video;
export import Graphics.Resources.Materials.Material;
export import Graphics.Resources.Residency.GPUResourceResidency;
export import Graphics.Resources.Textures.Texture;
export import Graphics.FrameTargets;
export import Graphics.Shaders.Library;

export import Video.Presentation;

namespace Graphics
{

export class VideoRenderer final : public Engine::Video::FramePresenter
{
public:
	static constexpr std::size_t Max_Presentations = 32;

	VideoRenderer() noexcept = default;
	~VideoRenderer() noexcept
	{
		Shutdown();
	}

	VideoRenderer(const VideoRenderer &) = delete;
	VideoRenderer &operator=(const VideoRenderer &) = delete;

	bool Initialize(Device &device, const std::filesystem::path &shader_directory)
	{
		if (m_initialized)
			return false;

		m_device = &device;
		m_residency.Set_Device(device);
		m_textures.Reserve(Max_Presentations);
		m_materials.Reserve(Max_Presentations);

		m_shader = m_shaders.Load_Video(shader_directory);
		if (!m_shader.Is_Valid()) {
			m_device = nullptr;
			return false;
		}

		m_pipeline = m_shaders.Create_Pipeline(device, m_shader, Make_Video_Pipeline());
		if (!m_pipeline.Is_Valid()) {
			m_shaders.Destroy(m_shader);
			m_shader = {};
			m_device = nullptr;
			return false;
		}

		constexpr std::array<VideoVertex, 4> vertices = {{
			{{-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
			{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
			{{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
			{{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		}};
		constexpr std::array<std::uint16_t, 6> indices = {0, 1, 2, 0, 2, 3};
		m_vertex_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(vertices)), RHIBufferUsage::Vertex, sizeof(VideoVertex)},
			std::as_bytes(std::span<const VideoVertex>(vertices)));
		m_index_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(indices)), RHIBufferUsage::Index, 0},
			std::as_bytes(std::span<const std::uint16_t>(indices)));
		if (!m_vertex_buffer.Is_Valid() || !m_index_buffer.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_graph.Reserve(1, 1, 1);
		m_color_resource = m_graph.Create_Resource({GraphResourceKind::Texture});
		m_pass = VideoPass::Add_To_Graph(m_graph, m_color_resource, 0xffffffffu);
		m_initialized = m_color_resource.Is_Valid() && m_pass.Is_Valid();
		if (!m_initialized)
			Shutdown();
		return m_initialized;
	}

	void Shutdown() noexcept
	{
		m_accepting = false;
		m_presentation_count = 0;
		if (m_device != nullptr) {
			m_residency.Clear();
			for (Presentation &presentation : m_presentations) {
				if (presentation.material.Is_Valid())
					m_materials.Destroy(presentation.material);
				if (presentation.texture.Is_Valid())
					m_textures.Destroy(presentation.texture);
				presentation = {};
			}
			if (m_vertex_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_vertex_buffer);
			if (m_index_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_index_buffer);
			if (m_pipeline.Is_Valid())
				m_device->Destroy_Pipeline(m_pipeline);
			if (m_shader.Is_Valid())
				m_shaders.Destroy(m_shader);
		}

		m_vertex_buffer = {};
		m_index_buffer = {};
		m_pipeline = {};
		m_shader = {};
		m_color_resource = {};
		m_pass = {};
		m_graph = {};
		m_plan = {};
		m_device = nullptr;
		m_initialized = false;
	}

	void Begin_Frame() noexcept
	{
		m_presentation_count = 0;
		m_accepting = m_initialized;
		for (Presentation &presentation : m_presentations)
			presentation.submitted = false;
	}

	bool Submit(
		Engine::Video::PresentationId source_id,
		const Engine::Video::DecodedVideoFrame &frame,
		Engine::Video::PresentationRect destination) override
	{
		if (!m_accepting || !frame.Is_Valid() || destination.width == 0 || destination.height == 0)
			return false;

		Presentation *presentation = Find_Presentation(source_id);
		if (presentation == nullptr) {
			if (m_presentation_count >= m_presentations.size())
				return false;
			presentation = &m_presentations[m_presentation_count++];
			presentation->source_id = source_id;
		}

		const std::uint64_t row_pitch_64 = static_cast<std::uint64_t>(frame.width) * 4u;
		if (row_pitch_64 == 0 || frame.height > std::numeric_limits<std::uint64_t>::max() / row_pitch_64)
			return false;
		const std::uint64_t byte_size = row_pitch_64 * frame.height;
		if (row_pitch_64 > std::numeric_limits<std::uint32_t>::max()
			|| byte_size > std::numeric_limits<std::size_t>::max())
			return false;
		const std::uint32_t row_pitch = static_cast<std::uint32_t>(row_pitch_64);
		if (presentation->pixels.size() < static_cast<std::size_t>(byte_size))
			presentation->pixels.resize(static_cast<std::size_t>(byte_size));
		if (!Copy_To_RGBA8(frame, presentation->pixels))
			return false;

		presentation->frame = {
			frame.width,
			frame.height,
			row_pitch,
			Engine::Video::PixelFormat::RGBA8,
			frame.frame_index,
			frame.presentation_time_us,
			{presentation->pixels.data(), static_cast<std::size_t>(byte_size)}
		};
		presentation->destination = destination;
		presentation->submitted = true;
		return true;
	}

	bool Render(CommandList &command_list, const FrameTargets &targets) noexcept
	{
		m_accepting = false;
		if (!m_initialized)
			return true;

		m_draw_count = 0;
		for (std::size_t index = 0; index < m_presentation_count; ++index) {
			Presentation &presentation = m_presentations[index];
			if (!presentation.submitted)
				continue;
			if (!Prepare_Presentation(presentation))
				return false;

			const std::int64_t right = static_cast<std::int64_t>(presentation.destination.x)
				+ presentation.destination.width;
			const std::int64_t bottom = static_cast<std::int64_t>(presentation.destination.y)
				+ presentation.destination.height;
			const std::int64_t left = std::max<std::int64_t>(0, presentation.destination.x);
			const std::int64_t top = std::max<std::int64_t>(0, presentation.destination.y);
			const std::int64_t clipped_right = std::min<std::int64_t>(targets.backbuffer.width, right);
			const std::int64_t clipped_bottom = std::min<std::int64_t>(targets.backbuffer.height, bottom);
			if (left >= clipped_right || top >= clipped_bottom)
				continue;

			m_draws[m_draw_count++] = {
				{static_cast<std::uint32_t>(left), static_cast<std::uint32_t>(top),
					static_cast<std::uint32_t>(clipped_right - left), static_cast<std::uint32_t>(clipped_bottom - top), 0.0f, 1.0f},
				m_pipeline,
				m_vertex_buffer,
				m_index_buffer,
				sizeof(VideoVertex),
				RHIIndexFormat::UInt16,
				6,
				presentation.resources
			};
		}

		m_binding = GraphResourceBinding::Texture(m_color_resource, targets.backbuffer.texture);
		const std::span<GraphResourceBinding> bindings(&m_binding, 1);
		if (!m_plan.Compile(m_graph, bindings))
			return false;

		const VideoPassInput input{{m_draws.data(), m_draw_count}, m_color_resource};
		return m_plan.Execute(m_graph, command_list, [this, &input](GraphPassHandle pass, CommandList &commands, const PassResources &resources) noexcept {
			return pass == m_pass && VideoPass::Execute(commands, resources, input);
		});
	}

	bool Is_Initialized() const noexcept
	{
		return m_initialized;
	}

	bool Accepting_Submissions() const noexcept
	{
		return m_accepting;
	}

	std::size_t Presentation_Count() const noexcept
	{
		return m_presentation_count;
	}

	RHITextureHandle Texture_Handle(std::size_t index = 0) const noexcept
	{
		return index < m_presentations.size() ? m_presentations[index].resident_texture : RHITextureHandle{};
	}

	std::size_t Draw_Count() const noexcept
	{
		return m_draw_count;
	}

private:
	struct VideoVertex final
	{
		float position[3]{};
		float color[4]{1.0f, 1.0f, 1.0f, 1.0f};
		float uv[2]{};
	};

	static_assert(sizeof(VideoVertex) == 36);

	struct Presentation final
	{
		std::uint64_t source_id = 0;
		std::vector<std::byte> pixels;
		Engine::Video::DecodedVideoFrame frame{};
		Engine::Video::PresentationRect destination{};
		TextureHandle texture{};
		MaterialHandle material{};
		RHITextureHandle resident_texture{};
		RHIBufferHandle material_constants{};
		std::array<RHIBindlessResource, 2> resources{};
		bool submitted = false;
	};

	Presentation *Find_Presentation(std::uint64_t source_id) noexcept
	{
		for (std::size_t index = 0; index < m_presentation_count; ++index) {
			if (m_presentations[index].source_id == source_id)
				return &m_presentations[index];
		}
		return nullptr;
	}

	static bool Copy_To_RGBA8(const Engine::Video::DecodedVideoFrame &frame, std::vector<std::byte> &destination) noexcept
	{
		const std::uint32_t source_bytes_per_pixel = Engine::Video::Bytes_Per_Pixel(frame.format);
		const std::uint64_t output_row_pitch_64 = static_cast<std::uint64_t>(frame.width) * 4u;
		if (output_row_pitch_64 == 0 || frame.height > std::numeric_limits<std::uint64_t>::max() / output_row_pitch_64)
			return false;
		const std::uint64_t output_size = output_row_pitch_64 * frame.height;
		if (source_bytes_per_pixel == 0 || output_row_pitch_64 > std::numeric_limits<std::size_t>::max()
			|| output_size > destination.size())
			return false;
		const std::size_t output_row_pitch = static_cast<std::size_t>(output_row_pitch_64);

		for (std::uint32_t y = 0; y < frame.height; ++y) {
			const std::byte *source = frame.pixels.data() + static_cast<std::size_t>(y) * frame.row_pitch;
			std::byte *output = destination.data() + static_cast<std::size_t>(y) * output_row_pitch;
			for (std::uint32_t x = 0; x < frame.width; ++x) {
				const std::byte *pixel = source + static_cast<std::size_t>(x) * source_bytes_per_pixel;
				std::uint8_t red = 0;
				std::uint8_t green = 0;
				std::uint8_t blue = 0;
				std::uint8_t alpha = 255;
				switch (frame.format) {
				case Engine::Video::PixelFormat::RGB8:
					red = std::to_integer<std::uint8_t>(pixel[0]);
					green = std::to_integer<std::uint8_t>(pixel[1]);
					blue = std::to_integer<std::uint8_t>(pixel[2]);
					break;
				case Engine::Video::PixelFormat::RGBA8:
					red = std::to_integer<std::uint8_t>(pixel[0]);
					green = std::to_integer<std::uint8_t>(pixel[1]);
					blue = std::to_integer<std::uint8_t>(pixel[2]);
					alpha = std::to_integer<std::uint8_t>(pixel[3]);
					break;
				case Engine::Video::PixelFormat::BGRA8:
				case Engine::Video::PixelFormat::BGRX8:
					blue = std::to_integer<std::uint8_t>(pixel[0]);
					green = std::to_integer<std::uint8_t>(pixel[1]);
					red = std::to_integer<std::uint8_t>(pixel[2]);
					alpha = frame.format == Engine::Video::PixelFormat::BGRX8 ? 255 : std::to_integer<std::uint8_t>(pixel[3]);
					break;
				case Engine::Video::PixelFormat::RGB565: {
					const std::uint16_t value = static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(pixel[0]))
						| static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(pixel[1])) << 8;
					red = static_cast<std::uint8_t>(((value >> 11) & 0x1f) * 255 / 31);
					green = static_cast<std::uint8_t>(((value >> 5) & 0x3f) * 255 / 63);
					blue = static_cast<std::uint8_t>((value & 0x1f) * 255 / 31);
					break;
				}
				case Engine::Video::PixelFormat::RGB555: {
					const std::uint16_t value = static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(pixel[0]))
						| static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(pixel[1])) << 8;
					red = static_cast<std::uint8_t>(((value >> 10) & 0x1f) * 255 / 31);
					green = static_cast<std::uint8_t>(((value >> 5) & 0x1f) * 255 / 31);
					blue = static_cast<std::uint8_t>((value & 0x1f) * 255 / 31);
					break;
				}
				case Engine::Video::PixelFormat::Unknown:
					return false;
				}

				output[x * 4 + 0] = static_cast<std::byte>(red);
				output[x * 4 + 1] = static_cast<std::byte>(green);
				output[x * 4 + 2] = static_cast<std::byte>(blue);
				output[x * 4 + 3] = static_cast<std::byte>(alpha);
			}
		}
		return true;
	}

	bool Prepare_Presentation(Presentation &presentation)
	{
		if (m_device == nullptr || !presentation.frame.Is_Valid())
			return false;

		if (!presentation.texture.Is_Valid()) {
			Texture texture;
			texture.width = presentation.frame.width;
			texture.height = presentation.frame.height;
			texture.mip_count = 1;
			texture.format = TextureFormat::RGBA8_UNorm;
			texture.usage = TextureUsage::Sampled;
			texture.pixel_data = presentation.frame.pixels;
			texture.row_pitch = presentation.frame.row_pitch;
			presentation.texture = m_textures.Create(texture);
			if (!presentation.texture.Is_Valid())
				return false;

			Material material;
			material.shader = m_shader;
			material.textures[0] = presentation.texture;
			material.flags = MaterialFlags::Unlit | MaterialFlags::Transparent;
			material.parameters.values[0] = 1.0f;
			material.parameters.values[1] = 1.0f;
			material.parameters.values[2] = 1.0f;
			material.parameters.values[3] = 1.0f;
			material.parameters.values[4] = static_cast<float>(presentation.texture.Get_Index());
			presentation.material = m_materials.Create(material);
			if (!presentation.material.Is_Valid())
				return false;
		}

		Texture *texture = m_textures.Resolve(presentation.texture);
		if (texture == nullptr)
			return false;
		texture->width = presentation.frame.width;
		texture->height = presentation.frame.height;
		texture->mip_count = 1;
		texture->format = TextureFormat::RGBA8_UNorm;
		texture->usage = TextureUsage::Sampled;
		texture->pixel_data = presentation.frame.pixels;
		texture->row_pitch = presentation.frame.row_pitch;
		texture->Mark_Dirty();
		if (!m_residency.Upload_Texture(presentation.texture, *texture))
			return false;

		const GPUResidentTexture resident_texture = m_residency.Texture_Info(presentation.texture);
		presentation.resident_texture = resident_texture.texture;
		if (!resident_texture.texture.Is_Valid())
			return false;

		Material *material = m_materials.Resolve(presentation.material);
		if (material == nullptr || !m_residency.Upload_Material(presentation.material, *material))
			return false;
		const GPUResidentMaterial resident_material = m_residency.Material_Info(presentation.material);
		presentation.material_constants = resident_material.constants;
		if (!presentation.material_constants.Is_Valid())
			return false;

		presentation.resources[0] = {
			ResourceIndex(presentation.texture.Get_Index(), presentation.texture.Get_Generation()),
			RHIResourceType::Texture,
			{},
			presentation.resident_texture
		};
		presentation.resources[1] = {
			ResourceIndex(presentation.material.Get_Index(), presentation.material.Get_Generation()),
			RHIResourceType::Material,
			presentation.material_constants,
			{}
		};
		return true;
	}

	Device *m_device = nullptr;
	ShaderLibrary m_shaders;
	ShaderHandle m_shader{};
	PipelineHandle m_pipeline{};
	RHIBufferHandle m_vertex_buffer{};
	RHIBufferHandle m_index_buffer{};
	TexturePool m_textures;
	MaterialPool m_materials;
	GPUResourceResidency m_residency;
	RenderGraph m_graph;
	ExecutionPlan m_plan;
	GraphResourceBinding m_binding{};
	GraphResourceHandle m_color_resource{};
	GraphPassHandle m_pass{};
	std::array<Presentation, Max_Presentations> m_presentations{};
	std::array<VideoDraw, Max_Presentations> m_draws{};
	std::size_t m_presentation_count = 0;
	std::size_t m_draw_count = 0;
	bool m_accepting = false;
	bool m_initialized = false;
};

export VideoRenderer &GetVideoRenderer() noexcept
{
	static VideoRenderer renderer;
	return renderer;
}

}
