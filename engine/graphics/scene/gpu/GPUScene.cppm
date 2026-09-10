module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(RTS_PROFILE_TRACY)
#include <tracy/Tracy.hpp>
#define GRAPHICS_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#endif

export module Graphics.Scene.GPUScene;

export import Graphics.Resources.Materials.Material;
export import Graphics.Resources.Bindless.BindlessResourceTable;
export import Graphics.Resources.Meshes.Mesh;
export import Graphics.Resources.Samplers.Sampler;
export import Graphics.Resources.Textures.Texture;
export import Graphics.Scene.RenderScene;
export import Graphics.Scene.Lighting;
export import Graphics.Scene.Shadows;
export import Graphics.Scene.Decals;
export import Graphics.Scene.Models.Skinning;

import Graphics.Memory.AlignedAllocator;

namespace Graphics
{

export inline constexpr std::uint32_t Invalid_GPU_Index = std::numeric_limits<std::uint32_t>::max();

export struct alignas(16) GPUInstanceData final
{
	std::array<float, 16> transform{};
	std::array<float, 4> bounds{};
	std::uint32_t mesh_index = Invalid_GPU_Index;
	std::uint32_t material_index = Invalid_GPU_Index;
	std::uint32_t flags = 0;
	std::uint32_t visibility_mask = All_Submeshes_Visible;
	std::uint32_t bone_matrix_index = Invalid_Bone_Matrix_Index;
	std::uint32_t bone_matrix_count = 0;
	std::uint32_t bone_reserved0 = 0;
	std::uint32_t bone_reserved1 = 0;
};

export struct alignas(16) GPUMeshData final
{
	std::uint32_t vertex_count = 0;
	std::uint32_t index_count = 0;
	std::uint32_t vertex_stride = 0;
	std::uint32_t index_format = 0;
	std::uint32_t lod_count = 0;
	std::array<std::uint32_t, Mesh::MaxLodCount> lod_indices{};
	std::array<float, Mesh::MaxLodCount> lod_max_screen_sizes{};
	std::uint32_t part_offset = 0;
	std::uint32_t part_count = 0;
	std::uint32_t vertex_format = static_cast<std::uint32_t>(MeshVertexFormat::Position3Color4UV2);
};

export struct alignas(16) GPUMeshPartData final
{
	std::uint32_t first_index = 0;
	std::uint32_t index_count = 0;
	std::int32_t base_vertex = 0;
	std::uint32_t material_index = Invalid_GPU_Index;
	std::uint32_t pass_key = 0;
	std::uint32_t visibility_group = Invalid_Mesh_Part_Group;
	std::array<std::uint32_t, 2> reserved{};
};

export struct alignas(16) GPUMaterialData final
{
	std::array<float, MaterialParameterBlock::ValueCount> parameters{};
	std::array<std::uint32_t, Material::TextureSlotCount> texture_indices{};
	std::array<std::uint32_t, Material::SamplerSlotCount> sampler_indices{};
	std::uint32_t flags = 0;
	std::array<std::uint32_t, 3> reserved{};
};

export struct alignas(16) GPUDrawData final
{
	std::uint32_t instance_index = Invalid_GPU_Index;
	std::uint32_t material_index = Invalid_GPU_Index;
	std::uint32_t mesh_index = Invalid_GPU_Index;
	std::uint32_t flags = 0;
};

static_assert(sizeof(GPUInstanceData) == 112);
static_assert(sizeof(GPUMeshData) == 64);
static_assert(sizeof(GPUMeshPartData) == 32);
static_assert(sizeof(GPUMaterialData) == 144);
static_assert(sizeof(GPUDrawData) == 16);

export enum class GPUSceneTable : std::uint8_t
{
	Instances,
	Meshes,
	Materials,
	Lights,
	Shadows,
	Decals
};

export struct GPUSceneDirtyRange final
{
	GPUSceneTable table = GPUSceneTable::Instances;
	std::uint32_t first = 0;
	std::uint32_t count = 0;
};

template <typename Data, typename Handle>
class DenseTable final
{
public:
	using Index = typename Handle::Index;
	using Generation = typename Handle::Generation;

