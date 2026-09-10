module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

export module Graphics.Resources.Residency.GPUResourceResidency;

export import Graphics.Resources.Materials.Material;
export import Graphics.Resources.Meshes.Mesh;
export import Graphics.Resources.Textures.Texture;
export import Graphics.RHI;

namespace Graphics
{

export inline constexpr std::uint32_t Invalid_GPU_Resource_Index = std::numeric_limits<std::uint32_t>::max();

export struct GPUResidentMesh final
{
	std::uint32_t gpu_index = Invalid_GPU_Resource_Index;
	RHIBufferHandle vertex_buffer{};
	RHIBufferHandle index_buffer{};
	MeshIndexFormat index_format = MeshIndexFormat::None;
	std::uint32_t vertex_count = 0;
	std::uint32_t index_count = 0;
	std::uint32_t vertex_stride = 0;
	std::uint32_t vertex_byte_size = 0;
	std::uint32_t index_byte_size = 0;
	std::uint32_t revision = 0;
};

export struct GPUResidentTexture final
{
	std::uint32_t gpu_index = Invalid_GPU_Resource_Index;
	RHITextureHandle texture{};
	RHITexture description{};
	std::uint32_t revision = 0;
};

export struct GPUResidentMaterial final
{
	std::uint32_t gpu_index = Invalid_GPU_Resource_Index;
	RHIBufferHandle constants{};
	std::array<std::uint32_t, Material::TextureSlotCount> texture_indices{};
	std::array<RHITextureHandle, Material::TextureSlotCount> textures{};
	std::array<std::uint32_t, Material::SamplerSlotCount> sampler_indices{};
	MaterialFlags flags = MaterialFlags::None;
	std::uint32_t revision = 0;
};

export class GPUResourceResidency final
{
public:
	GPUResourceResidency() noexcept = default;

	explicit GPUResourceResidency(Device &device) noexcept
		: m_device(&device)
	{
	}

	~GPUResourceResidency() noexcept
	{
		Clear();
	}

	GPUResourceResidency(const GPUResourceResidency &) = delete;
	GPUResourceResidency &operator=(const GPUResourceResidency &) = delete;

	void Set_Device(Device &device) noexcept
	{
		m_device = &device;
	}

	bool Upload_Mesh(MeshHandle handle, const Mesh &mesh)
	{
		if (!Is_Valid_Mesh(handle, mesh))
			return false;

		MeshSlot &slot = Mesh_Slot(handle);
		if (slot.generation != 0 && slot.generation != handle.Get_Generation())
			return false;
		if (slot.generation == 0 && slot.last_generation == handle.Get_Generation())
			return false;

		const std::uint32_t vertex_byte_size = static_cast<std::uint32_t>(mesh.vertex_data.size());
		const std::uint32_t index_byte_size = static_cast<std::uint32_t>(mesh.index_data.size());
		if (slot.generation == handle.Get_Generation() && slot.resource.vertex_buffer.Is_Valid() && slot.resource.index_buffer.Is_Valid()) {
			if (slot.resource.revision == mesh.revision)
				return true;

			if (slot.resource.vertex_byte_size == vertex_byte_size && slot.resource.index_byte_size == index_byte_size) {
				if (!m_device->Update_Buffer(slot.resource.vertex_buffer, 0, mesh.vertex_data)
					|| !m_device->Update_Buffer(slot.resource.index_buffer, 0, mesh.index_data))
					return false;

				slot.resource.vertex_count = mesh.vertex_count;
				slot.resource.index_count = mesh.index_count;
				slot.resource.vertex_stride = mesh.vertex_stride;
				slot.resource.index_format = mesh.index_format;
				slot.resource.revision = mesh.revision;
				return true;
			}
		}

		const RHIBufferHandle vertex_buffer = m_device->Create_Buffer_Initialized({vertex_byte_size, RHIBufferUsage::Vertex, mesh.vertex_stride}, mesh.vertex_data);
		if (!vertex_buffer.Is_Valid())
			return false;

		const RHIBufferHandle index_buffer = m_device->Create_Buffer_Initialized({index_byte_size, RHIBufferUsage::Index, 0}, mesh.index_data);
		if (!index_buffer.Is_Valid()) {
			m_device->Destroy_Buffer(vertex_buffer);
			return false;
		}

		if (slot.generation == handle.Get_Generation())
			Release_Mesh(slot.resource);
		slot.generation = handle.Get_Generation();
		slot.resource = {
			handle.Get_Index(),
			vertex_buffer,
			index_buffer,
			mesh.index_format,
			mesh.vertex_count,
			mesh.index_count,
			mesh.vertex_stride,
			vertex_byte_size,
			index_byte_size,
			mesh.revision
		};
		return true;
	}

