module;

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <utility>
#include <vector>

#if defined(RTS_PROFILE_TRACY)
#include <tracy/Tracy.hpp>
#define GRAPHICS_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#endif

export module Graphics.Scene.Lighting.Renderer;

export import Graphics.RHI;
export import Graphics.Scene.RenderScene;

namespace Graphics
{

export class LightRenderer final
{
public:
	bool Initialize(Device &device, std::size_t max_lights = 4096)
	{
		if (m_device != nullptr || max_lights == 0 || max_lights > std::numeric_limits<std::uint32_t>::max() / sizeof(GPULightData))
			return false;
		if (m_scene.Light_Count() > max_lights)
			return false;

		m_scene.Reserve_Lights(max_lights);
		m_packed_lights.reserve(max_lights);
		m_uploaded_lights.reserve(max_lights);
		m_light_buffer = device.Create_Buffer({
			static_cast<std::uint32_t>(max_lights * sizeof(GPULightData)),
			RHIBufferUsage::Storage,
			static_cast<std::uint32_t>(sizeof(GPULightData))
		});
		if (!m_light_buffer.Is_Valid())
			return false;

		m_light_buffer_index = ResourceIndex(0, 1);
		m_light_resource = {m_light_buffer_index, RHIResourceType::Buffer, m_light_buffer, {}};

		m_device = &device;
		m_max_lights = max_lights;
		return true;
	}

	void Shutdown() noexcept
	{
		if (m_device != nullptr && m_light_buffer.Is_Valid())
			m_device->Destroy_Buffer(m_light_buffer);
		m_light_buffer = {};
		m_light_buffer_index = {};
		m_light_resource = {};
		m_packed_lights.clear();
		m_uploaded_lights.clear();
		m_light_count = 0;
		m_max_lights = 0;
		m_device = nullptr;
	}

	bool Is_Initialized() const noexcept
	{
		return m_device != nullptr && m_light_buffer.Is_Valid() && m_light_buffer_index.Is_Valid();
	}

	LightHandle Create_Point_Light(const RenderLight &light = {})
	{
		if (m_max_lights != 0 && m_scene.Light_Count() >= m_max_lights)
			return {};
		return m_scene.Create_Light(light);
	}

	bool Update_Point_Light(LightHandle handle, const RenderLight &light) noexcept
	{
		return m_scene.Update_Light(handle, light);
	}

	bool Destroy_Point_Light(LightHandle handle) noexcept
	{
		return m_scene.Destroy_Light(handle);
	}

	RenderScene &Scene() noexcept
	{
		return m_scene;
	}

	const RenderScene &Scene() const noexcept
	{
		return m_scene;
	}

	std::span<const RHIBindlessResource> Bindless_Resources() const noexcept
	{
		return m_light_resource.type == RHIResourceType::Invalid
			? std::span<const RHIBindlessResource>{}
			: std::span<const RHIBindlessResource>(&m_light_resource, 1);
	}

	ResourceIndex Light_Buffer_Index() const noexcept
	{
		return m_light_buffer_index;
	}

	RHIBufferHandle Light_Buffer() const noexcept
	{
		return m_light_buffer;
	}

	std::size_t Light_Count() const noexcept
	{
		return m_light_count;
	}

	std::span<const GPULightData> Packed_Lights() const noexcept
	{
		return m_packed_lights;
	}

	bool Sync() noexcept
	{
		GRAPHICS_PROFILE_SCOPE("Graphics::LightRenderer::Sync");
		if (!Is_Initialized())
			return false;

		const RenderLightData lights = m_scene.Lights();
		if (lights.Size() > m_max_lights || lights.Size() > m_packed_lights.capacity() || lights.Size() > m_uploaded_lights.capacity())
			return false;

		const std::size_t previous_count = m_light_count;
		m_packed_lights.resize(lights.Size());
		for (std::size_t dense_index = 0; dense_index < lights.Size(); ++dense_index)
			m_packed_lights[dense_index] = Pack_Light(lights, dense_index);

		m_uploaded_lights.resize(lights.Size());
		std::size_t first_changed = lights.Size();
		std::size_t last_changed = 0;
		for (std::size_t index = 0; index < lights.Size(); ++index) {
			if (index >= previous_count || m_packed_lights[index] != m_uploaded_lights[index]) {
				if (first_changed == lights.Size())
					first_changed = index;
				last_changed = index;
			}
		}

		if (first_changed != lights.Size()) {
			const std::size_t update_count = last_changed - first_changed + 1;
			const auto *first = reinterpret_cast<const std::byte *>(m_packed_lights.data() + first_changed);
			const std::span<const std::byte> bytes(first, update_count * sizeof(GPULightData));
			if (!m_device->Update_Buffer(m_light_buffer, static_cast<std::uint32_t>(first_changed * sizeof(GPULightData)), bytes))
				return false;
		}

		for (std::size_t index = 0; index < lights.Size(); ++index)
			m_uploaded_lights[index] = m_packed_lights[index];
		m_light_count = lights.Size();
		return true;
	}

private:
	static GPULightData Pack_Light(const RenderLightData &lights, std::size_t dense_index) noexcept
	{
		GPULightData packed;
		packed.position_range = {
			lights.position_x[dense_index],
			lights.position_y[dense_index],
			lights.position_z[dense_index],
			lights.ranges[dense_index]
		};
		packed.direction_intensity = {
			lights.direction_x[dense_index],
			lights.direction_y[dense_index],
			lights.direction_z[dense_index],
			lights.intensities[dense_index]
		};
		packed.color_inner_angle = {
			lights.color_r[dense_index],
			lights.color_g[dense_index],
			lights.color_b[dense_index],
			lights.inner_angles[dense_index]
		};
		packed.outer_angle = lights.outer_angles[dense_index];
		packed.type = static_cast<std::uint32_t>(lights.types[dense_index]);
		packed.flags = static_cast<std::uint32_t>(lights.flags[dense_index]);
		return packed;
	}

	Device *m_device = nullptr;
	RenderScene m_scene;
	RHIBufferHandle m_light_buffer{};
	ResourceIndex m_light_buffer_index{};
	RHIBindlessResource m_light_resource{};
	std::vector<GPULightData> m_packed_lights;
	std::vector<GPULightData> m_uploaded_lights;
	std::size_t m_light_count = 0;
	std::size_t m_max_lights = 0;
};

namespace
{
LightRenderer g_light_renderer;
}

export LightRenderer &GetLightRenderer() noexcept
{
	return g_light_renderer;
}

export LightHandle CreatePointLight(const RenderLight &light)
{
	return g_light_renderer.Create_Point_Light(light);
}

export bool UpdatePointLight(LightHandle handle, const RenderLight &light) noexcept
{
	return g_light_renderer.Update_Point_Light(handle, light);
}

export bool DestroyPointLight(LightHandle handle) noexcept
{
	return g_light_renderer.Destroy_Point_Light(handle);
}

}