	static_assert(std::is_nothrow_copy_constructible_v<Data>);
	static_assert(std::is_nothrow_copy_assignable_v<Data>);
	static_assert(std::is_nothrow_move_assignable_v<Data>);

	void Reserve(std::size_t value_capacity, std::size_t slot_capacity)
	{
		m_values.reserve(value_capacity);
		m_dense_to_slot.reserve(value_capacity);
		if (slot_capacity > m_slots.size())
			m_slots.resize(slot_capacity);
	}

	void Clear() noexcept
	{
		m_values.clear();
		m_dense_to_slot.clear();
		for (Slot &slot : m_slots) {
			slot.dense_index = Invalid_GPU_Index;
			slot.generation = 0;
		}
	}

	std::size_t Size() const noexcept
	{
		return m_values.size();
	}

	std::span<const Data> Values() const noexcept
	{
		return m_values;
	}

	std::uint32_t Index_Of(Handle handle) const noexcept
	{
		if (!handle.Is_Valid())
			return Invalid_GPU_Index;

		const Index slot_index = handle.Get_Index();
		if (slot_index >= m_slots.size())
			return Invalid_GPU_Index;

		const Slot &slot = m_slots[slot_index];
		if (slot.dense_index == Invalid_GPU_Index || slot.generation != handle.Get_Generation())
			return Invalid_GPU_Index;

		return slot.dense_index;
	}

	bool Can_Upsert(Handle handle) const noexcept
	{
		if (!handle.Is_Valid())
			return false;

		const Index slot_index = handle.Get_Index();
		if (slot_index >= m_slots.size())
			return false;

		const Slot &slot = m_slots[slot_index];
		if (slot.dense_index != Invalid_GPU_Index)
			return slot.generation == handle.Get_Generation();

		return m_values.size() < m_values.capacity()
			&& m_values.size() < static_cast<std::size_t>(Invalid_GPU_Index);
	}

	bool Upsert(Handle handle, const Data &value) noexcept
	{
		if (!Can_Upsert(handle))
			return false;

		const Index slot_index = handle.Get_Index();
		Slot &slot = m_slots[slot_index];
		if (slot.dense_index != Invalid_GPU_Index) {
			m_values[slot.dense_index] = value;
			return true;
		}

		const std::uint32_t dense_index = static_cast<std::uint32_t>(m_values.size());
		m_values.push_back(value);
		m_dense_to_slot.push_back(slot_index);
		slot.dense_index = dense_index;
		slot.generation = handle.Get_Generation();
		return true;
	}

	bool Remove(Handle handle, std::uint32_t &dirty_index) noexcept
	{
		const std::uint32_t dense_index = Index_Of(handle);
		if (dense_index == Invalid_GPU_Index)
			return false;

		const std::uint32_t last_dense_index = static_cast<std::uint32_t>(m_values.size() - 1);
		if (dense_index != last_dense_index) {
			m_values[dense_index] = std::move(m_values[last_dense_index]);

			const Index moved_slot_index = m_dense_to_slot[last_dense_index];
			m_dense_to_slot[dense_index] = moved_slot_index;
			m_slots[moved_slot_index].dense_index = dense_index;
		}

		m_values.pop_back();
		m_dense_to_slot.pop_back();

		Slot &slot = m_slots[handle.Get_Index()];
		slot.dense_index = Invalid_GPU_Index;
		slot.generation = 0;
		dirty_index = dense_index;
		return true;
	}

private:
	struct Slot final
	{
		std::uint32_t dense_index = Invalid_GPU_Index;
		Generation generation = 0;
	};