	bool Upload_Mesh(MeshHandle handle, const MeshPool &meshes)
	{
		const Mesh *mesh = meshes.Resolve(handle);
		return mesh != nullptr && Upload_Mesh(handle, *mesh);
	}

	bool Upload_Texture(TextureHandle handle, const Texture &texture)
	{
		RHITexture description{};
		RHITextureUpload upload{};
		if (!Build_Texture_Upload(texture, description, upload) || !handle.Is_Valid())
			return false;

		TextureSlot &slot = Texture_Slot(handle);
		if (slot.generation != 0 && slot.generation != handle.Get_Generation())
			return false;
		if (slot.generation == 0 && slot.last_generation == handle.Get_Generation())
			return false;

		if (slot.generation == handle.Get_Generation() && slot.resource.texture.Is_Valid()) {
			if (slot.resource.revision == texture.revision && slot.resource.description.width == description.width && slot.resource.description.height == description.height && slot.resource.description.mip_count == description.mip_count && slot.resource.description.format == description.format && slot.resource.description.usage == description.usage)
				return true;

			if (slot.resource.description.width == description.width && slot.resource.description.height == description.height && slot.resource.description.mip_count == description.mip_count && slot.resource.description.format == description.format && slot.resource.description.usage == description.usage) {
				if (!m_device->Update_Texture(slot.resource.texture, upload))
					return false;

				slot.resource.description = description;
				slot.resource.revision = texture.revision;
				return true;
			}
		}

		const RHITextureHandle new_texture = Create_Texture(description, upload);
		if (!new_texture.Is_Valid())
			return false;

		if (slot.generation == handle.Get_Generation())
			Release_Texture(slot.resource);
		slot.generation = handle.Get_Generation();
		slot.resource = {handle.Get_Index(), new_texture, description, texture.revision};
		return true;
	}

	bool Upload_Texture(TextureHandle handle, const TexturePool &textures)
	{
		const Texture *texture = textures.Resolve(handle);
		return texture != nullptr && Upload_Texture(handle, *texture);
	}

	bool Upload_Material(MaterialHandle handle, const Material &material)
	{
		if (!handle.Is_Valid())
			return false;

		std::array<std::uint32_t, Material::TextureSlotCount> texture_indices{};
		std::array<RHITextureHandle, Material::TextureSlotCount> textures{};
		texture_indices.fill(Invalid_GPU_Resource_Index);
		for (std::size_t index = 0; index < material.textures.size(); ++index) {
			const TextureHandle texture_handle = material.textures[index];
			if (!texture_handle.Is_Valid())
				continue;

			const GPUResidentTexture texture = Texture_Info(texture_handle);
			if (!texture.texture.Is_Valid())
				return false;

			texture_indices[index] = texture.gpu_index;
			textures[index] = texture.texture;
		}
		std::array<std::uint32_t, Material::SamplerSlotCount> sampler_indices{};
		sampler_indices.fill(Invalid_GPU_Resource_Index);
		for (std::size_t index = 0; index < material.samplers.size(); ++index) {
			if (material.samplers[index].Is_Valid())
				sampler_indices[index] = material.samplers[index].Get_Index();
		}

		MaterialSlot &slot = Material_Slot(handle);
		if (slot.generation != 0 && slot.generation != handle.Get_Generation())
			return false;
		if (slot.generation == 0 && slot.last_generation == handle.Get_Generation())
			return false;

		const bool metadata_unchanged = slot.generation == handle.Get_Generation()
			&& slot.resource.texture_indices == texture_indices
			&& slot.resource.textures == textures
			&& slot.resource.sampler_indices == sampler_indices
			&& slot.resource.flags == material.flags;
		if (metadata_unchanged && slot.resource.revision == material.revision && slot.resource.constants.Is_Valid())
			return true;

		const std::span<const std::byte> parameter_data = material.parameters.Bytes();
		RHIBufferHandle constants = slot.resource.constants;
		if (!constants.Is_Valid()) {
			constants = m_device->Create_Buffer_Initialized({static_cast<std::uint32_t>(parameter_data.size()), RHIBufferUsage::Constant, 16}, parameter_data);
			if (!constants.Is_Valid())
				return false;
		} else if (slot.resource.revision != material.revision && !m_device->Update_Buffer(constants, 0, parameter_data)) {
			return false;
		}

		GPUResidentMaterial resource;
		resource.gpu_index = handle.Get_Index();
		resource.constants = constants;
		resource.texture_indices = texture_indices;
		resource.textures = textures;
		resource.sampler_indices = sampler_indices;
		resource.flags = material.flags;
		resource.revision = material.revision;
		if (slot.generation == handle.Get_Generation() && slot.resource.constants.Is_Valid() && slot.resource.constants != constants)
			m_device->Destroy_Buffer(slot.resource.constants);
		slot.generation = handle.Get_Generation();
		slot.resource = resource;
		return true;
	}

