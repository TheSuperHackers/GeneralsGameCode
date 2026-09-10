module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>

#if defined(RTS_PROFILE_TRACY)
#include <tracy/Tracy.hpp>
#define GRAPHICS_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#endif

export module Graphics.Scene.DrawGeneration;

export import Graphics.Scene.GPUScene;
export import Graphics.Scene.LOD;
export import Graphics.Shaders.Pipeline;

namespace Graphics
{

export struct DrawPass final
{
	std::uint32_t pass_key = 0;
	PipelineHandle pipeline{};
	std::uint64_t sort_key = 0;
	PipelineHandle skinned_pipeline{};
	PipelineHandle double_sided_pipeline{};
	PipelineHandle skinned_double_sided_pipeline{};
	PipelineHandle alpha_test_pipeline{};
	PipelineHandle transparent_pipeline{};
	PipelineHandle additive_pipeline{};
	PipelineHandle multiply_pipeline{};
	PipelineHandle skinned_alpha_test_pipeline{};
	PipelineHandle skinned_transparent_pipeline{};
	PipelineHandle skinned_additive_pipeline{};
	PipelineHandle skinned_multiply_pipeline{};
	PipelineHandle double_sided_alpha_test_pipeline{};
	PipelineHandle double_sided_transparent_pipeline{};
	PipelineHandle double_sided_additive_pipeline{};
	PipelineHandle double_sided_multiply_pipeline{};
	PipelineHandle skinned_double_sided_alpha_test_pipeline{};
	PipelineHandle skinned_double_sided_transparent_pipeline{};
	PipelineHandle skinned_double_sided_additive_pipeline{};
	PipelineHandle skinned_double_sided_multiply_pipeline{};
	bool transparent = false;
};

export struct alignas(16) DrawData final
{
	std::uint32_t mesh_index = Invalid_GPU_Index;
	std::uint32_t material_index = Invalid_GPU_Index;
	std::uint32_t instance_index = Invalid_GPU_Index;
	std::uint32_t instance_count = 1;
	PipelineHandle pipeline{};
	std::uint64_t sort_key = 0;
	std::uint32_t submesh_index = 0;
	std::uint32_t gpu_draw_index = Invalid_GPU_Index;
	std::uint32_t reserved = 0;
};

static_assert(sizeof(DrawData) == 48);

export class DrawSet;

PipelineHandle Select_Pipeline(const DrawPass &pass, bool skinned, bool double_sided, bool alpha_test,
	std::uint32_t material_flags) noexcept;

export bool Build_Draw_Data(const LODSet &lod_set, const GPUScene &gpu_scene, DrawPass pass, DrawSet &draw_set) noexcept;

export class DrawSet final
{
public:
	explicit DrawSet(std::span<DrawData> storage) noexcept
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
	friend bool Build_Draw_Data(const LODSet &lod_set, const GPUScene &gpu_scene, DrawPass pass, DrawSet &draw_set) noexcept;

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
			if (left.pipeline.Get_Index() != right.pipeline.Get_Index())
				return left.pipeline.Get_Index() < right.pipeline.Get_Index();
			if (left.pipeline.Get_Generation() != right.pipeline.Get_Generation())
				return left.pipeline.Get_Generation() < right.pipeline.Get_Generation();
			if (left.material_index != right.material_index)
				return left.material_index < right.material_index;
			if (left.mesh_index != right.mesh_index)
				return left.mesh_index < right.mesh_index;
			if (left.sort_key != right.sort_key)
				return left.sort_key < right.sort_key;
			if (left.submesh_index != right.submesh_index)
				return left.submesh_index < right.submesh_index;
			if (left.instance_count != right.instance_count)
				return left.instance_count < right.instance_count;
			return left.instance_index < right.instance_index;
		});
	}

	std::span<DrawData> m_storage;
	std::size_t m_count = 0;
};