	AlignedVector<Data> m_values;
	std::vector<Index> m_dense_to_slot;
	std::vector<Slot> m_slots;
};

template <typename Pool>
std::size_t Required_Slot_Capacity(const Pool &pool) noexcept
{
	std::size_t capacity = 0;
	pool.For_Each([&](auto handle, const auto &) noexcept {
		const std::size_t required = static_cast<std::size_t>(handle.Get_Index()) + 1;
		if (required > capacity)
			capacity = required;
	});
	return capacity;
}

GPUInstanceData Pack_Instance(const RenderTransformData &transforms, std::size_t dense_index, const RenderBoundsData &bounds, RenderInstanceFlags flags, SubmeshVisibilityMask visibility_mask, std::uint32_t mesh_index, std::uint32_t material_index, BoneMatrixRange bone_range = {}) noexcept
{
	GPUInstanceData data;
	for (std::size_t element = 0; element < data.transform.size(); ++element)
		data.transform[element] = transforms.elements[element][dense_index];
	data.bounds = {
		bounds.center[0][dense_index],
		bounds.center[1][dense_index],
		bounds.center[2][dense_index],
		bounds.radii[dense_index]
	};
	data.mesh_index = mesh_index;
	data.material_index = material_index;
	data.flags = static_cast<std::uint32_t>(flags);
	data.visibility_mask = visibility_mask;
	if (bone_range.Is_Valid()) {
		data.bone_matrix_index = bone_range.first_matrix;
		data.bone_matrix_count = bone_range.count;
	}
	return data;
}

GPUMeshData Pack_Mesh(const Mesh &mesh, const DenseTable<GPUMeshData, MeshHandle> &mesh_table,
	const DenseTable<GPUMaterialData, MaterialHandle> &material_table, std::uint32_t part_offset) noexcept
{
	GPUMeshData data;
	data.vertex_count = mesh.vertex_count;
	data.index_count = mesh.index_count;
	data.vertex_stride = mesh.vertex_stride;
	data.index_format = static_cast<std::uint32_t>(mesh.index_format);
	data.lod_count = mesh.lod_count < Mesh::MaxLodCount ? mesh.lod_count : Mesh::MaxLodCount;
	data.lod_indices.fill(Invalid_GPU_Index);

	for (std::uint32_t lod_index = 0; lod_index < data.lod_count; ++lod_index) {
		data.lod_indices[lod_index] = mesh_table.Index_Of(mesh.lods[lod_index].mesh);
		data.lod_max_screen_sizes[lod_index] = mesh.lods[lod_index].max_screen_size;
	}
	data.part_offset = part_offset;
	data.part_count = mesh.parts.empty() ? 1u : static_cast<std::uint32_t>(mesh.parts.size());
	data.vertex_format = static_cast<std::uint32_t>(mesh.vertex_format);

	return data;
}

GPUMaterialData Pack_Material(const Material &material, const TexturePool &textures, const SamplerPool &samplers,
	const BindlessResourceTable *bindless = nullptr) noexcept
{
	GPUMaterialData data;
	data.parameters = material.parameters.values;
	data.texture_indices.fill(Invalid_GPU_Index);
	data.sampler_indices.fill(Invalid_GPU_Index);
	data.flags = static_cast<std::uint32_t>(material.flags);

	for (std::size_t texture_index = 0; texture_index < material.textures.size(); ++texture_index) {
		const TextureHandle handle = material.textures[texture_index];
		if (textures.Resolve(handle) != nullptr) {
			data.texture_indices[texture_index] = bindless != nullptr
				? bindless->Texture_Index(handle).Get_Index()
				: handle.Get_Index();
		}
	}

	for (std::size_t sampler_index = 0; sampler_index < material.samplers.size(); ++sampler_index) {
		const SamplerHandle handle = material.samplers[sampler_index];
		if (samplers.Resolve(handle) != nullptr) {
			data.sampler_indices[sampler_index] = bindless != nullptr
				? bindless->Sampler_Index(handle).Get_Index()
				: handle.Get_Index();
		}
	}

	return data;
}

GPULightData Pack_Light(const RenderLightData &lights, std::size_t dense_index) noexcept
{
	GPULightData data;
	data.position_range = {
		lights.position_x[dense_index],
		lights.position_y[dense_index],
		lights.position_z[dense_index],
		lights.ranges[dense_index]
	};
	data.direction_intensity = {
		lights.direction_x[dense_index],
		lights.direction_y[dense_index],
		lights.direction_z[dense_index],
		lights.intensities[dense_index]
	};
	data.color_inner_angle = {
		lights.color_r[dense_index],
		lights.color_g[dense_index],
		lights.color_b[dense_index],
		lights.inner_angles[dense_index]
	};
	data.outer_angle = lights.outer_angles[dense_index];
	data.type = static_cast<std::uint32_t>(lights.types[dense_index]);
	data.flags = static_cast<std::uint32_t>(lights.flags[dense_index]);
	return data;
}

std::size_t Required_Light_Slot_Capacity(const RenderLightData &lights) noexcept
{
	std::size_t capacity = 0;
	for (const LightHandle handle : lights.handles) {
		const std::size_t required = static_cast<std::size_t>(handle.Get_Index()) + 1;
		if (required > capacity)
			capacity = required;
	}
	return capacity;
}

GPUDecalData Pack_Decal(const RenderDecalData &decals, std::size_t dense_index, const DenseTable<GPUMaterialData, MaterialHandle> &materials) noexcept
{
	GPUDecalData data;
	data.transform = decals.transforms[dense_index].matrix.values;
	data.bounds = {
		decals.bounds.center_x[dense_index],
		decals.bounds.center_y[dense_index],
		decals.bounds.center_z[dense_index],
		decals.bounds.radii[dense_index]
	};
	data.material_index = materials.Index_Of(decals.materials[dense_index]);
	data.flags = static_cast<std::uint32_t>(decals.flags[dense_index]);
	return data;
}

std::size_t Required_Decal_Slot_Capacity(const RenderDecalData &decals) noexcept
{
	std::size_t capacity = 0;
	for (const DecalHandle handle : decals.handles) {
		const std::size_t required = static_cast<std::size_t>(handle.Get_Index()) + 1;
		if (required > capacity)
			capacity = required;
	}
	return capacity;
}

export class GPUScene final
{
public:
	void Reserve(std::size_t instance_capacity, std::size_t mesh_capacity, std::size_t material_capacity, std::size_t dirty_range_capacity = 0, std::size_t light_capacity = 0, std::size_t decal_capacity = 0)
	{
		m_instances.Reserve(instance_capacity, instance_capacity);
		m_meshes.Reserve(mesh_capacity, mesh_capacity);
		m_materials.Reserve(material_capacity, material_capacity);
		m_mesh_parts.reserve(mesh_capacity * Max_Model_Part_Count);
		m_lights.Reserve(light_capacity, light_capacity);
		m_shadows.Reserve(light_capacity, light_capacity);
		m_decals.Reserve(decal_capacity, decal_capacity);
		if (dirty_range_capacity == 0)
			dirty_range_capacity = instance_capacity + mesh_capacity + material_capacity + light_capacity + decal_capacity + 4;
		m_dirty_ranges.reserve(dirty_range_capacity);
	}