	bool Upload_Material(MaterialHandle handle, const MaterialPool &materials)
	{
		const Material *material = materials.Resolve(handle);
		return material != nullptr && Upload_Material(handle, *material);
	}

	bool Destroy_Mesh(MeshHandle handle) noexcept
	{
		if (!Is_Mesh_Handle_Resident(handle))
			return false;

		MeshSlot &slot = m_meshes[handle.Get_Index()];
		const bool result = Release_Mesh(slot.resource);
		slot.generation = 0;
		slot.last_generation = handle.Get_Generation();
		return result;
	}

	bool Destroy_Texture(TextureHandle handle) noexcept
	{
		if (!Is_Texture_Handle_Resident(handle))
			return false;

		TextureSlot &slot = m_textures[handle.Get_Index()];
		Invalidate_Materials_Using(slot.resource.texture);
		const bool result = Release_Texture(slot.resource);
		slot.generation = 0;
		slot.last_generation = handle.Get_Generation();
		return result;
	}

	bool Destroy_Material(MaterialHandle handle) noexcept
	{
		if (!Is_Material_Handle_Resident(handle))
			return false;

		MaterialSlot &slot = m_materials[handle.Get_Index()];
		const bool result = Release_Material(slot.resource);
		slot.generation = 0;
		slot.last_generation = handle.Get_Generation();
		return result;
	}

	GPUResidentMesh Mesh_Info(MeshHandle handle) const noexcept
	{
		if (!Is_Mesh_Handle_Resident(handle))
			return {};

		return m_meshes[handle.Get_Index()].resource;
	}

	GPUResidentTexture Texture_Info(TextureHandle handle) const noexcept
	{
		if (!Is_Texture_Handle_Resident(handle))
			return {};

		return m_textures[handle.Get_Index()].resource;
	}

	GPUResidentMaterial Material_Info(MaterialHandle handle) const noexcept
	{
		if (!Is_Material_Handle_Resident(handle))
			return {};

		return m_materials[handle.Get_Index()].resource;
	}

	std::uint32_t Mesh_Index(MeshHandle handle) const noexcept
	{
		return Mesh_Info(handle).gpu_index;
	}

	std::uint32_t Texture_Index(TextureHandle handle) const noexcept
	{
		return Texture_Info(handle).gpu_index;
	}

	std::uint32_t Material_Index(MaterialHandle handle) const noexcept
	{
		return Material_Info(handle).gpu_index;
	}

	void Clear() noexcept
	{
		for (MeshSlot &slot : m_meshes) {
			Release_Mesh(slot.resource);
			slot = {};
		}
		for (TextureSlot &slot : m_textures) {
			Release_Texture(slot.resource);
			slot = {};
		}
		for (MaterialSlot &slot : m_materials) {
			Release_Material(slot.resource);
			slot = {};
		}
	}

private:
	template <typename Resource>
	struct Slot final
	{
		std::uint32_t generation = 0;
		std::uint32_t last_generation = 0;
		Resource resource{};
	};

	using MeshSlot = Slot<GPUResidentMesh>;
	using TextureSlot = Slot<GPUResidentTexture>;
	using MaterialSlot = Slot<GPUResidentMaterial>;