export bool Build_Draw_Data(const LODSet &lod_set, const GPUScene &gpu_scene, DrawPass pass, DrawSet &draw_set) noexcept
{
	GRAPHICS_PROFILE_SCOPE("Graphics::Build_Draw_Data");
	draw_set.Clear();
	if (!pass.pipeline.Is_Valid())
		return false;

	const std::span<const GPUInstanceData> instances = gpu_scene.Instances();
	const std::span<const GPUMaterialData> materials = gpu_scene.Materials();

	for (const LODSelection &selection : lod_set.Selections()) {
		const std::uint32_t mesh_index = gpu_scene.Mesh_Index(selection.mesh);
		const std::uint32_t instance_index = gpu_scene.Instance_Index(selection.instance);
		if (mesh_index == Invalid_GPU_Index || mesh_index >= gpu_scene.Meshes().size()
			|| instance_index == Invalid_GPU_Index || instance_index >= instances.size())
			continue;

		const GPUMeshData &mesh = gpu_scene.Meshes()[mesh_index];
		if (mesh.part_count == 0 || mesh.part_count > Max_Model_Part_Count
			|| static_cast<std::uint64_t>(mesh.part_offset) + mesh.part_count > gpu_scene.Mesh_Parts().size())
			continue;
		const bool skinned = mesh.vertex_format == static_cast<std::uint32_t>(MeshVertexFormat::Position3Color4UV2Skinned);
		const bool double_sided = (instances[instance_index].flags
			& static_cast<std::uint32_t>(RenderInstanceFlags::DoubleSided)) != 0;

	for (std::uint32_t submesh_index = 0; submesh_index < mesh.part_count; ++submesh_index) {
			const GPUMeshPartData &part = gpu_scene.Mesh_Parts()[mesh.part_offset + submesh_index];
			const std::uint32_t visibility_group = part.visibility_group == Invalid_Mesh_Part_Group
				? submesh_index : part.visibility_group;
			if (!Is_Submesh_Visible(instances[instance_index].visibility_mask, visibility_group))
				continue;

			const std::uint32_t material_index = part.material_index != Invalid_GPU_Index
				? part.material_index : instances[instance_index].material_index;
			if (material_index == Invalid_GPU_Index || material_index >= materials.size())
				continue;

			const std::uint32_t material_flags = materials[material_index].flags;
			const bool is_transparent = (material_flags & (static_cast<std::uint32_t>(MaterialFlags::Transparent)
				| static_cast<std::uint32_t>(MaterialFlags::Additive)
				| static_cast<std::uint32_t>(MaterialFlags::Multiply))) != 0;
			if (is_transparent != pass.transparent)
				continue;
			const bool alpha_test = (material_flags & static_cast<std::uint32_t>(MaterialFlags::AlphaTest)) != 0;
			const PipelineHandle pipeline = Select_Pipeline(pass, skinned, double_sided, alpha_test, material_flags);
			if (!pipeline.Is_Valid())
				return false;
			if (!draw_set.Try_Append({
				mesh_index,
				material_index,
				instance_index,
				1,
				pipeline,
				pass.sort_key,
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

PipelineHandle Select_Pipeline(const DrawPass &pass, bool skinned, bool double_sided, bool alpha_test,
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
				return Select(pass.skinned_double_sided_additive_pipeline, pass.skinned_double_sided_pipeline);
			if (multiply)
				return Select(pass.skinned_double_sided_multiply_pipeline, pass.skinned_double_sided_pipeline);
			if (pass.transparent)
				return Select(pass.skinned_double_sided_transparent_pipeline, pass.skinned_double_sided_pipeline);
			if (alpha_test && pass.skinned_double_sided_alpha_test_pipeline.Is_Valid())
				return pass.skinned_double_sided_alpha_test_pipeline;
			return Select(pass.skinned_double_sided_pipeline, pass.skinned_pipeline);
		}
		if (additive)
			return Select(pass.double_sided_additive_pipeline, pass.double_sided_pipeline);
		if (multiply)
			return Select(pass.double_sided_multiply_pipeline, pass.double_sided_pipeline);
		if (pass.transparent)
			return Select(pass.double_sided_transparent_pipeline, pass.double_sided_pipeline);
		if (alpha_test && pass.double_sided_alpha_test_pipeline.Is_Valid())
			return pass.double_sided_alpha_test_pipeline;
		return Select(pass.double_sided_pipeline, pass.pipeline);
	}
	if (skinned) {
		if (additive)
			return Select(pass.skinned_additive_pipeline, pass.skinned_pipeline);
		if (multiply)
			return Select(pass.skinned_multiply_pipeline, pass.skinned_pipeline);
		if (pass.transparent)
			return Select(pass.skinned_transparent_pipeline, pass.skinned_pipeline);
		if (alpha_test && pass.skinned_alpha_test_pipeline.Is_Valid())
			return pass.skinned_alpha_test_pipeline;
		return Select(pass.skinned_pipeline, pass.pipeline);
	}
	if (additive)
		return Select(pass.additive_pipeline, pass.pipeline);
	if (multiply)
		return Select(pass.multiply_pipeline, pass.pipeline);
	if (pass.transparent)
		return Select(pass.transparent_pipeline, pass.pipeline);
	if (alpha_test && pass.alpha_test_pipeline.Is_Valid())
		return pass.alpha_test_pipeline;
	return pass.pipeline;
}

}
