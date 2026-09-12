module;
#include <cstddef>
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.economy.components.income_components;
export import games.generalszh.gameplay.economy.algorithms.auto_deposit;
export import engine.ecs.core.component_registry;

export namespace generalszh::economy
{
struct IncomeDefinition { std::int32_t base{}, captureBonus{}; bool actualMoney{true}; };
struct IncomeSource { std::uint32_t object{}, moduleTag{}; std::size_t slot{}; };
struct IncomeObservation
{
	std::int32_t player{-1}, boost{};
	bool active{false}, neutral{true}, constructionComplete{false}, visible{false}, dispatchEnabled{true};
};
struct IncomePayout { Payout value{}; };
}

export namespace ecs
{
#define ZH_INCOME_COMPONENT(Type, Name, Policy) \
template<> struct ComponentTraits<generalszh::economy::Type> { \
 static constexpr std::string_view StableName = Name; \
 static constexpr std::uint32_t Version = 1; \
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Policy; };
ZH_INCOME_COMPONENT(IncomeDefinition, "games.generalszh.income.definition", Serializable)
ZH_INCOME_COMPONENT(IncomeSource, "games.generalszh.income.source", Transient)
ZH_INCOME_COMPONENT(IncomeObservation, "games.generalszh.income.observation", Transient)
ZH_INCOME_COMPONENT(IncomePayout, "games.generalszh.income.payout", Transient)
#undef ZH_INCOME_COMPONENT
}
