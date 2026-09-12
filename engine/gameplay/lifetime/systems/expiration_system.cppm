module;
#include <cstddef>
#include <string_view>

export module engine.gameplay.lifetime.systems.expiration_system;
export import engine.gameplay.lifetime.components.expiration;
export import engine.gameplay.lifetime.components.expiration_eligibility;
export import engine.ecs.system.system;

export namespace engine::gameplay::lifetime
{
struct ExpirationSystem
{
	using Query = ecs::Query<ecs::Write<Expiration>, ecs::Optional<ExpirationEligibility>>;

	void Execute(Query::Chunk chunk, ecs::SystemContext &context) const noexcept
	{
		const auto tick = context.Tick();
		auto expirations = chunk.Get<Expiration>();
		const auto eligibility = chunk.Get<ExpirationEligibility>();
		// Optional-column selection is once per chunk. Ungated archetypes retain
		// the original contiguous loop, without a per-row optional lookup/check.
		if (eligibility.empty())
		{
			for (Expiration &expiration : expirations)
				if (expiration.state == ExpirationState::Armed && expiration.deadline <= tick)
					expiration.state = ExpirationState::Elapsed;
		}
		else
		{
			for (std::size_t row = 0; row < expirations.size(); ++row)
				if (eligibility[row].enabled && expirations[row].state == ExpirationState::Armed
					&& expirations[row].deadline <= tick)
					expirations[row].state = ExpirationState::Elapsed;
		}
	}
};
}

export namespace ecs
{
template<>
struct SystemTraits<engine::gameplay::lifetime::ExpirationSystem>
{
	static constexpr std::string_view StableName = "engine.gameplay.lifetime.expiration_system";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};
}
