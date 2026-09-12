module;
#include <cstddef>
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.economy.components.account_range;
export import engine.ecs.core.component_registry;

export namespace generalszh::economy
{
struct AccountRange { std::size_t first{}, count{}; };
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::economy::AccountRange>
{
	static constexpr std::string_view StableName = "games.generalszh.economy.account_range";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
