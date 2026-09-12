module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.economy.components.income_account;
export import engine.ecs.core.entity;
export import engine.ecs.core.component_registry;

export namespace generalszh::economy
{
// Generation-safe modern relationship, not a native Player/Object identifier.
// A future serializer must encode/remap entity references explicitly.
struct IncomeAccount { ecs::Entity entity; };
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::economy::IncomeAccount>
{
	static constexpr std::string_view StableName = "games.generalszh.income.account";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
