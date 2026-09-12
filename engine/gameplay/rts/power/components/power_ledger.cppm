module;
#include <cstdint>
#include <string_view>
#include <type_traits>

export module engine.gameplay.rts.power.components.power_ledger;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::power
{
using PowerValue = std::int32_t;

// The ledger is intentionally only the integral production/consumption state.
// Ownership, sabotage, eligibility, bonuses and notifications remain at the
// composition boundary that currently owns those policies.
struct PowerLedger
{
	PowerValue production{0};
	PowerValue consumption{0};
};

static_assert(std::is_standard_layout_v<PowerLedger>);
static_assert(std::is_trivially_copyable_v<PowerLedger>);
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::power::PowerLedger>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.power.power_ledger";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
