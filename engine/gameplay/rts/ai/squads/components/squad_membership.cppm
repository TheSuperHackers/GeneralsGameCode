module;

#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.ai.squads.components.squad_membership;

export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;

export namespace engine::gameplay::rts::ai::squads
{

// A member stores only the generation-safe identity of its squad.  The member
// list is derived from this column; it is never duplicated on the squad.
struct SquadMembership
{
	ecs::Entity squad{};
};

} // namespace engine::gameplay::rts::ai::squads

export namespace ecs
{

template<>
struct ComponentTraits<engine::gameplay::rts::ai::squads::SquadMembership>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.ai.squads.membership";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

} // namespace ecs
