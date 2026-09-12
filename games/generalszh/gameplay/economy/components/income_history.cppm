module;
#include <array>
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.economy.components.income_history;
export import engine.ecs.core.component_registry;

export namespace generalszh::economy
{
// Zero Hour's explicitly advanced gross-income ring, not a generic elapsed-time
// window: revisiting the current index deliberately does not age its contents.
struct IncomeHistory
{
	std::array<std::uint32_t, 60> buckets{};
	std::uint32_t current{0};
	std::uint32_t total{0};
};
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::economy::IncomeHistory>
{
	static constexpr std::string_view StableName = "games.generalszh.economy.income_history";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