	bool Build(const RenderScene &scene, const MeshPool &meshes, const TexturePool &textures, const SamplerPool &samplers,
		const MaterialPool &materials, const BoneMatrixTable *bone_matrices = nullptr,
		const BindlessResourceTable *bindless = nullptr)
	{
		GRAPHICS_PROFILE_SCOPE("Graphics::GPUScene::Build");
		m_instances.Clear();
		m_meshes.Clear();
		m_materials.Clear();
		m_mesh_parts.clear();
		m_lights.Clear();
		m_shadows.Clear();
		m_decals.Clear();
		m_dirty_ranges.clear();
		m_instances.Reserve(scene.Size(), Required_Slot_Capacity(scene));
		m_meshes.Reserve(meshes.Size(), Required_Slot_Capacity(meshes));
		m_materials.Reserve(materials.Size(), Required_Slot_Capacity(materials));
		const RenderLightData lights = scene.Lights();
		m_lights.Reserve(lights.Size(), Required_Light_Slot_Capacity(lights));
		m_shadows.Reserve(lights.Size(), Required_Light_Slot_Capacity(lights));
		const RenderDecalData decals = scene.Decals();
		m_decals.Reserve(decals.Size(), Required_Decal_Slot_Capacity(decals));
		m_dirty_ranges.reserve(scene.Size() + meshes.Size() + materials.Size() + lights.Size() + decals.Size() + 4);

		bool complete = true;
		materials.For_Each([&](MaterialHandle handle, const Material &material) noexcept {
			complete = m_materials.Upsert(handle, Pack_Material(material, textures, samplers, bindless)) && complete;
		});
		meshes.For_Each([&](MeshHandle handle, const Mesh &mesh) noexcept {
			const std::uint32_t part_offset = static_cast<std::uint32_t>(m_mesh_parts.size());
			if (mesh.parts.empty())
				m_mesh_parts.push_back({0, mesh.index_count, 0});
			else
				for (const MeshPart &part : mesh.parts)
					m_mesh_parts.push_back({part.first_index, part.index_count, part.base_vertex,
						m_materials.Index_Of(part.material), part.pass_key, part.visibility_group});
			complete = m_meshes.Upsert(handle, Pack_Mesh(mesh, m_meshes, m_materials, part_offset)) && complete;
		});
		for (std::size_t dense_index = 0; dense_index < lights.Size(); ++dense_index)
			complete = m_lights.Upsert(lights.handles[dense_index], Pack_Light(lights, dense_index)) && complete;
		for (std::size_t dense_index = 0; dense_index < decals.Size(); ++dense_index)
			complete = m_decals.Upsert(decals.handles[dense_index], Pack_Decal(decals, dense_index, m_materials)) && complete;
		const RenderSceneData scene_data = scene.Data();
		for (std::size_t dense_index = 0; dense_index < scene_data.Size(); ++dense_index) {
			const GPUInstanceData data = Pack_Instance(
				scene_data.transforms,
				dense_index,
				 scene_data.bounds,
				scene_data.flags[dense_index],
				scene_data.visibility_masks[dense_index],
				m_meshes.Index_Of(scene_data.meshes[dense_index]),
				m_materials.Index_Of(scene_data.materials[dense_index]),
				bone_matrices != nullptr ? bone_matrices->Range(scene_data.poses[dense_index]) : BoneMatrixRange{});
			complete = m_instances.Upsert(scene_data.handles[dense_index], data) && complete;
		}
		if (!complete)
			return false;

		Mark_Dirty(GPUSceneTable::Meshes, 0, m_meshes.Size());
		Mark_Dirty(GPUSceneTable::Materials, 0, m_materials.Size());
		Mark_Dirty(GPUSceneTable::Lights, 0, m_lights.Size());
		Mark_Dirty(GPUSceneTable::Decals, 0, m_decals.Size());
		Mark_Dirty(GPUSceneTable::Instances, 0, m_instances.Size());
		return true;
	}

