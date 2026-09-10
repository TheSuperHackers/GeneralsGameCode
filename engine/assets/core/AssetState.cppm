module;

#include <cstdint>

export module Assets.States;

namespace Assets
{

export enum class AssetState : std::uint8_t
{
	Unloaded,
	Loading,
	Ready,
	Failed
};

}
