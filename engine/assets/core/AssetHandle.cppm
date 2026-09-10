module;

#include <cstddef>
#include <cstdint>

export module Assets.Handles;

namespace Assets
{

export struct ModelAssetHandleTag
{
};

export struct TextureAssetHandleTag
{
};

export struct AnimationAssetHandleTag
{
};

export struct MeshAssetHandleTag
{
};

export struct MaterialAssetHandleTag
{
};

export struct SkeletonAssetHandleTag
{
};

export struct FontAssetHandleTag
{
};

export template <typename Tag>
class AssetHandle final
{
public:
	using Index = std::uint32_t;
	using Generation = std::uint32_t;

	constexpr AssetHandle() noexcept = default;
	constexpr AssetHandle(std::nullptr_t) noexcept
	{
	}

	constexpr AssetHandle(Index index, Generation generation) noexcept
		: m_index(index),
		  m_generation(generation)
	{
	}

	static constexpr AssetHandle Invalid() noexcept
	{
		return {};
	}

	constexpr bool Is_Valid() const noexcept
	{
		return m_index != InvalidIndex && m_generation != 0;
	}

	constexpr explicit operator bool() const noexcept
	{
		return Is_Valid();
	}

	constexpr Index Get_Index() const noexcept
	{
		return m_index;
	}

	constexpr Generation Get_Generation() const noexcept
	{
		return m_generation;
	}

	friend constexpr bool operator==(const AssetHandle &left, const AssetHandle &right) noexcept
	{
		return left.m_index == right.m_index && left.m_generation == right.m_generation;
	}

	friend constexpr bool operator!=(const AssetHandle &left, const AssetHandle &right) noexcept
	{
		return !(left == right);
	}

	friend constexpr bool operator==(const AssetHandle &handle, std::nullptr_t) noexcept
	{
		return !handle.Is_Valid();
	}

	friend constexpr bool operator==(std::nullptr_t, const AssetHandle &handle) noexcept
	{
		return !handle.Is_Valid();
	}

	friend constexpr bool operator!=(const AssetHandle &handle, std::nullptr_t) noexcept
	{
		return handle.Is_Valid();
	}

	friend constexpr bool operator!=(std::nullptr_t, const AssetHandle &handle) noexcept
	{
		return handle.Is_Valid();
	}

private:
	static constexpr Index InvalidIndex = ~Index{0};

	Index m_index = InvalidIndex;
	Generation m_generation = 0;
};

export using ModelAssetHandle = AssetHandle<ModelAssetHandleTag>;
export using TextureAssetHandle = AssetHandle<TextureAssetHandleTag>;
export using AnimationAssetHandle = AssetHandle<AnimationAssetHandleTag>;
export using MeshAssetHandle = AssetHandle<MeshAssetHandleTag>;
export using MaterialAssetHandle = AssetHandle<MaterialAssetHandleTag>;
export using SkeletonAssetHandle = AssetHandle<SkeletonAssetHandleTag>;
export using FontAssetHandle = AssetHandle<FontAssetHandleTag>;

}