	static bool Is_Valid_Mesh(MeshHandle handle, const Mesh &mesh) noexcept
	{
		if (!handle.Is_Valid() || mesh.vertex_count == 0 || mesh.index_count == 0 || mesh.vertex_stride == 0 || mesh.index_format == MeshIndexFormat::None)
			return false;

		const std::uint64_t vertex_size = static_cast<std::uint64_t>(mesh.vertex_count) * mesh.vertex_stride;
		const std::uint32_t index_stride = mesh.index_format == MeshIndexFormat::UInt16 ? 2u : 4u;
		const std::uint64_t index_size = static_cast<std::uint64_t>(mesh.index_count) * index_stride;
		return vertex_size <= std::numeric_limits<std::uint32_t>::max()
			&& index_size <= std::numeric_limits<std::uint32_t>::max()
			&& mesh.vertex_data.size() == vertex_size
			&& mesh.index_data.size() == index_size;
	}

	static std::uint32_t Bytes_Per_Pixel(TextureFormat format) noexcept
	{
		switch (format) {
		case TextureFormat::R8_UNorm:
			return 1;
		case TextureFormat::RG8_UNorm:
			return 2;
		case TextureFormat::RGBA8_UNorm:
		case TextureFormat::BGRA8_UNorm:
		case TextureFormat::Depth32_Float:
			return 4;
		case TextureFormat::RGBA16_Float:
			return 8;
		case TextureFormat::RGBA32_Float:
			return 16;
		case TextureFormat::Unknown:
			return 0;
		}

		return 0;
	}

	static bool Build_Texture_Upload(const Texture &texture, RHITexture &description, RHITextureUpload &upload) noexcept
	{
		if (texture.width == 0 || texture.height == 0 || texture.depth != 1 || texture.mip_count == 0 || texture.pixel_data.empty())
			return false;

		RHITextureFormat format = RHITextureFormat::RGBA8_UNorm;
		switch (texture.format) {
		case TextureFormat::R8_UNorm:
			format = RHITextureFormat::R8_UNorm;
			break;
		case TextureFormat::RG8_UNorm:
			format = RHITextureFormat::RG8_UNorm;
			break;
		case TextureFormat::RGBA8_UNorm:
			format = RHITextureFormat::RGBA8_UNorm;
			break;
		case TextureFormat::BGRA8_UNorm:
			format = RHITextureFormat::BGRA8_UNorm;
			break;
		case TextureFormat::RGBA16_Float:
			format = RHITextureFormat::RGBA16_Float;
			break;
		case TextureFormat::RGBA32_Float:
			format = RHITextureFormat::RGBA32_Float;
			break;
		case TextureFormat::Depth32_Float:
			format = RHITextureFormat::D32_Float;
			break;
		case TextureFormat::Unknown:
			return false;
		}

		std::uint32_t usage = 0;
		if (Has_Texture_Usage(texture.usage, TextureUsage::Sampled))
			usage |= static_cast<std::uint32_t>(RHITextureUsage::ShaderResource);
		if (Has_Texture_Usage(texture.usage, TextureUsage::RenderTarget))
			usage |= static_cast<std::uint32_t>(RHITextureUsage::RenderTarget);
		if (Has_Texture_Usage(texture.usage, TextureUsage::DepthStencil))
			usage |= static_cast<std::uint32_t>(RHITextureUsage::DepthStencil);
		if (Has_Texture_Usage(texture.usage, TextureUsage::Storage))
			usage |= static_cast<std::uint32_t>(RHITextureUsage::UnorderedAccess);
		if (usage == 0)
			return false;

		const std::uint32_t bytes_per_pixel = Bytes_Per_Pixel(texture.format);
		const std::uint64_t minimum_row_pitch = static_cast<std::uint64_t>(texture.width) * bytes_per_pixel;
		const std::uint32_t row_pitch = texture.row_pitch == 0 ? static_cast<std::uint32_t>(minimum_row_pitch) : texture.row_pitch;
		if (bytes_per_pixel == 0 || minimum_row_pitch > std::numeric_limits<std::uint32_t>::max() || row_pitch < minimum_row_pitch || texture.pixel_data.size() < static_cast<std::uint64_t>(row_pitch) * texture.height || texture.mip_count > std::numeric_limits<std::uint32_t>::max())
			return false;

		description = {texture.width, texture.height, texture.mip_count, format, usage, texture.depth};
		upload = {texture.pixel_data, row_pitch};
		return true;
	}

