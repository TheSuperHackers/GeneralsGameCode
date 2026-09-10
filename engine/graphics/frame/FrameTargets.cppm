module;
#include <cstdint>

export module Graphics.FrameTargets;

export import Graphics.RHI;

namespace Graphics
{

export struct FrameTargets final
{
	RHIBackbuffer backbuffer{};
	RHIDepthTarget depth{};
	// Stable across rotating backbuffers; changes when the target set is replaced.
	// Zero denotes an individual texture without a logical target-set identity.
	std::uint64_t identity = 0;
};

}
