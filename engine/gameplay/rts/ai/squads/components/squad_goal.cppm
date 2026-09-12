module;

#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.ai.squads.components.squad_goal;

export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export import engine.gameplay.navigation.grid.navigation_grid;

export namespace engine::gameplay::rts::ai::squads
{

enum class SquadGoalKind : std::uint8_t
{
	None,
	Stop,
	AttackTarget,
	AttackMove,
	Hunt
};

// Authoritative state lives on the squad entity.  Members refer to this state
// through SquadMembership and never carry a copied goal.
struct SquadGoal
{
	SquadGoalKind kind{SquadGoalKind::None};
	ecs::Entity owner{};
	ecs::Entity target{};
	navigation::Cell destination{navigation::InvalidCell};
	std::uint32_t searchRadiusCells{};
	std::uint64_t revision{};
};

} // namespace engine::gameplay::rts::ai::squads

export namespace ecs
{

template<>
struct ComponentTraits<engine::gameplay::rts::ai::squads::SquadGoal>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.ai.squads.goal";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

} // namespace ecs
