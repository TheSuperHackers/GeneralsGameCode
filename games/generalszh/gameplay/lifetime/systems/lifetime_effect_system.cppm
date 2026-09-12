module;
#include <cstddef>
#include <string_view>
export module games.generalszh.gameplay.lifetime.systems.lifetime_effect_system;
export import games.generalszh.gameplay.lifetime.components.lifetime_dispatch;
export import engine.gameplay.lifetime.systems.expiration_system;

export namespace generalszh::lifetime
{
struct LifetimeEffectSystem
{
	using Query = ecs::Query<ecs::Read<engine::gameplay::lifetime::Expiration>, ecs::Write<LifetimeDispatch>>;
	void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
	{
		const auto expiration = chunk.Get<engine::gameplay::lifetime::Expiration>();
		auto dispatch = chunk.Get<LifetimeDispatch>();
		for (std::size_t row = 0; row != dispatch.size(); ++row)
			dispatch[row].ready = expiration[row].state == engine::gameplay::lifetime::ExpirationState::Elapsed;
	}
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::lifetime::LifetimeEffectSystem>
{
	static constexpr std::string_view StableName = "games.generalszh.lifetime.effects";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<engine::gameplay::lifetime::ExpirationSystem>;
};
}
