module;

#include <cstddef>
#include <cstdint>
#include <limits>

export module Graphics.Resources.Handles.ResourceHandle;

namespace Graphics
{

export template <typename Tag>
class ResourceHandle final
{
public:
	using Index = std::uint32_t;
	using Generation = std::uint32_t;

	constexpr ResourceHandle() noexcept = default;
	constexpr ResourceHandle(std::nullptr_t) noexcept
	{
	}

	constexpr ResourceHandle(Index index, Generation generation) noexcept
		: m_index(index),
		  m_generation(generation)
	{
	}

	static constexpr ResourceHandle Invalid() noexcept
	{
		return {};
	}

	constexpr ResourceHandle &operator=(std::nullptr_t) noexcept
	{
		m_index = InvalidIndex;
		m_generation = 0;
		return *this;
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

	friend constexpr bool operator==(const ResourceHandle &left, const ResourceHandle &right) noexcept
	{
		return left.m_index == right.m_index && left.m_generation == right.m_generation;
	}

	friend constexpr bool operator!=(const ResourceHandle &left, const ResourceHandle &right) noexcept
	{
		return !(left == right);
	}

	friend constexpr bool operator==(const ResourceHandle &handle, std::nullptr_t) noexcept
	{
		return !handle.Is_Valid();
	}

	friend constexpr bool operator==(std::nullptr_t, const ResourceHandle &handle) noexcept
	{
		return !handle.Is_Valid();
	}

	friend constexpr bool operator!=(const ResourceHandle &handle, std::nullptr_t) noexcept
	{
		return handle.Is_Valid();
	}

	friend constexpr bool operator!=(std::nullptr_t, const ResourceHandle &handle) noexcept
	{
		return handle.Is_Valid();
	}

private:
	static constexpr Index InvalidIndex = std::numeric_limits<Index>::max();

	Index m_index = InvalidIndex;
	Generation m_generation = 0;
};

export struct MeshHandleTag
{
};

export struct TextureHandleTag
{
};

export struct MaterialHandleTag
{
};

export struct InstanceHandleTag
{
};

export struct SamplerHandleTag
{
};

export struct PipelineHandleTag
{
};

export struct ShaderHandleTag
{
};

export struct LightHandleTag
{
};

export struct DecalHandleTag
{
};

export struct ParticleEmitterHandleTag
{
};

export struct BeamHandleTag
{
};

export struct SkeletonHandleTag
{
};

export struct BoneHandleTag
{
};

export struct AttachmentHandleTag
{
};

export struct AttachmentLinkHandleTag
{
};

export struct AnimationClipHandleTag
{
};

export struct PoseHandleTag
{
};

export struct ResourceIndexTag
{
};

export using MeshHandle = ResourceHandle<MeshHandleTag>;
export using TextureHandle = ResourceHandle<TextureHandleTag>;
export using MaterialHandle = ResourceHandle<MaterialHandleTag>;
export using SamplerHandle = ResourceHandle<SamplerHandleTag>;
export using InstanceHandle = ResourceHandle<InstanceHandleTag>;
export using PipelineHandle = ResourceHandle<PipelineHandleTag>;
export using ShaderHandle = ResourceHandle<ShaderHandleTag>;
export using LightHandle = ResourceHandle<LightHandleTag>;
export using DecalHandle = ResourceHandle<DecalHandleTag>;
export using ParticleEmitterHandle = ResourceHandle<ParticleEmitterHandleTag>;
export using BeamHandle = ResourceHandle<BeamHandleTag>;
export using SkeletonHandle = ResourceHandle<SkeletonHandleTag>;
export using BoneHandle = ResourceHandle<BoneHandleTag>;
export using AttachmentHandle = ResourceHandle<AttachmentHandleTag>;
export using AttachmentLinkHandle = ResourceHandle<AttachmentLinkHandleTag>;
export using AnimationClipHandle = ResourceHandle<AnimationClipHandleTag>;
export using PoseHandle = ResourceHandle<PoseHandleTag>;
export using ResourceIndex = ResourceHandle<ResourceIndexTag>;

}
