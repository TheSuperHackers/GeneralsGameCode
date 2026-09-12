module;

#include <cstdint>
#include <type_traits>

export module games.generalszh.gameplay.production.presentation.algorithms.construction_marker;

export import games.generalszh.gameplay.production.presentation.components.construction_marker;
export import games.generalszh.gameplay.production.presentation.definitions.construction_marker_timing;

export namespace generalszh::production
{
inline bool BeginConstructionMarker(ConstructionMarker &marker,
	const std::uint64_t now) noexcept
{
	if (marker.tick == 0)
	{
		marker.tick = now;
		return true;
	}

	return false;
}

template<typename Clock = std::uint64_t>
	requires (std::is_same_v<Clock, std::uint32_t> || std::is_same_v<Clock, std::uint64_t>)
inline bool AdvanceConstructionMarker(ConstructionMarker &marker,
	const std::type_identity_t<Clock> now,
	const ConstructionMarkerTiming timing) noexcept
{
	if (marker.tick != 0)
	{
		const Clock age = static_cast<Clock>(now - static_cast<Clock>(marker.tick));
		if (age > timing.duration)
		{
			marker.tick = 0;
			return true;
		}
	}

	return false;
}
}