	bool Sync_Instance(InstanceHandle handle, const RenderScene &scene, const BoneMatrixTable *bone_matrices = nullptr) noexcept
	{
		const std::uint32_t dense_index = scene.Dense_Index(handle);
		if (dense_index == Invalid_Render_Scene_Index || !m_instances.Can_Upsert(handle))
			return false;

		const std::uint32_t index = Existing_Or_Next_Index(m_instances, handle);
		if (!Can_Mark(GPUSceneTable::Instances, index))
			return false;

		const RenderSceneData scene_data = scene.Data();
		if (dense_index >= scene_data.Size())
			return false;

		const GPUInstanceData data = Pack_Instance(
			scene_data.transforms,
			dense_index,
			scene_data.bounds,
			scene_data.flags[dense_index],
			scene_data.visibility_masks[dense_index],
			m_meshes.Index_Of(scene_data.meshes[dense_index]),
			m_materials.Index_Of(scene_data.materials[dense_index]),
			bone_matrices != nullptr ? bone_matrices->Range(scene_data.poses[dense_index]) : BoneMatrixRange{});
		if (!m_instances.Upsert(handle, data))
			return false;

		Mark_Dirty(GPUSceneTable::Instances, index, 1);
		return true;
	}

	bool Sync_Mesh(MeshHandle handle, const MeshPool &meshes) noexcept
	{
		const Mesh *mesh = meshes.Resolve(handle);
		if (mesh == nullptr || !m_meshes.Can_Upsert(handle))
			return false;

		const std::uint32_t existing_index = m_meshes.Index_Of(handle);
		const std::uint32_t index = Existing_Or_Next_Index(m_meshes, handle);
		if (!Can_Mark(GPUSceneTable::Meshes, index))
			return false;

		const std::uint32_t part_count = mesh->parts.empty() ? 1u : static_cast<std::uint32_t>(mesh->parts.size());
		std::uint32_t part_offset = 0;
		if (existing_index != Invalid_GPU_Index) {
			const GPUMeshData &current = m_meshes.Values()[existing_index];
			if (part_count != current.part_count || static_cast<std::uint64_t>(current.part_offset) + part_count > m_mesh_parts.size())
				return false;
			part_offset = current.part_offset;
		} else {
			part_offset = static_cast<std::uint32_t>(m_mesh_parts.size());
			if (mesh->parts.empty())
				m_mesh_parts.push_back({0, mesh->index_count, 0});
			else
				for (const MeshPart &part : mesh->parts)
					m_mesh_parts.push_back({part.first_index, part.index_count, part.base_vertex,
						m_materials.Index_Of(part.material), part.pass_key, part.visibility_group});
		}

		if (existing_index != Invalid_GPU_Index) {
			if (mesh->parts.empty())
				m_mesh_parts[part_offset] = {0, mesh->index_count, 0};
			else
				for (std::uint32_t part_index = 0; part_index < part_count; ++part_index) {
					const MeshPart &part = mesh->parts[part_index];
					m_mesh_parts[part_offset + part_index] = {part.first_index, part.index_count, part.base_vertex,
						m_materials.Index_Of(part.material), part.pass_key, part.visibility_group};
				}
		}

		if (!m_meshes.Upsert(handle, Pack_Mesh(*mesh, m_meshes, m_materials, part_offset)))
			return false;

		Mark_Dirty(GPUSceneTable::Meshes, index, 1);
		return true;
	}

