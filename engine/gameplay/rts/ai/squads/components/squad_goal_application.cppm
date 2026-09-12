module;

#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.ai.squads.components.squad_goal_application;

export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;

export namespace engine::gameplay::rts::ai::squads
{

// Persistent one-shot activation latch.  The squad generation is part of the
// key so an entity-index reuse cannot inherit an old activation.
struct SquadGoalApplication
{
	ecs::Entity squad{};
	std::uint64_t appliedRevision{};
};

} // namespace engine::gameplay::rts::ai::squads

export namespace ecs
{

template<>
struct ComponentTraits<engine::gameplay::rts::ai::squads::SquadGoalApplication>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.ai.squads.goal_application";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

} // namespace ecs
