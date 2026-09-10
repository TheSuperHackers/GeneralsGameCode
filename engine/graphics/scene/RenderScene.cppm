module;

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <utility>
#include <vector>

export module Graphics.Scene.RenderScene;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Resources.Meshes.Mesh;
export import Graphics.Resources.Materials.Material;
export import Graphics.Scene.Decals;
export import Graphics.Scene.Lighting;
export import Graphics.Scene.Models.ModelVisibility;

import Graphics.Memory.AlignedAllocator;

namespace Graphics
{

export struct alignas(16) RenderTransform final
{
	std::array<float, 16> matrix{};
};

export struct alignas(16) RenderBounds final
{
	std::array<float, 3> center{};
	float radius = 0.0f;
};

export struct alignas(16) RenderWorldBounds final
{
	float center_x = 0.0f;
	float center_y = 0.0f;
	float center_z = 0.0f;
	float radius = 0.0f;
};

export struct RenderBoundsCenterView final
{
	const std::array<std::span<const float>, 3> *components = nullptr;
	std::size_t dense_index = 0;

	float operator[](std::size_t component) const noexcept
	{
		return (*components)[component][dense_index];
	}
};

export struct RenderBoundsView final
{
	RenderBoundsCenterView center{};
	float radius = 0.0f;
};

export struct RenderBoundsData final
{
	std::array<std::span<const float>, 3> center{};
	std::span<const float> radii{};

	std::size_t Size() const noexcept
	{
		return center[0].size();
	}

	RenderBoundsView operator[](std::size_t dense_index) const noexcept
	{
		return {RenderBoundsCenterView{&center, dense_index}, radii[dense_index]};
	}
};

export struct RenderWorldBoundsData final
{
	std::span<const float> center_x{};
	std::span<const float> center_y{};
	std::span<const float> center_z{};
	std::span<const float> radii{};

	std::size_t Size() const noexcept
	{
		return center_x.size();
	}