	bool Sync_Material(MaterialHandle handle, const MaterialPool &materials, const TexturePool &textures,
		const SamplerPool &samplers, const BindlessResourceTable *bindless = nullptr) noexcept
	{
		const Material *material = materials.Resolve(handle);
		if (material == nullptr || !m_materials.Can_Upsert(handle))
			return false;

		const std::uint32_t index = Existing_Or_Next_Index(m_materials, handle);
		if (!Can_Mark(GPUSceneTable::Materials, index))
			return false;

		if (!m_materials.Upsert(handle, Pack_Material(*material, textures, samplers, bindless)))
			return false;

		Mark_Dirty(GPUSceneTable::Materials, index, 1);
		return true;
	}

	bool Sync_Light(LightHandle handle, const RenderScene &scene) noexcept
	{
		const std::uint32_t dense_index = scene.Dense_Light_Index(handle);
		if (dense_index == Invalid_Render_Scene_Index || !m_lights.Can_Upsert(handle))
			return false;

		const std::uint32_t index = Existing_Or_Next_Index(m_lights, handle);
		if (!Can_Mark(GPUSceneTable::Lights, index))
			return false;

		const RenderLightData lights = scene.Lights();
		if (dense_index >= lights.Size())
			return false;
		if (!m_lights.Upsert(handle, Pack_Light(lights, dense_index)))
			return false;

		Mark_Dirty(GPUSceneTable::Lights, index, 1);
		return true;
	}

