module;

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <limits>
#include <span>

export module Graphics.Scene.Transparency;

export import Graphics.Resources.Materials.Material;
export import Graphics.Scene.DrawGeneration;
export import Graphics.Scene.LOD;
export import Graphics.Scene.RenderScene;
export import Graphics.Scene.Views.View;

namespace Graphics
{

export constexpr bool Is_Transparent_Material(MaterialFlags flags) noexcept
{
	return Has_Material_Flag(flags, MaterialFlags::Transparent)
		|| Has_Material_Flag(flags, MaterialFlags::Additive)
		|| Has_Material_Flag(flags, MaterialFlags::Multiply);
}

export constexpr bool Is_Transparent_Material(const Material &material) noexcept
{
	return Is_Transparent_Material(material.flags);
}

export PipelineDesc Make_Transparent_Pipeline(PipelineDesc description) noexcept
{
	description.depth_test = true;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Alpha;
	return description;
}

export class TransparentDrawSet final
{
public:
	explicit TransparentDrawSet(std::span<DrawData> storage) noexcept
		: m_storage(storage)
	{
	}

	void Clear() noexcept
	{
		m_count = 0;
	}

	std::size_t Size() const noexcept
	{
		return m_count;
	}

	std::span<const DrawData> Records() const noexcept
	{
		return {m_storage.data(), m_count};
	}

	std::span<DrawData> Mutable_Records() noexcept
	{
		return {m_storage.data(), m_count};
	}

private:
	friend bool Build_Transparent_Draw_Data(const RenderScene &, const MaterialPool &, const LODSet &, const View &, const GPUScene &, DrawPass, TransparentDrawSet &) noexcept;

	bool Try_Append(DrawData data) noexcept
	{
		if (m_count >= m_storage.size())
			return false;

		m_storage[m_count++] = data;
		return true;
	}

	void Sort() noexcept
	{
		std::sort(m_storage.begin(), m_storage.begin() + m_count, [](const DrawData &left, const DrawData &right) noexcept {
			if (left.sort_key != right.sort_key)
				return left.sort_key < right.sort_key;
			if (left.pipeline.Get_Index() != right.pipeline.Get_Index())
				return left.pipeline.Get_Index() < right.pipeline.Get_Index();
			if (left.pipeline.Get_Generation() != right.pipeline.Get_Generation())
				return left.pipeline.Get_Generation() < right.pipeline.Get_Generation();
			if (left.material_index != right.material_index)
				return left.material_index < right.material_index;
			if (left.mesh_index != right.mesh_index)
				return left.mesh_index < right.mesh_index;
			if (left.instance_count != right.instance_count)
				return left.instance_count < right.instance_count;
			return left.instance_index < right.instance_index;
		});
	}