	RenderWorldBounds operator[](std::size_t dense_index) const noexcept
	{
		return {
			center_x[dense_index],
			center_y[dense_index],
			center_z[dense_index],
			radii[dense_index]
		};
	}
};

namespace
{
RenderWorldBounds Build_World_Bounds(const RenderTransform &transform, const RenderBounds &bounds) noexcept
{
	const std::array<float, 16> &matrix = transform.matrix;
	const std::array<float, 3> &local_center = bounds.center;

	RenderWorldBounds world_bounds;
	world_bounds.center_x = matrix[0] * local_center[0] + matrix[1] * local_center[1] + matrix[2] * local_center[2] + matrix[3];
	world_bounds.center_y = matrix[4] * local_center[0] + matrix[5] * local_center[1] + matrix[6] * local_center[2] + matrix[7];
	world_bounds.center_z = matrix[8] * local_center[0] + matrix[9] * local_center[1] + matrix[10] * local_center[2] + matrix[11];

	const float x_scale_squared = matrix[0] * matrix[0] + matrix[4] * matrix[4] + matrix[8] * matrix[8];
	const float y_scale_squared = matrix[1] * matrix[1] + matrix[5] * matrix[5] + matrix[9] * matrix[9];
	const float z_scale_squared = matrix[2] * matrix[2] + matrix[6] * matrix[6] + matrix[10] * matrix[10];
	float max_scale_squared = x_scale_squared;
	if (y_scale_squared > max_scale_squared)
		max_scale_squared = y_scale_squared;
	if (z_scale_squared > max_scale_squared)
		max_scale_squared = z_scale_squared;

	world_bounds.radius = bounds.radius * std::sqrt(max_scale_squared);
	return world_bounds;
}
}

export enum class RenderInstanceFlags : std::uint32_t
{
	None = 0,
	CastsShadow = 1u << 0,
	ReceivesShadow = 1u << 1,
	Hidden = 1u << 2,
	DoubleSided = 1u << 3
};

export constexpr RenderInstanceFlags operator|(RenderInstanceFlags left, RenderInstanceFlags right) noexcept
{
	return static_cast<RenderInstanceFlags>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

export constexpr RenderInstanceFlags operator&(RenderInstanceFlags left, RenderInstanceFlags right) noexcept
{
	return static_cast<RenderInstanceFlags>(static_cast<std::uint32_t>(left) & static_cast<std::uint32_t>(right));
}

export constexpr bool Has_Render_Instance_Flag(RenderInstanceFlags flags, RenderInstanceFlags flag) noexcept
{
	return (flags & flag) == flag;
}

export constexpr bool Render_Instance_Casts_Shadow(RenderInstanceFlags flags) noexcept
{
	return Has_Render_Instance_Flag(flags, RenderInstanceFlags::CastsShadow);
}

export constexpr RenderInstanceFlags Set_Render_Instance_Casts_Shadow(
	RenderInstanceFlags flags, bool enabled) noexcept
{
	return enabled
		? flags | RenderInstanceFlags::CastsShadow
		: static_cast<RenderInstanceFlags>(static_cast<std::uint32_t>(flags)
			& ~static_cast<std::uint32_t>(RenderInstanceFlags::CastsShadow));
}

export struct RenderInstance final
{
	RenderTransform transform{};
	RenderBounds bounds{};
	MeshHandle mesh{};
	MaterialHandle material{};
	SkeletonHandle skeleton{};
	PoseHandle pose{};
	RenderInstanceFlags flags = RenderInstanceFlags::None;
	SubmeshVisibilityMask visibility_mask = All_Submeshes_Visible;
};

export struct RenderTransformMatrixView final
{
	const std::array<std::span<const float>, 16> *elements = nullptr;
	std::size_t dense_index = 0;

	float operator[](std::size_t element) const noexcept
	{
		return (*elements)[element][dense_index];
	}
};

export struct RenderTransformView final
{
	RenderTransformMatrixView matrix{};
};

export struct RenderTransformData final
{
	std::array<std::span<const float>, 16> elements{};

	std::size_t Size() const noexcept
	{
		return elements[0].size();
	}

	RenderTransformView operator[](std::size_t dense_index) const noexcept
	{
		return {RenderTransformMatrixView{&elements, dense_index}};
	}
};

export struct RenderInstanceView final
{
	RenderTransform transform{};
	RenderBounds bounds{};
	RenderWorldBounds world_bounds{};
	const MeshHandle &mesh;
	const MaterialHandle &material;
	const SkeletonHandle &skeleton;
	const PoseHandle &pose;
	const RenderInstanceFlags &flags;
	const SubmeshVisibilityMask &visibility_mask;
};

export inline constexpr std::uint32_t Invalid_Render_Scene_Index = std::numeric_limits<std::uint32_t>::max();

export struct RenderSceneData final
{
	RenderTransformData transforms{};
	RenderBoundsData bounds{};
	RenderWorldBoundsData world_bounds{};
	std::span<const MeshHandle> meshes{};
	std::span<const MaterialHandle> materials{};
	std::span<const SkeletonHandle> skeletons{};
	std::span<const PoseHandle> poses{};
	std::span<const RenderInstanceFlags> flags{};
	std::span<const SubmeshVisibilityMask> visibility_masks{};
	std::span<const InstanceHandle> handles{};

	std::size_t Size() const noexcept
	{
		return transforms.Size();
	}
};

export class RenderScene final
{
public:
	void Reserve(std::size_t capacity)
	{
		for (HotFloatVector &column : m_transform_columns)
			column.reserve(capacity);
		for (HotFloatVector &column : m_bounds_columns)
			column.reserve(capacity);
		for (HotFloatVector &column : m_world_bounds_columns)
			column.reserve(capacity);
		m_meshes.reserve(capacity);
		m_materials.reserve(capacity);
		m_skeletons.reserve(capacity);
		m_poses.reserve(capacity);
		m_flags.reserve(capacity);
		m_visibility_masks.reserve(capacity);
		m_dense_handles.reserve(capacity);
		m_slots.reserve(capacity);
	}

	void Reserve_Lights(std::size_t capacity)
	{
		m_light_types.reserve(capacity);
		m_light_flags.reserve(capacity);
		m_light_position_x.reserve(capacity);
		m_light_position_y.reserve(capacity);
		m_light_position_z.reserve(capacity);
		m_light_direction_x.reserve(capacity);
		m_light_direction_y.reserve(capacity);
		m_light_direction_z.reserve(capacity);
		m_light_color_r.reserve(capacity);
		m_light_color_g.reserve(capacity);
		m_light_color_b.reserve(capacity);
		m_light_intensities.reserve(capacity);
		m_light_ranges.reserve(capacity);
		m_light_inner_angles.reserve(capacity);
		m_light_outer_angles.reserve(capacity);
		m_light_dense_handles.reserve(capacity);
		m_light_slots.reserve(capacity);
	}

	void Reserve_Decals(std::size_t capacity)
	{
		m_decal_transforms.reserve(capacity);
		for (HotFloatVector &column : m_decal_bounds_columns)
			column.reserve(capacity);
		m_decal_materials.reserve(capacity);
		m_decal_flags.reserve(capacity);
		m_decal_dense_handles.reserve(capacity);
		m_decal_slots.reserve(capacity);
	}

	InstanceHandle Create(const RenderInstance &instance = {})
	{
		Ensure_Instance_Capacity();

		const bool reuses_slot = m_free_head != Invalid_Render_Scene_Index;
		const Index slot_index = reuses_slot
			? m_free_head
			: static_cast<Index>(m_slots.size());
		const Index dense_index = static_cast<Index>(Size());

		for (std::size_t column = 0; column < m_transform_columns.size(); ++column)
			m_transform_columns[column].push_back(instance.transform.matrix[column]);
		for (std::size_t component = 0; component < instance.bounds.center.size(); ++component)
			m_bounds_columns[component].push_back(instance.bounds.center[component]);
		m_bounds_columns[3].push_back(instance.bounds.radius);
		const RenderWorldBounds world_bounds = Build_World_Bounds(instance.transform, instance.bounds);
		m_world_bounds_columns[0].push_back(world_bounds.center_x);
		m_world_bounds_columns[1].push_back(world_bounds.center_y);
		m_world_bounds_columns[2].push_back(world_bounds.center_z);
		m_world_bounds_columns[3].push_back(world_bounds.radius);
		m_meshes.push_back(instance.mesh);
		m_materials.push_back(instance.material);
		m_skeletons.push_back(instance.skeleton);
		m_poses.push_back(instance.pose);
		m_flags.push_back(instance.flags);
		m_visibility_masks.push_back(instance.visibility_mask);

		if (reuses_slot) {
			Slot &slot = m_slots[slot_index];
			m_free_head = slot.next_free;
			slot.next_free = Invalid_Render_Scene_Index;
			slot.dense_index = dense_index;
		} else {
			m_slots.push_back({dense_index, Invalid_Render_Scene_Index, 1});
		}

		m_dense_handles.emplace_back(slot_index, m_slots[slot_index].generation);
		return InstanceHandle(slot_index, m_slots[slot_index].generation);
	}

	bool Destroy(InstanceHandle handle) noexcept
	{
		if (!Is_Valid_Handle(handle))
			return false;

		const Index slot_index = handle.Get_Index();
		const Index dense_index = m_slots[slot_index].dense_index;
		const Index last_dense_index = static_cast<Index>(Size() - 1);

		if (dense_index != last_dense_index) {
			for (std::size_t column = 0; column < m_transform_columns.size(); ++column)
				m_transform_columns[column][dense_index] = m_transform_columns[column][last_dense_index];
			for (std::size_t column = 0; column < m_bounds_columns.size(); ++column) {
				m_bounds_columns[column][dense_index] = m_bounds_columns[column][last_dense_index];
				m_world_bounds_columns[column][dense_index] = m_world_bounds_columns[column][last_dense_index];
			}
			m_meshes[dense_index] = m_meshes[last_dense_index];
			m_materials[dense_index] = m_materials[last_dense_index];
			m_skeletons[dense_index] = m_skeletons[last_dense_index];
			m_poses[dense_index] = m_poses[last_dense_index];
			m_flags[dense_index] = m_flags[last_dense_index];
			m_visibility_masks[dense_index] = m_visibility_masks[last_dense_index];

			const Index moved_slot_index = m_dense_handles[last_dense_index].Get_Index();
			m_dense_handles[dense_index] = m_dense_handles[last_dense_index];
			m_slots[moved_slot_index].dense_index = dense_index;
		}

		for (HotFloatVector &column : m_transform_columns)
			column.pop_back();
		for (HotFloatVector &column : m_bounds_columns)
			column.pop_back();
		for (HotFloatVector &column : m_world_bounds_columns)
			column.pop_back();
		m_meshes.pop_back();
		m_materials.pop_back();
		m_skeletons.pop_back();
		m_poses.pop_back();
		m_flags.pop_back();
		m_visibility_masks.pop_back();
		m_dense_handles.pop_back();

		Slot &slot = m_slots[slot_index];
		slot.dense_index = Invalid_Render_Scene_Index;
		slot.generation = Next_Generation(slot.generation);
		slot.next_free = m_free_head;
		m_free_head = slot_index;
		return true;
	}

	bool Update(InstanceHandle handle, const RenderInstance &instance) noexcept
	{
		if (!Is_Valid_Handle(handle))
			return false;

		const Index dense_index = m_slots[handle.Get_Index()].dense_index;
		for (std::size_t column = 0; column < m_transform_columns.size(); ++column)
			m_transform_columns[column][dense_index] = instance.transform.matrix[column];
		for (std::size_t component = 0; component < instance.bounds.center.size(); ++component)
			m_bounds_columns[component][dense_index] = instance.bounds.center[component];
		m_bounds_columns[3][dense_index] = instance.bounds.radius;
		const RenderWorldBounds world_bounds = Build_World_Bounds(instance.transform, instance.bounds);
		m_world_bounds_columns[0][dense_index] = world_bounds.center_x;
		m_world_bounds_columns[1][dense_index] = world_bounds.center_y;
		m_world_bounds_columns[2][dense_index] = world_bounds.center_z;
		m_world_bounds_columns[3][dense_index] = world_bounds.radius;
		m_meshes[dense_index] = instance.mesh;
		m_materials[dense_index] = instance.material;
		m_skeletons[dense_index] = instance.skeleton;
		m_poses[dense_index] = instance.pose;
		m_flags[dense_index] = instance.flags;
		m_visibility_masks[dense_index] = instance.visibility_mask;
		return true;
	}

	bool Update_Transform(InstanceHandle handle, const RenderTransform &transform) noexcept
	{
		if (!Is_Valid_Handle(handle))
			return false;

		const std::uint32_t dense_index = m_slots[handle.Get_Index()].dense_index;
		for (std::size_t column = 0; column < m_transform_columns.size(); ++column)
			m_transform_columns[column][dense_index] = transform.matrix[column];
		const RenderBounds bounds{
			{
				m_bounds_columns[0][dense_index],
				m_bounds_columns[1][dense_index],
				m_bounds_columns[2][dense_index]
			},
			m_bounds_columns[3][dense_index]
		};
		const RenderWorldBounds world_bounds = Build_World_Bounds(transform, bounds);
		m_world_bounds_columns[0][dense_index] = world_bounds.center_x;
		m_world_bounds_columns[1][dense_index] = world_bounds.center_y;
		m_world_bounds_columns[2][dense_index] = world_bounds.center_z;
		m_world_bounds_columns[3][dense_index] = world_bounds.radius;
		return true;
	}

	bool Get_Transform(InstanceHandle handle, RenderTransform &transform) const noexcept
	{
		if (!Is_Valid_Handle(handle))
			return false;

		const std::uint32_t dense_index = m_slots[handle.Get_Index()].dense_index;
		for (std::size_t column = 0; column < m_transform_columns.size(); ++column)
			transform.matrix[column] = m_transform_columns[column][dense_index];
		return true;
	}

	bool Update_Visibility(InstanceHandle handle, SubmeshVisibilityMask visibility_mask) noexcept
	{
		if (!Is_Valid_Handle(handle))
			return false;

		m_visibility_masks[m_slots[handle.Get_Index()].dense_index] = visibility_mask;
		return true;
	}

	bool Update_Pose(InstanceHandle handle, PoseHandle pose) noexcept
	{
		if (!Is_Valid_Handle(handle))
			return false;

		m_poses[m_slots[handle.Get_Index()].dense_index] = pose;
		return true;
	}

	bool Update_Shadow_Casting(InstanceHandle handle, bool enabled) noexcept
	{
		if (!Is_Valid_Handle(handle))
			return false;

		const std::uint32_t dense_index = m_slots[handle.Get_Index()].dense_index;
		m_flags[dense_index] = Set_Render_Instance_Casts_Shadow(m_flags[dense_index], enabled);
		return true;
	}

	LightHandle Create_Light(const RenderLight &light = {})
	{
		Ensure_Light_Capacity();

		const bool reuses_slot = m_light_free_head != Invalid_Render_Scene_Index;
		const Index slot_index = reuses_slot
			? m_light_free_head
			: static_cast<Index>(m_light_slots.size());
		const Index dense_index = static_cast<Index>(m_light_types.size());

		m_light_types.push_back(light.type);
		m_light_flags.push_back(light.flags);
		m_light_position_x.push_back(light.position.x);
		m_light_position_y.push_back(light.position.y);
		m_light_position_z.push_back(light.position.z);
		m_light_direction_x.push_back(light.direction.x);
		m_light_direction_y.push_back(light.direction.y);
		m_light_direction_z.push_back(light.direction.z);
		m_light_color_r.push_back(light.color.x);
		m_light_color_g.push_back(light.color.y);
		m_light_color_b.push_back(light.color.z);
		m_light_intensities.push_back(light.intensity);
		m_light_ranges.push_back(light.range);
		m_light_inner_angles.push_back(light.inner_angle);
		m_light_outer_angles.push_back(light.outer_angle);

		if (reuses_slot) {
			LightSlot &slot = m_light_slots[slot_index];
			m_light_free_head = slot.next_free;
			slot.next_free = Invalid_Render_Scene_Index;
			slot.dense_index = dense_index;
		} else {
			m_light_slots.push_back({dense_index, Invalid_Render_Scene_Index, 1});
		}

		m_light_dense_handles.emplace_back(slot_index, m_light_slots[slot_index].generation);
		return LightHandle(slot_index, m_light_slots[slot_index].generation);
	}

	bool Destroy_Light(LightHandle handle) noexcept
	{
		if (!Is_Valid_Light_Handle(handle))
			return false;

		const Index slot_index = handle.Get_Index();
		const Index dense_index = m_light_slots[slot_index].dense_index;
		const Index last_dense_index = static_cast<Index>(m_light_types.size() - 1);

		if (dense_index != last_dense_index) {
			m_light_types[dense_index] = m_light_types[last_dense_index];
			m_light_flags[dense_index] = m_light_flags[last_dense_index];
			m_light_position_x[dense_index] = m_light_position_x[last_dense_index];
			m_light_position_y[dense_index] = m_light_position_y[last_dense_index];
			m_light_position_z[dense_index] = m_light_position_z[last_dense_index];
			m_light_direction_x[dense_index] = m_light_direction_x[last_dense_index];
			m_light_direction_y[dense_index] = m_light_direction_y[last_dense_index];
			m_light_direction_z[dense_index] = m_light_direction_z[last_dense_index];
			m_light_color_r[dense_index] = m_light_color_r[last_dense_index];
			m_light_color_g[dense_index] = m_light_color_g[last_dense_index];
			m_light_color_b[dense_index] = m_light_color_b[last_dense_index];
			m_light_intensities[dense_index] = m_light_intensities[last_dense_index];
			m_light_ranges[dense_index] = m_light_ranges[last_dense_index];
			m_light_inner_angles[dense_index] = m_light_inner_angles[last_dense_index];
			m_light_outer_angles[dense_index] = m_light_outer_angles[last_dense_index];

			const Index moved_slot_index = m_light_dense_handles[last_dense_index].Get_Index();
			m_light_dense_handles[dense_index] = m_light_dense_handles[last_dense_index];
			m_light_slots[moved_slot_index].dense_index = dense_index;
		}

		m_light_types.pop_back();
		m_light_flags.pop_back();
		m_light_position_x.pop_back();
		m_light_position_y.pop_back();
		m_light_position_z.pop_back();
		m_light_direction_x.pop_back();
		m_light_direction_y.pop_back();
		m_light_direction_z.pop_back();
		m_light_color_r.pop_back();
		m_light_color_g.pop_back();
		m_light_color_b.pop_back();
		m_light_intensities.pop_back();
		m_light_ranges.pop_back();
		m_light_inner_angles.pop_back();
		m_light_outer_angles.pop_back();
		m_light_dense_handles.pop_back();

		LightSlot &slot = m_light_slots[slot_index];
		slot.dense_index = Invalid_Render_Scene_Index;
		slot.generation = Next_Generation(slot.generation);
		slot.next_free = m_light_free_head;
		m_light_free_head = slot_index;
		return true;
	}

	bool Update_Light(LightHandle handle, const RenderLight &light) noexcept
	{
		if (!Is_Valid_Light_Handle(handle))
			return false;

		const Index dense_index = m_light_slots[handle.Get_Index()].dense_index;
		m_light_types[dense_index] = light.type;
		m_light_flags[dense_index] = light.flags;
		m_light_position_x[dense_index] = light.position.x;
		m_light_position_y[dense_index] = light.position.y;
		m_light_position_z[dense_index] = light.position.z;
		m_light_direction_x[dense_index] = light.direction.x;
		m_light_direction_y[dense_index] = light.direction.y;
		m_light_direction_z[dense_index] = light.direction.z;
		m_light_color_r[dense_index] = light.color.x;
		m_light_color_g[dense_index] = light.color.y;
		m_light_color_b[dense_index] = light.color.z;
		m_light_intensities[dense_index] = light.intensity;
		m_light_ranges[dense_index] = light.range;
		m_light_inner_angles[dense_index] = light.inner_angle;
		m_light_outer_angles[dense_index] = light.outer_angle;
		return true;
	}

	DecalHandle Create_Decal(const RenderDecal &decal = {})
	{
		Ensure_Decal_Capacity();

		const bool reuses_slot = m_decal_free_head != Invalid_Render_Scene_Index;
		const Index slot_index = reuses_slot
			? m_decal_free_head
			: static_cast<Index>(m_decal_slots.size());
		const Index dense_index = static_cast<Index>(m_decal_transforms.size());

		m_decal_transforms.push_back(decal.transform);
		m_decal_bounds_columns[0].push_back(decal.bounds.center.x);
		m_decal_bounds_columns[1].push_back(decal.bounds.center.y);
		m_decal_bounds_columns[2].push_back(decal.bounds.center.z);
		m_decal_bounds_columns[3].push_back(decal.bounds.radius);
		m_decal_materials.push_back(decal.material);
		m_decal_flags.push_back(decal.flags);

		if (reuses_slot) {
			DecalSlot &slot = m_decal_slots[slot_index];
			m_decal_free_head = slot.next_free;
			slot.next_free = Invalid_Render_Scene_Index;
			slot.dense_index = dense_index;
		} else {
			m_decal_slots.push_back({dense_index, Invalid_Render_Scene_Index, 1});
		}

		m_decal_dense_handles.emplace_back(slot_index, m_decal_slots[slot_index].generation);
		return DecalHandle(slot_index, m_decal_slots[slot_index].generation);
	}

	bool Destroy_Decal(DecalHandle handle) noexcept
	{
		if (!Is_Valid_Decal_Handle(handle))
			return false;

		const Index slot_index = handle.Get_Index();
		const Index dense_index = m_decal_slots[slot_index].dense_index;
		const Index last_dense_index = static_cast<Index>(m_decal_transforms.size() - 1);

		if (dense_index != last_dense_index) {
			m_decal_transforms[dense_index] = std::move(m_decal_transforms[last_dense_index]);
			for (HotFloatVector &column : m_decal_bounds_columns)
				column[dense_index] = column[last_dense_index];
			m_decal_materials[dense_index] = m_decal_materials[last_dense_index];
			m_decal_flags[dense_index] = m_decal_flags[last_dense_index];

			const Index moved_slot_index = m_decal_dense_handles[last_dense_index].Get_Index();
			m_decal_dense_handles[dense_index] = m_decal_dense_handles[last_dense_index];
			m_decal_slots[moved_slot_index].dense_index = dense_index;
		}

		m_decal_transforms.pop_back();
		for (HotFloatVector &column : m_decal_bounds_columns)
			column.pop_back();
		m_decal_materials.pop_back();
		m_decal_flags.pop_back();
		m_decal_dense_handles.pop_back();

		DecalSlot &slot = m_decal_slots[slot_index];
		slot.dense_index = Invalid_Render_Scene_Index;
		slot.generation = Next_Generation(slot.generation);
		slot.next_free = m_decal_free_head;
		m_decal_free_head = slot_index;
		return true;
	}

	bool Update_Decal(DecalHandle handle, const RenderDecal &decal) noexcept
	{
		if (!Is_Valid_Decal_Handle(handle))
			return false;

		const Index dense_index = m_decal_slots[handle.Get_Index()].dense_index;
		m_decal_transforms[dense_index] = decal.transform;
		m_decal_bounds_columns[0][dense_index] = decal.bounds.center.x;
		m_decal_bounds_columns[1][dense_index] = decal.bounds.center.y;
		m_decal_bounds_columns[2][dense_index] = decal.bounds.center.z;
		m_decal_bounds_columns[3][dense_index] = decal.bounds.radius;
		m_decal_materials[dense_index] = decal.material;
		m_decal_flags[dense_index] = decal.flags;
		return true;
	}

	std::uint32_t Dense_Decal_Index(DecalHandle handle) const noexcept
	{
		if (!Is_Valid_Decal_Handle(handle))
			return Invalid_Render_Scene_Index;

		return m_decal_slots[handle.Get_Index()].dense_index;
	}

	RenderDecalData Decals() const noexcept
	{
		return {
			m_decal_transforms,
			Make_Decal_Bounds_Data(),
			m_decal_materials,
			m_decal_flags,
			m_decal_dense_handles
		};
	}

	std::size_t Decal_Count() const noexcept
	{
		return m_decal_transforms.size();
	}

	template <typename Function>
	void For_Each_Decal(Function &&function) const noexcept
	{
		for (std::size_t dense_index = 0; dense_index < m_decal_transforms.size(); ++dense_index)
			function(m_decal_dense_handles[dense_index], Make_Decal_View(dense_index));
	}

	template <typename Function>
	bool Visit_Decal(DecalHandle handle, Function &&function) const noexcept
	{
		const std::uint32_t dense_index = Dense_Decal_Index(handle);
		if (dense_index == Invalid_Render_Scene_Index)
			return false;

		function(handle, Make_Decal_View(dense_index));
		return true;
	}

	std::uint32_t Dense_Light_Index(LightHandle handle) const noexcept
	{
		if (!Is_Valid_Light_Handle(handle))
			return Invalid_Render_Scene_Index;

		return m_light_slots[handle.Get_Index()].dense_index;
	}

	RenderLightData Lights() const noexcept
	{
		return {
			m_light_types,
			m_light_flags,
			m_light_position_x,
			m_light_position_y,
			m_light_position_z,
			m_light_direction_x,
			m_light_direction_y,
			m_light_direction_z,
			m_light_color_r,
			m_light_color_g,
			m_light_color_b,
			m_light_intensities,
			m_light_ranges,
			m_light_inner_angles,
			m_light_outer_angles,
			m_light_dense_handles
		};
	}

	std::size_t Light_Count() const noexcept
	{
		return m_light_types.size();
	}

	template <typename Function>
	void For_Each_Light(Function &&function) const noexcept
	{
		for (std::size_t dense_index = 0; dense_index < m_light_types.size(); ++dense_index)
			function(m_light_dense_handles[dense_index], Make_Light_View(dense_index));
	}

	template <typename Function>
	bool Visit_Light(LightHandle handle, Function &&function) const noexcept
	{
		const std::uint32_t dense_index = Dense_Light_Index(handle);
		if (dense_index == Invalid_Render_Scene_Index)
			return false;

		function(handle, Make_Light_View(dense_index));
		return true;
	}

	std::uint32_t Dense_Index(InstanceHandle handle) const noexcept
	{
		if (!Is_Valid_Handle(handle))
			return Invalid_Render_Scene_Index;

		return m_slots[handle.Get_Index()].dense_index;
	}

	RenderSceneData Data() const noexcept
	{
		return {
			Make_Transform_Data(),
			Make_Bounds_Data(),
			Make_World_Bounds_Data(),
			m_meshes,
			m_materials,
			m_skeletons,
			m_poses,
			m_flags,
			m_visibility_masks,
			m_dense_handles
		};
	}

	std::size_t Size() const noexcept
	{
		return m_transform_columns[0].size();
	}

	template <typename Function>
	void For_Each(Function &&function) const noexcept
	{
		for (std::size_t dense_index = 0; dense_index < Size(); ++dense_index)
			function(m_dense_handles[dense_index], Make_View(dense_index));
	}

	template <typename Function>
	bool Visit(InstanceHandle handle, Function &&function) const noexcept
	{
		const std::uint32_t dense_index = Dense_Index(handle);
		if (dense_index == Invalid_Render_Scene_Index)
			return false;

		function(handle, Make_View(dense_index));
		return true;
	}

private:
	using Index = InstanceHandle::Index;
	using Generation = InstanceHandle::Generation;

	struct Slot final
	{
		Index dense_index = Invalid_Render_Scene_Index;
		Index next_free = Invalid_Render_Scene_Index;
		Generation generation = 0;
	};

	static constexpr Generation Next_Generation(Generation generation) noexcept
	{
		const Generation next_generation = generation + 1;
		return next_generation == 0 ? 1 : next_generation;
	}

	static std::size_t Next_Capacity(std::size_t current_capacity) noexcept
	{
		return current_capacity == 0
			? 1
			: current_capacity > std::numeric_limits<std::size_t>::max() / 2
				? std::numeric_limits<std::size_t>::max()
				: current_capacity * 2;
	}

	void Ensure_Instance_Capacity()
	{
		if (Size() < m_transform_columns[0].capacity())
			return;

		Reserve(Next_Capacity(Size()));
	}

	void Ensure_Light_Capacity()
	{
		if (m_light_types.size() < m_light_types.capacity())
			return;

		Reserve_Lights(Next_Capacity(m_light_types.size()));
	}

	void Ensure_Decal_Capacity()
	{
		if (m_decal_transforms.size() < m_decal_transforms.capacity())
			return;

		Reserve_Decals(Next_Capacity(m_decal_transforms.size()));
	}

	bool Is_Valid_Handle(InstanceHandle handle) const noexcept
	{
		if (!handle.Is_Valid())
			return false;

		const Index slot_index = handle.Get_Index();
		if (slot_index >= m_slots.size())
			return false;

		const Slot &slot = m_slots[slot_index];
		return slot.dense_index != Invalid_Render_Scene_Index && slot.generation == handle.Get_Generation();
	}

	bool Is_Valid_Light_Handle(LightHandle handle) const noexcept
	{
		if (!handle.Is_Valid())
			return false;

		const Index slot_index = handle.Get_Index();
		if (slot_index >= m_light_slots.size())
			return false;

		const LightSlot &slot = m_light_slots[slot_index];
		return slot.dense_index != Invalid_Render_Scene_Index && slot.generation == handle.Get_Generation();
	}

	bool Is_Valid_Decal_Handle(DecalHandle handle) const noexcept
	{
		if (!handle.Is_Valid())
			return false;

		const Index slot_index = handle.Get_Index();
		if (slot_index >= m_decal_slots.size())
			return false;

		const DecalSlot &slot = m_decal_slots[slot_index];
		return slot.dense_index != Invalid_Render_Scene_Index && slot.generation == handle.Get_Generation();
	}

	RenderInstanceView Make_View(std::uint32_t dense_index) const noexcept
	{
		return {
			Make_Transform(dense_index),
			Make_Bounds(dense_index),
			Make_World_Bounds(dense_index),
			m_meshes[dense_index],
			m_materials[dense_index],
			m_skeletons[dense_index],
			m_poses[dense_index],
			m_flags[dense_index],
			m_visibility_masks[dense_index]
		};
	}

	RenderTransform Make_Transform(std::uint32_t dense_index) const noexcept
	{
		RenderTransform transform;
		for (std::size_t column = 0; column < m_transform_columns.size(); ++column)
			transform.matrix[column] = m_transform_columns[column][dense_index];
		return transform;
	}

	RenderTransformData Make_Transform_Data() const noexcept
	{
		RenderTransformData data;
		for (std::size_t column = 0; column < m_transform_columns.size(); ++column)
			data.elements[column] = m_transform_columns[column];
		return data;
	}

	RenderBounds Make_Bounds(std::uint32_t dense_index) const noexcept
	{
		return {
			{m_bounds_columns[0][dense_index], m_bounds_columns[1][dense_index], m_bounds_columns[2][dense_index]},
			m_bounds_columns[3][dense_index]
		};
	}

	RenderBoundsData Make_Bounds_Data() const noexcept
	{
		RenderBoundsData data;
		data.center[0] = m_bounds_columns[0];
		data.center[1] = m_bounds_columns[1];
		data.center[2] = m_bounds_columns[2];
		data.radii = m_bounds_columns[3];
		return data;
	}

	RenderWorldBounds Make_World_Bounds(std::uint32_t dense_index) const noexcept
	{
		return {
			m_world_bounds_columns[0][dense_index],
			m_world_bounds_columns[1][dense_index],
			m_world_bounds_columns[2][dense_index],
			m_world_bounds_columns[3][dense_index]
		};
	}

	RenderWorldBoundsData Make_World_Bounds_Data() const noexcept
	{
		return {
			m_world_bounds_columns[0],
			m_world_bounds_columns[1],
			m_world_bounds_columns[2],
			m_world_bounds_columns[3]
		};
	}

	RenderLightView Make_Light_View(std::uint32_t dense_index) const noexcept
	{
		return {
			m_light_dense_handles[dense_index],
			m_light_types[dense_index],
			m_light_flags[dense_index],
			{m_light_position_x[dense_index], m_light_position_y[dense_index], m_light_position_z[dense_index]},
			{m_light_direction_x[dense_index], m_light_direction_y[dense_index], m_light_direction_z[dense_index]},
			{m_light_color_r[dense_index], m_light_color_g[dense_index], m_light_color_b[dense_index]},
			m_light_intensities[dense_index],
			m_light_ranges[dense_index],
			m_light_inner_angles[dense_index],
			m_light_outer_angles[dense_index]
		};
	}

	RenderDecalView Make_Decal_View(std::uint32_t dense_index) const noexcept
	{
		return {
			m_decal_transforms[dense_index],
			{
				{
					m_decal_bounds_columns[0][dense_index],
					m_decal_bounds_columns[1][dense_index],
					m_decal_bounds_columns[2][dense_index]
				},
				m_decal_bounds_columns[3][dense_index]
			},
			m_decal_materials[dense_index],
			m_decal_flags[dense_index]
		};
	}

	DecalBoundsData Make_Decal_Bounds_Data() const noexcept
	{
		return {
			m_decal_bounds_columns[0],
			m_decal_bounds_columns[1],
			m_decal_bounds_columns[2],
			m_decal_bounds_columns[3]
		};
	}

	template <typename Type>
	using HotVector = AlignedVector<Type, 16>;

	using HotFloatVector = HotVector<float>;

	std::array<HotFloatVector, 16> m_transform_columns;
	std::array<HotFloatVector, 4> m_bounds_columns;
	std::array<HotFloatVector, 4> m_world_bounds_columns;
	HotVector<MeshHandle> m_meshes;
	HotVector<MaterialHandle> m_materials;
	HotVector<SkeletonHandle> m_skeletons;
	HotVector<PoseHandle> m_poses;
	HotVector<RenderInstanceFlags> m_flags;
	HotVector<SubmeshVisibilityMask> m_visibility_masks;
	HotVector<InstanceHandle> m_dense_handles;
	std::vector<Slot> m_slots;
	Index m_free_head = Invalid_Render_Scene_Index;

	struct LightSlot final
	{
		Index dense_index = Invalid_Render_Scene_Index;
		Index next_free = Invalid_Render_Scene_Index;
		Generation generation = 0;
	};

	HotVector<RenderLightType> m_light_types;
	HotVector<RenderLightFlags> m_light_flags;
	HotFloatVector m_light_position_x;
	HotFloatVector m_light_position_y;
	HotFloatVector m_light_position_z;
	HotFloatVector m_light_direction_x;
	HotFloatVector m_light_direction_y;
	HotFloatVector m_light_direction_z;
	HotFloatVector m_light_color_r;
	HotFloatVector m_light_color_g;
	HotFloatVector m_light_color_b;
	HotFloatVector m_light_intensities;
	HotFloatVector m_light_ranges;
	HotFloatVector m_light_inner_angles;
	HotFloatVector m_light_outer_angles;
	HotVector<LightHandle> m_light_dense_handles;
	std::vector<LightSlot> m_light_slots;
	Index m_light_free_head = Invalid_Render_Scene_Index;

	struct DecalSlot final
	{
		Index dense_index = Invalid_Render_Scene_Index;
		Index next_free = Invalid_Render_Scene_Index;
		Generation generation = 0;
	};

	HotVector<DecalTransform> m_decal_transforms;
	std::array<HotFloatVector, 4> m_decal_bounds_columns;
	HotVector<MaterialHandle> m_decal_materials;
	HotVector<RenderDecalFlags> m_decal_flags;
	HotVector<DecalHandle> m_decal_dense_handles;
	std::vector<DecalSlot> m_decal_slots;
	Index m_decal_free_head = Invalid_Render_Scene_Index;
};

}