	bool Remove_Instance(InstanceHandle handle) noexcept
	{
		return Remove(m_instances, GPUSceneTable::Instances, handle);
	}

	bool Remove_Light(LightHandle handle) noexcept
	{
		return Remove(m_lights, GPUSceneTable::Lights, handle);
	}

	bool Sync_Shadow_Cascades(const ShadowCascades &cascades) noexcept
	{
		if (!cascades.light.Is_Valid() || cascades.count == 0 || cascades.count > Max_Shadow_Cascades)
			return false;

		const std::uint32_t light_index = m_lights.Index_Of(cascades.light);
		if (light_index == Invalid_GPU_Index)
			return false;

		const std::uint32_t shadow_index = Existing_Or_Next_Index(m_shadows, cascades.light);
		if (!m_shadows.Can_Upsert(cascades.light)
			|| !Can_Mark(GPUSceneTable::Shadows, shadow_index)
			|| !Can_Mark(GPUSceneTable::Lights, light_index))
			return false;

		if (!m_shadows.Upsert(cascades.light, cascades.Pack_GPU_Data(light_index)))
			return false;

		Mark_Dirty(GPUSceneTable::Shadows, shadow_index, 1);
		if (!Set_Light_Shadow_Data_Index(cascades.light, shadow_index))
			return false;

		return true;
	}

	bool Remove_Shadow_Cascades(LightHandle handle) noexcept
	{
		const std::uint32_t shadow_index = m_shadows.Index_Of(handle);
		if (shadow_index == Invalid_GPU_Index)
			return false;

		const std::uint32_t light_index = m_lights.Index_Of(handle);
		if (light_index != Invalid_GPU_Index) {
			if (!Can_Mark(GPUSceneTable::Lights, light_index))
				return false;

			GPULightData light = m_lights.Values()[light_index];
			light.shadow_data_index = Invalid_Shadow_Data_Index;
			if (!m_lights.Upsert(handle, light))
				return false;
		}

		if (!Can_Mark(GPUSceneTable::Shadows, shadow_index))
			return false;

		std::uint32_t removed_index = 0;
		if (!m_shadows.Remove(handle, removed_index))
			return false;

		if (light_index != Invalid_GPU_Index)
			Mark_Dirty(GPUSceneTable::Lights, light_index, 1);
		Mark_Dirty(GPUSceneTable::Shadows, removed_index, 1);
		return true;
	}

	bool Sync_Decal(DecalHandle handle, const RenderScene &scene) noexcept
	{
		const std::uint32_t dense_index = scene.Dense_Decal_Index(handle);
		if (dense_index == Invalid_Render_Scene_Index || !m_decals.Can_Upsert(handle))
			return false;

		const std::uint32_t index = Existing_Or_Next_Index(m_decals, handle);
		if (!Can_Mark(GPUSceneTable::Decals, index))
			return false;

		const RenderDecalData decals = scene.Decals();
		if (dense_index >= decals.Size())
			return false;
		if (!m_decals.Upsert(handle, Pack_Decal(decals, dense_index, m_materials)))
			return false;

		Mark_Dirty(GPUSceneTable::Decals, index, 1);
		return true;
	}

	bool Remove_Decal(DecalHandle handle) noexcept
	{
		return Remove(m_decals, GPUSceneTable::Decals, handle);
	}

	bool Set_Light_Shadow_Data_Index(LightHandle handle, std::uint32_t shadow_index) noexcept
	{
		const std::uint32_t light_index = m_lights.Index_Of(handle);
		if (light_index == Invalid_GPU_Index || !Can_Mark(GPUSceneTable::Lights, light_index))
			return false;

		GPULightData light = m_lights.Values()[light_index];
		light.shadow_data_index = shadow_index;
		if (!m_lights.Upsert(handle, light))
			return false;

		Mark_Dirty(GPUSceneTable::Lights, light_index, 1);
		return true;
	}

	std::uint32_t Instance_Index(InstanceHandle handle) const noexcept
	{
		return m_instances.Index_Of(handle);
	}

