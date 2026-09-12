module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.rts.economy.components.income_dispatch;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::economy
{
// Derived execution-boundary input, separate from payment eligibility. A blocked
// source retains its deadline and emits no due pulse; time itself does not pause.
struct IncomeDispatch { bool enabled{true}; };
}
export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::economy::IncomeDispatch>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.economy.income_dispatch";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
