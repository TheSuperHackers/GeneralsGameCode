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

export module Graphics.Scene.Screen.Distortion;

export import Graphics.Resources.Bindless.BindlessResourceTable;
export import Graphics.RenderGraph.Execution;
export import Graphics.Shaders.Library;
export import Graphics.Scene.Views.View;

namespace Graphics
{

export struct ScreenDistortionData final
{
	std::span<const float> position_x{};
	std::span<const float> position_y{};
	std::span<const float> position_z{};
	std::span<const float> offset_x{};
	std::span<const float> offset_y{};
	std::span<const float> sizes{};
	std::span<const float> opacities{};
    std::array<float,3> tint{1,1,1};

	std::size_t Size() const noexcept
	{
		return position_x.size();
	}
};

export struct ScreenDistortionVertex final
{
	float position[3]{};
	float color[4]{};
	float uv[2]{};
};

static_assert(sizeof(ScreenDistortionVertex) == 36);

export class ScreenDistortionRenderer final
{
public:
	bool Initialize(Device &device, const std::filesystem::path &shader_directory, std::size_t max_distortions = 512)
	{
		if (m_device != nullptr || max_distortions == 0 || max_distortions > std::numeric_limits<std::uint16_t>::max() / 5)
			return false;

		m_device = &device;
		m_max_distortions = max_distortions;
		m_vertices.resize(max_distortions * 5);
		std::vector<std::uint16_t> indices(max_distortions * 12);
		for (std::size_t distortion = 0; distortion < max_distortions; ++distortion) {
			const std::uint16_t base = static_cast<std::uint16_t>(distortion * 5);
			const std::size_t index = distortion * 12;
			indices[index + 0] = base + 0;
			indices[index + 1] = base + 4;
			indices[index + 2] = base + 3;
			indices[index + 3] = base + 3;
			indices[index + 4] = base + 4;
			indices[index + 5] = base + 2;
			indices[index + 6] = base + 2;
			indices[index + 7] = base + 4;
			indices[index + 8] = base + 1;
			indices[index + 9] = base + 1;
			indices[index + 10] = base + 4;
			indices[index + 11] = base + 0;
		}

		m_graph = std::make_unique<RenderGraph>();
		m_graph->Reserve(4, 2, 5);
		m_source_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		m_background_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		m_color_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		m_depth_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		const std::array<GraphResourceUse, 2> copy_uses = {
			GraphResourceUse::Read(m_source_resource),
			GraphResourceUse::Write(m_background_resource)
		};
		const std::array<GraphResourceUse, 3> draw_uses = {
			GraphResourceUse::Read(m_background_resource),
			GraphResourceUse::Write(m_color_resource),
			GraphResourceUse::Read(m_depth_resource)
		};
		m_copy_pass = m_graph->Add_Pass({30}, copy_uses);
		m_draw_pass = m_graph->Add_Pass({31}, draw_uses);
		if (!m_source_resource.Is_Valid() || !m_background_resource.Is_Valid() || !m_color_resource.Is_Valid()
			|| !m_depth_resource.Is_Valid() || !m_copy_pass.Is_Valid() || !m_draw_pass.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_shader = m_shaders.Load_Screen_Distortion(shader_directory);
		if (!m_shader.Is_Valid()) {
			Shutdown();
			return false;
		}
		m_pipeline = m_shaders.Create_Pipeline(device, m_shader, Make_Screen_Distortion_Pipeline());
		if (!m_pipeline.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_vertex_buffer = device.Create_Buffer({
			static_cast<std::uint32_t>(m_vertices.size() * sizeof(ScreenDistortionVertex)),
			RHIBufferUsage::Vertex,
			static_cast<std::uint32_t>(sizeof(ScreenDistortionVertex))
		});
		m_index_buffer = device.Create_Buffer_Initialized({
			static_cast<std::uint32_t>(indices.size() * sizeof(std::uint16_t)),
			RHIBufferUsage::Index,
			static_cast<std::uint32_t>(sizeof(std::uint16_t))
		}, std::as_bytes(std::span<const std::uint16_t>(indices)));
		if (!m_vertex_buffer.Is_Valid() || !m_index_buffer.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_bindless.Reserve(1, 0, 1);
		m_background_handle = TextureHandle(0, 1);
		m_background_width = 0;
		m_background_height = 0;
		return true;
	}

	void Shutdown() noexcept
	{
		if (m_device != nullptr) {
			if (m_vertex_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_vertex_buffer);
			if (m_index_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_index_buffer);
			if (m_background_texture.Is_Valid())
				m_device->Destroy_Texture(m_background_texture);
			if (m_pipeline.Is_Valid())
				m_device->Destroy_Pipeline(m_pipeline);
		}
		m_bindless.Clear();
		m_shaders.Destroy(m_shader);
		m_shader = {};
		m_pipeline = {};
		m_vertex_buffer = {};
		m_index_buffer = {};
		m_background_texture = {};
		m_background_handle = {};
		m_vertices.clear();
		m_graph.reset();
		m_plan = {};
		m_source_resource = {};
		m_background_resource = {};
		m_color_resource = {};
		m_depth_resource = {};
		m_copy_pass = {};
		m_draw_pass = {};
		m_background_width = 0;
		m_background_height = 0;
		m_max_distortions = 0;
		m_device = nullptr;
	}

	bool Is_Initialized() const noexcept
	{
		return m_device != nullptr && m_pipeline.Is_Valid() && m_vertex_buffer.Is_Valid() && m_index_buffer.Is_Valid();
	}

	bool Set_View(const View &view) noexcept
	{
		if (!Is_Initialized())
			return false;
		m_view = view;
		return true;
	}

	bool Render(CommandList &commands, RHITextureHandle color_target, RHITextureHandle depth_target,
		RHIViewport viewport, const ScreenDistortionData &data,
        RHITextureFormat color_format=RHITextureFormat::BGRA8_UNorm) noexcept
	{
		if (!Is_Initialized() || !color_target.Is_Valid() || !depth_target.Is_Valid() || viewport.width == 0 || viewport.height == 0)
			return false;
		if (data.Size() == 0)
			return true;
		if (data.Size() > m_max_distortions || data.position_y.size() != data.Size() || data.position_z.size() != data.Size()
			|| data.offset_x.size() != data.Size() || data.offset_y.size() != data.Size() || data.sizes.size() != data.Size()
			|| data.opacities.size() != data.Size())
			return false;

		if (!Ensure_Background(viewport.width, viewport.height,color_format))
			return false;
		const std::size_t vertex_count = data.Size() * 5;
		for (std::size_t index = 0; index < data.Size(); ++index)
			Pack_Distortion(data, index, m_vertices.data() + index * 5);
		if (!m_device->Update_Buffer(m_vertex_buffer, 0, std::as_bytes(std::span<const ScreenDistortionVertex>(m_vertices.data(), vertex_count))))
			return false;

		m_bindings[0] = GraphResourceBinding::Texture(m_source_resource, color_target);
		m_bindings[1] = GraphResourceBinding::Texture(m_background_resource, m_background_texture);
		m_bindings[2] = GraphResourceBinding::Texture(m_color_resource, color_target);
		m_bindings[3] = GraphResourceBinding::Texture(m_depth_resource, depth_target);
		if (!m_plan.Is_Valid() && !m_plan.Compile(*m_graph, m_bindings))
			return false;

		const std::uint32_t index_count = static_cast<std::uint32_t>(data.Size() * 12);
		return m_plan.Execute(*m_graph, commands, [&](GraphPassHandle pass, CommandList &command_list, const PassResources &resources) noexcept {
			if (pass == m_copy_pass)
				return command_list.Copy_Texture(resources.Texture(m_source_resource), resources.Texture(m_background_resource));
			if (pass != m_draw_pass)
				return false;
			return command_list.Set_Render_Targets(resources.Texture(m_color_resource), resources.Texture(m_depth_resource))
				&& command_list.Set_Viewport(viewport)
				&& command_list.Set_Bindless_Resources(m_bindless.Resources())
				&& command_list.Bind_Pipeline(m_pipeline)
				&& command_list.Set_Vertex_Buffer(0, m_vertex_buffer, sizeof(ScreenDistortionVertex), 0)
				&& command_list.Set_Index_Buffer(m_index_buffer, RHIIndexFormat::UInt16, 0)
				&& command_list.Draw_Indexed(index_count, 0, 0, 1, 0);
		});
	}

private:
	static std::array<float, 4> Transform(const Matrix4x4 &matrix, float x, float y, float z, float w) noexcept
	{
		return {
			matrix(0, 0) * x + matrix(0, 1) * y + matrix(0, 2) * z + matrix(0, 3) * w,
			matrix(1, 0) * x + matrix(1, 1) * y + matrix(1, 2) * z + matrix(1, 3) * w,
			matrix(2, 0) * x + matrix(2, 1) * y + matrix(2, 2) * z + matrix(2, 3) * w,
			matrix(3, 0) * x + matrix(3, 1) * y + matrix(3, 2) * z + matrix(3, 3) * w
		};
	}

	bool Ensure_Background(std::uint32_t width, std::uint32_t height,RHITextureFormat format) noexcept
	{
		if (m_background_texture.Is_Valid() && m_background_width == width && m_background_height == height
            && m_background_format==format)
			return true;
		const auto replacement=m_device->Create_Texture({
			width,
			height,
			1,
			format,
			static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)
		});
		if (!replacement.Is_Valid())
			return false;
        const bool bound=m_background_texture.Is_Valid()
            ? m_bindless.Update_Texture(m_background_handle,replacement)
            : m_bindless.Register_Texture(m_background_handle,replacement).Is_Valid();
        if (!bound) {
            m_device->Destroy_Texture(replacement);
            return false;
        }
        if (m_background_texture.Is_Valid()) m_device->Destroy_Texture(m_background_texture);
        m_background_texture=replacement;
		m_background_format=format;
		m_background_width = width;
		m_background_height = height;
		return true;
	}

	void Pack_Distortion(const ScreenDistortionData &data, std::size_t index, ScreenDistortionVertex *vertices) const noexcept
	{
		const std::array<float, 4> view_center = Transform(m_view.view_matrix, data.position_x[index], data.position_y[index], data.position_z[index], 1.0f);
		const float size = data.sizes[index];
		const std::array<std::array<float, 3>, 4> offsets = {{
			{{-size, size, 0.0f}},
			{{-size, -size, 0.0f}},
			{{size, -size, 0.0f}},
			{{size, size, 0.0f}}
		}};
		std::array<std::array<float, 2>, 4> uvs{};
		for (std::size_t corner = 0; corner < 4; ++corner) {
			const std::array<float, 4> clip = Transform(m_view.projection_matrix,
				view_center[0] + offsets[corner][0], view_center[1] + offsets[corner][1], view_center[2], view_center[3]);
			const float inverse_w = std::fabs(clip[3]) > 1.0e-6f ? 1.0f / clip[3] : 0.0f;
			const float ndc_x = clip[0] * inverse_w;
			const float ndc_y = clip[1] * inverse_w;
			vertices[corner].position[0] = ndc_x;
			vertices[corner].position[1] = ndc_y;
			vertices[corner].position[2] = clip[2] * inverse_w;
			vertices[corner].color[0] = data.tint[0];
			vertices[corner].color[1] = data.tint[1];
			vertices[corner].color[2] = data.tint[2];
			vertices[corner].color[3] = 0.0f;
			uvs[corner] = {(ndc_x + 1.0f) * 0.5f, (1.0f - ndc_y) * 0.5f};
			vertices[corner].uv[0] = uvs[corner][0];
			vertices[corner].uv[1] = uvs[corner][1];
		}
		float offset_x = data.offset_x[index];
		float offset_y = data.offset_y[index];
		for (const auto &uv : uvs) {
			if (uv[0] < 0.0f || uv[0] > 1.0f)
				offset_x = 0.0f;
			if (uv[1] < 0.0f || uv[1] > 1.0f)
				offset_y = 0.0f;
		}
		const float uv_span_x = uvs[3][0] - uvs[0][0];
		const float uv_span_y = uvs[1][1] - uvs[0][1];
		vertices[4].uv[0] = uvs[0][0] + uv_span_x * (0.5f + offset_x);
		vertices[4].uv[1] = uvs[0][1] + uv_span_y * (0.5f + offset_y);
		vertices[4].color[0] = data.tint[0];
		vertices[4].color[1] = data.tint[1];
		vertices[4].color[2] = data.tint[2];
		vertices[4].color[3] = data.opacities[index];
		const std::array<float, 4> center_clip = Transform(m_view.projection_matrix, view_center[0], view_center[1], view_center[2], view_center[3]);
		const float center_inverse_w = std::fabs(center_clip[3]) > 1.0e-6f ? 1.0f / center_clip[3] : 0.0f;
		vertices[4].position[0] = center_clip[0] * center_inverse_w;
		vertices[4].position[1] = center_clip[1] * center_inverse_w;
		vertices[4].position[2] = center_clip[2] * center_inverse_w;
	}

	Device *m_device = nullptr;
	std::size_t m_max_distortions = 0;
	std::size_t m_background_width = 0;
	std::size_t m_background_height = 0;
	View m_view{};
	ShaderLibrary m_shaders;
	ShaderHandle m_shader{};
	PipelineHandle m_pipeline{};
	RHITextureHandle m_background_texture{};
    RHITextureFormat m_background_format=RHITextureFormat::BGRA8_UNorm;
	TextureHandle m_background_handle{};
	RHIBufferHandle m_vertex_buffer{};
	RHIBufferHandle m_index_buffer{};
	BindlessResourceTable m_bindless;
	std::vector<ScreenDistortionVertex> m_vertices;
	std::unique_ptr<RenderGraph> m_graph;
	ExecutionPlan m_plan;
	std::array<GraphResourceBinding, 4> m_bindings{};
	GraphResourceHandle m_source_resource{};
	GraphResourceHandle m_background_resource{};
	GraphResourceHandle m_color_resource{};
	GraphResourceHandle m_depth_resource{};
	GraphPassHandle m_copy_pass{};
	GraphPassHandle m_draw_pass{};
};

namespace
{
ScreenDistortionRenderer g_screen_distortion_renderer;
}

export ScreenDistortionRenderer &GetScreenDistortionRenderer() noexcept
{
	return g_screen_distortion_renderer;
}

}
