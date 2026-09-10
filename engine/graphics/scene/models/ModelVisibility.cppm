module;

#include <cstdint>

export module Graphics.Scene.Models.ModelVisibility;

namespace Graphics
{

export using ModelPartId = std::uint32_t;
export using SubmeshVisibilityMask = std::uint32_t;

export inline constexpr ModelPartId Max_Model_Part_Count = 32;
export inline constexpr SubmeshVisibilityMask All_Submeshes_Visible = ~SubmeshVisibilityMask{0};

export constexpr bool Is_Submesh_Visible(SubmeshVisibilityMask mask, ModelPartId part) noexcept
{
	return part < Max_Model_Part_Count && (mask & (SubmeshVisibilityMask{1} << part)) != 0;
}

export constexpr SubmeshVisibilityMask Set_Submesh_Visible(SubmeshVisibilityMask mask, ModelPartId part, bool visible) noexcept
{
	if (part >= Max_Model_Part_Count)
		return mask;

	const SubmeshVisibilityMask bit = SubmeshVisibilityMask{1} << part;
	return visible ? mask | bit : mask & ~bit;
}

}
