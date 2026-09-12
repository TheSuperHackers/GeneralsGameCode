module;

#include <cstdint>
#include <string_view>

export module games.generalszh.gameplay.production.entry.components.exit_reservation;

export import engine.ecs.core.component_registry;

export namespace generalszh::production
{
enum class ExitReservation : std::int32_t
{
	NoneNeeded = -2,
	NoneAvailable = -1,
	Door1 = 0,
	Door2 = 1,
	Door3 = 2,
	Door4 = 3
};

// Named animation doors are not the full token domain. Parking exits assign
// nonnegative slot tokens from configuration, including values above Door4.

struct ProductionExitReservation
{
	ExitReservation value{ExitReservation::NoneAvailable};
};

constexpr bool IsValidExitReservation(const std::int32_t value) noexcept
{
	return value >= static_cast<std::int32_t>(ExitReservation::NoneNeeded);
}

inline void ClearExitReservation(ProductionExitReservation &reservation) noexcept
{
	reservation.value = ExitReservation::NoneAvailable;
}
}

export namespace ecs
{
template<>
struct ComponentTraits<generalszh::production::ProductionExitReservation>
{
	static constexpr std::string_view StableName =
		"games.generalszh.production.exit_reservation";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
