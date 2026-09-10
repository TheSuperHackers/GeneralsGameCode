module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

export module Graphics.Resources.Meshes.Mesh;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Resources.Pools.ResourcePool;

namespace Graphics
{

export inline constexpr std::uint32_t Invalid_Mesh_Part_Group = ~std::uint32_t{0};

export enum class MeshIndexFormat : std::uint8_t
{
	None,
	UInt16,
	UInt32
};

export enum class MeshVertexFormat : std::uint8_t
{
	Position3Color4UV2,
	Position3Color4UV2Skinned
};

export struct MeshSkinningData final
{
	std::array<std::uint16_t, 4> bone_indices{};
	std::array<float, 4> bone_weights{};
};

static_assert(std::is_trivially_copyable_v<MeshSkinningData>);

export struct MeshPart final
{
	std::uint32_t first_index = 0;
	std::uint32_t index_count = 0;
	std::int32_t base_vertex = 0;
	MaterialHandle material{};
	std::uint32_t pass_key = 0;
	std::uint32_t visibility_group = Invalid_Mesh_Part_Group;
};

export struct MeshLod final
{
	MeshHandle mesh{};
	float max_screen_size = 0.0f;
};

export struct Mesh final
{
	using Count = std::uint32_t;
	using Stride = std::uint32_t;
	static constexpr std::uint8_t MaxLodCount = 4;

	Count vertex_count = 0;
	Count index_count = 0;
	Stride vertex_stride = 0;
	MeshIndexFormat index_format = MeshIndexFormat::None;
	std::array<MeshLod, MaxLodCount> lods{};
	std::uint8_t lod_count = 0;
	std::span<const std::byte> vertex_data{};
	std::span<const std::byte> index_data{};
	std::span<const MeshPart> parts{};
	std::uint32_t revision = 1;
	MeshVertexFormat vertex_format = MeshVertexFormat::Position3Color4UV2;
	std::uint32_t skin_bone_count = 0;

	void Mark_Dirty() noexcept
	{
		++revision;
		if (revision == 0)
			revision = 1;
	}
};

export using MeshPool = ResourcePool<Mesh, MeshHandle>;

}
