module;

#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>

export module engine.gameplay.rts.radar.algorithms.radar_availability;
export import engine.gameplay.rts.radar.components.radar_availability;

export namespace engine::gameplay::rts::radar
{
inline bool HasRadar(const RadarAvailability &availability) noexcept
{
	return !(availability.suppressed && availability.resistantProducers == 0)
		&& availability.producers > 0;
}

inline void AddProvider(RadarAvailability &availability, const bool resistant) noexcept
{
	// Internal count limits are debug-only programming/content invariants. The
	// unsigned operation and bit_cast keep invalid Release overflow defined.
	assert(availability.producers < (std::numeric_limits<std::int32_t>::max)());
	availability.producers = std::bit_cast<std::int32_t>(
		static_cast<std::uint32_t>(availability.producers) + std::uint32_t{1});

	if (resistant)
	{
		assert(availability.resistantProducers < (std::numeric_limits<std::int32_t>::max)());
		availability.resistantProducers = std::bit_cast<std::int32_t>(
			static_cast<std::uint32_t>(availability.resistantProducers) + std::uint32_t{1});
	}
}

inline void RemoveProvider(RadarAvailability &availability, const bool resistant) noexcept
{
	// This is the legacy precondition. In particular, do not add a separate
	// resistantProducers > 0 precondition: legacy removal only checks the
	// overall radar producer count.
	assert(availability.producers > 0);
	availability.producers = std::bit_cast<std::int32_t>(
		static_cast<std::uint32_t>(availability.producers) - std::uint32_t{1});

	if (resistant)
	{
		assert(availability.resistantProducers != (std::numeric_limits<std::int32_t>::min)());
		availability.resistantProducers = std::bit_cast<std::int32_t>(
			static_cast<std::uint32_t>(availability.resistantProducers) - std::uint32_t{1});
	}
}

inline void SetSuppressed(RadarAvailability &availability, const bool suppressed) noexcept
{
	availability.suppressed = suppressed;
}
}