	std::span<DrawData> m_storage;
	std::size_t m_count = 0;
};

export bool Build_Transparent_Draw_Data(
	const RenderScene &scene,
	const MaterialPool &materials,
	const LODSet &lod_set,
	const View &view,
	const GPUScene &gpu_scene,
	DrawPass pass,
	TransparentDrawSet &draw_set) noexcept;

PipelineHandle Select_Transparent_Pipeline(const DrawPass &pass, bool skinned, bool double_sided, bool alpha_test,
	std::uint32_t material_flags) noexcept;

namespace
{
std::uint32_t Depth_Sort_Key(const View &view, const RenderWorldBoundsData &bounds, std::size_t dense_index) noexcept
{
	const float view_z = view.view_matrix(2, 0) * bounds.center_x[dense_index]
		+ view.view_matrix(2, 1) * bounds.center_y[dense_index]
		+ view.view_matrix(2, 2) * bounds.center_z[dense_index]
		+ view.view_matrix(2, 3);
	const float depth = -view_z;
	if (!std::isfinite(depth) || depth <= 0.0f)
		return std::numeric_limits<std::uint32_t>::max();

	return std::numeric_limits<std::uint32_t>::max() - std::bit_cast<std::uint32_t>(depth);
}

std::uint64_t Make_Sort_Key(const View &view, const RenderWorldBoundsData &bounds, std::size_t dense_index, std::uint64_t pass_sort_key) noexcept
{
	return (static_cast<std::uint64_t>(Depth_Sort_Key(view, bounds, dense_index)) << 32)
		| static_cast<std::uint32_t>(pass_sort_key);
}
}

export bool Build_Transparent_Draw_Data(
	const RenderScene &scene,
	const MaterialPool &materials,
	const LODSet &lod_set,
	const View &view,
	const GPUScene &gpu_scene,
	DrawPass pass,
	TransparentDrawSet &draw_set) noexcept
{
	draw_set.Clear();
	if (!pass.pipeline.Is_Valid())
		return false;
	pass.transparent = true;

	const RenderSceneData scene_data = scene.Data();
	const std::span<const GPUInstanceData> instances = gpu_scene.Instances();
	const std::span<const GPUMaterialData> gpu_materials = gpu_scene.Materials();
	for (const LODSelection &selection : lod_set.Selections()) {
		const std::uint32_t scene_index = scene.Dense_Index(selection.instance);
		if (scene_index == Invalid_Render_Scene_Index || scene_index >= scene_data.Size())
			continue;

		const std::uint32_t mesh_index = gpu_scene.Mesh_Index(selection.mesh);
		const std::uint32_t instance_index = gpu_scene.Instance_Index(selection.instance);
		if (mesh_index == Invalid_GPU_Index || mesh_index >= gpu_scene.Meshes().size()
			|| instance_index == Invalid_GPU_Index || instance_index >= instances.size())
			continue;

		const GPUMeshData &mesh = gpu_scene.Meshes()[mesh_index];
		if (mesh.part_count == 0 || mesh.part_count > Max_Model_Part_Count
			|| static_cast<std::uint64_t>(mesh.part_offset) + mesh.part_count > gpu_scene.Mesh_Parts().size())
			continue;

		const bool double_sided = (instances[instance_index].flags
			& static_cast<std::uint32_t>(RenderInstanceFlags::DoubleSided)) != 0;
		const bool skinned = mesh.vertex_format == static_cast<std::uint32_t>(MeshVertexFormat::Position3Color4UV2Skinned);
		for (std::uint32_t submesh_index = 0; submesh_index < mesh.part_count; ++submesh_index) {
			const GPUMeshPartData &part = gpu_scene.Mesh_Parts()[mesh.part_offset + submesh_index];
			const std::uint32_t visibility_group = part.visibility_group == Invalid_Mesh_Part_Group
				? submesh_index : part.visibility_group;
			if (!Is_Submesh_Visible(instances[instance_index].visibility_mask, visibility_group))
				continue;
			const std::uint32_t material_index = part.material_index != Invalid_GPU_Index
				? part.material_index : instances[instance_index].material_index;
			if (material_index == Invalid_GPU_Index || material_index >= gpu_materials.size()
				|| !Is_Transparent_Material(static_cast<MaterialFlags>(gpu_materials[material_index].flags)))
				continue;

			const std::uint32_t material_flags = gpu_materials[material_index].flags;
			const bool alpha_test = (material_flags & static_cast<std::uint32_t>(MaterialFlags::AlphaTest)) != 0;
			const PipelineHandle pipeline = Select_Transparent_Pipeline(pass, skinned, double_sided, alpha_test, material_flags);
			if (!pipeline.Is_Valid())
				return false;

			if (!draw_set.Try_Append({
				mesh_index,
				material_index,
				instance_index,
				1,
				pipeline,
				Make_Sort_Key(view, scene_data.world_bounds, scene_index, pass.sort_key),
				submesh_index,
				0
			})) {
				draw_set.Clear();
				return false;
			}
		}
	}

	draw_set.Sort();
	return true;
}

PipelineHandle Select_Transparent_Pipeline(const DrawPass &pass, bool skinned, bool double_sided, bool alpha_test,
	std::uint32_t material_flags) noexcept
{
	const auto Select = [](PipelineHandle preferred, PipelineHandle fallback) noexcept {
		return preferred.Is_Valid() ? preferred : fallback;
	};
	const bool additive = (material_flags & static_cast<std::uint32_t>(MaterialFlags::Additive)) != 0;
	const bool multiply = (material_flags & static_cast<std::uint32_t>(MaterialFlags::Multiply)) != 0;
	if (double_sided) {
		if (skinned) {
			if (additive)
				return Select(pass.skinned_double_sided_additive_pipeline, pass.skinned_double_sided_transparent_pipeline);
			if (multiply)
				return Select(pass.skinned_double_sided_multiply_pipeline, pass.skinned_double_sided_transparent_pipeline);
			if (alpha_test && pass.skinned_double_sided_alpha_test_pipeline.Is_Valid())
				return pass.skinned_double_sided_alpha_test_pipeline;
			return Select(pass.skinned_double_sided_transparent_pipeline, pass.skinned_pipeline);
		}
		if (additive)
			return Select(pass.double_sided_additive_pipeline, pass.double_sided_transparent_pipeline);
		if (multiply)
			return Select(pass.double_sided_multiply_pipeline, pass.double_sided_transparent_pipeline);
		if (alpha_test && pass.double_sided_alpha_test_pipeline.Is_Valid())
			return pass.double_sided_alpha_test_pipeline;
		return Select(pass.double_sided_transparent_pipeline, pass.double_sided_pipeline);
	}
	if (skinned) {
		if (additive)
			return Select(pass.skinned_additive_pipeline, pass.skinned_transparent_pipeline);
		if (multiply)
			return Select(pass.skinned_multiply_pipeline, pass.skinned_transparent_pipeline);
		if (alpha_test && pass.skinned_alpha_test_pipeline.Is_Valid())
			return pass.skinned_alpha_test_pipeline;
		return Select(pass.skinned_transparent_pipeline, pass.skinned_pipeline);
	}
	if (additive)
		return Select(pass.additive_pipeline, pass.transparent_pipeline);
	if (multiply)
		return Select(pass.multiply_pipeline, pass.transparent_pipeline);
	if (alpha_test && pass.alpha_test_pipeline.Is_Valid())
		return pass.alpha_test_pipeline;
	return Select(pass.transparent_pipeline, pass.pipeline);
}

}