	RHITextureHandle Create_Texture(const RHITexture &description, const RHITextureUpload &upload)
	{
		if (description.mip_count == 1) {
			return m_device->Create_Texture_Initialized(description, upload);
		}

		const RHITextureHandle texture = m_device->Create_Texture(description);
		if (!texture.Is_Valid())
			return {};
		if (!m_device->Update_Texture(texture, upload)) {
			m_device->Destroy_Texture(texture);
			return {};
		}
		return texture;
	}

	MeshSlot &Mesh_Slot(MeshHandle handle)
	{
		if (m_meshes.size() <= handle.Get_Index())
			m_meshes.resize(static_cast<std::size_t>(handle.Get_Index()) + 1);
		return m_meshes[handle.Get_Index()];
	}

	TextureSlot &Texture_Slot(TextureHandle handle)
	{
		if (m_textures.size() <= handle.Get_Index())
			m_textures.resize(static_cast<std::size_t>(handle.Get_Index()) + 1);
		return m_textures[handle.Get_Index()];
	}

	MaterialSlot &Material_Slot(MaterialHandle handle)
	{
		if (m_materials.size() <= handle.Get_Index())
			m_materials.resize(static_cast<std::size_t>(handle.Get_Index()) + 1);
		return m_materials[handle.Get_Index()];
	}

	bool Is_Mesh_Handle_Resident(MeshHandle handle) const noexcept
	{
		return handle.Is_Valid() && handle.Get_Index() < m_meshes.size() && m_meshes[handle.Get_Index()].generation == handle.Get_Generation() && m_meshes[handle.Get_Index()].resource.vertex_buffer.Is_Valid() && m_meshes[handle.Get_Index()].resource.index_buffer.Is_Valid();
	}

	bool Is_Texture_Handle_Resident(TextureHandle handle) const noexcept
	{
		return handle.Is_Valid() && handle.Get_Index() < m_textures.size() && m_textures[handle.Get_Index()].generation == handle.Get_Generation() && m_textures[handle.Get_Index()].resource.texture.Is_Valid();
	}

	bool Is_Material_Handle_Resident(MaterialHandle handle) const noexcept
	{
		return handle.Is_Valid() && handle.Get_Index() < m_materials.size() && m_materials[handle.Get_Index()].generation == handle.Get_Generation() && m_materials[handle.Get_Index()].resource.constants.Is_Valid();
	}

	bool Release_Mesh(GPUResidentMesh &resource) noexcept
	{
		const bool vertex_result = !resource.vertex_buffer.Is_Valid() || m_device->Destroy_Buffer(resource.vertex_buffer);
		const bool index_result = !resource.index_buffer.Is_Valid() || m_device->Destroy_Buffer(resource.index_buffer);
		resource = {};
		return vertex_result && index_result;
	}

	bool Release_Texture(GPUResidentTexture &resource) noexcept
	{
		const bool result = !resource.texture.Is_Valid() || m_device->Destroy_Texture(resource.texture);
		resource = {};
		return result;
	}

	bool Release_Material(GPUResidentMaterial &resource) noexcept
	{
		const bool result = !resource.constants.Is_Valid() || m_device->Destroy_Buffer(resource.constants);
		resource = {};
		return result;
	}

	void Invalidate_Materials_Using(RHITextureHandle texture) noexcept
	{
		if (!texture.Is_Valid())
			return;

		for (MaterialSlot &slot : m_materials) {
			bool uses_texture = false;
			for (const RHITextureHandle material_texture : slot.resource.textures) {
				if (material_texture == texture) {
					uses_texture = true;
					break;
				}
			}
			if (!uses_texture)
				continue;

			Release_Material(slot.resource);
			slot.generation = 0;
			slot.last_generation = 0;
		}
	}

	Device *m_device = nullptr;
	std::vector<MeshSlot> m_meshes;
	std::vector<TextureSlot> m_textures;
	std::vector<MaterialSlot> m_materials;
};

}