	std::uint32_t Mesh_Index(MeshHandle handle) const noexcept
	{
		return m_meshes.Index_Of(handle);
	}

	std::uint32_t Material_Index(MaterialHandle handle) const noexcept
	{
		return m_materials.Index_Of(handle);
	}

	std::uint32_t Light_Index(LightHandle handle) const noexcept
	{
		return m_lights.Index_Of(handle);
	}

	std::uint32_t Decal_Index(DecalHandle handle) const noexcept
	{
		return m_decals.Index_Of(handle);
	}

	std::span<const GPUInstanceData> Instances() const noexcept
	{
		return m_instances.Values();
	}

	std::span<const GPUMeshData> Meshes() const noexcept
	{
		return m_meshes.Values();
	}

	std::span<const GPUMeshPartData> Mesh_Parts() const noexcept
	{
		return m_mesh_parts;
	}

	std::span<const GPUMaterialData> Materials() const noexcept
	{
		return m_materials.Values();
	}

	std::span<const GPULightData> Lights() const noexcept
	{
		return m_lights.Values();
	}

	std::span<const GPUShadowData> Shadows() const noexcept
	{
		return m_shadows.Values();
	}

	std::span<const GPUDecalData> Decals() const noexcept
	{
		return m_decals.Values();
	}

	std::span<const GPUSceneDirtyRange> Dirty_Ranges() const noexcept
	{
		return m_dirty_ranges;
	}

	void Clear_Dirty() noexcept
	{
		m_dirty_ranges.clear();
	}

private:
	template <typename Data, typename Handle>
	static std::uint32_t Existing_Or_Next_Index(const DenseTable<Data, Handle> &table, Handle handle) noexcept
	{
		const std::uint32_t existing_index = table.Index_Of(handle);
		return existing_index == Invalid_GPU_Index
			? static_cast<std::uint32_t>(table.Size())
			: existing_index;
	}

	bool Can_Mark(GPUSceneTable table, std::uint32_t index) const noexcept
	{
		if (!m_dirty_ranges.empty()) {
			const GPUSceneDirtyRange &last = m_dirty_ranges.back();
			if (last.table == table && last.first + last.count == index)
				return true;
		}

		return m_dirty_ranges.size() < m_dirty_ranges.capacity();
	}

	void Mark_Dirty(GPUSceneTable table, std::size_t first, std::size_t count) noexcept
	{
		if (count == 0)
			return;

		const std::uint32_t dirty_first = static_cast<std::uint32_t>(first);
		const std::uint32_t dirty_count = static_cast<std::uint32_t>(count);
		if (!m_dirty_ranges.empty()) {
			GPUSceneDirtyRange &last = m_dirty_ranges.back();
			if (last.table == table && last.first + last.count == dirty_first) {
				last.count += dirty_count;
				return;
			}
		}

		m_dirty_ranges.push_back({table, dirty_first, dirty_count});
	}

	template <typename Data, typename Handle>
	bool Remove(DenseTable<Data, Handle> &table, GPUSceneTable table_type, Handle handle) noexcept
	{
		const std::uint32_t index = table.Index_Of(handle);
		if (index == Invalid_GPU_Index || !Can_Mark(table_type, index))
			return false;

		std::uint32_t dirty_index = 0;
		if (!table.Remove(handle, dirty_index))
			return false;

		Mark_Dirty(table_type, dirty_index, 1);
		return true;
	}

	DenseTable<GPUInstanceData, InstanceHandle> m_instances;
	DenseTable<GPUMeshData, MeshHandle> m_meshes;
	std::vector<GPUMeshPartData> m_mesh_parts;
	DenseTable<GPUMaterialData, MaterialHandle> m_materials;
	DenseTable<GPULightData, LightHandle> m_lights;
	DenseTable<GPUShadowData, LightHandle> m_shadows;
	DenseTable<GPUDecalData, DecalHandle> m_decals;
	std::vector<GPUSceneDirtyRange> m_dirty_ranges;
};

}
