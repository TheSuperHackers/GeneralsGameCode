module;

#include <cstdint>

export module Graphics.Resources.Samplers.Sampler;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Resources.Pools.ResourcePool;

namespace Graphics
{

export enum class SamplerFilter : std::uint8_t
{
	Nearest,
	Linear
};

export enum class SamplerAddressMode : std::uint8_t
{
	Repeat,
	MirroredRepeat,
	ClampToEdge,
	ClampToBorder
};

export struct Sampler final
{
	SamplerFilter min_filter = SamplerFilter::Linear;
	SamplerFilter mag_filter = SamplerFilter::Linear;
	SamplerFilter mip_filter = SamplerFilter::Linear;
	SamplerAddressMode address_u = SamplerAddressMode::Repeat;
	SamplerAddressMode address_v = SamplerAddressMode::Repeat;
	SamplerAddressMode address_w = SamplerAddressMode::Repeat;
};

export using SamplerPool = ResourcePool<Sampler, SamplerHandle>;

}
