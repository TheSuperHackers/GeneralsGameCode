module;

#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.ai.components.force_production_controller;

export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::ai
{

// Authoritative per-account decision state.  The immutable policy is owned by
// the game composition; this component stores only the account's runtime
// cursor and cooldown deadline.
struct ForceProductionController
{
	std::uint32_t policyKey{};
	std::uint64_t nextDecisionTick{};
	bool enabled{true};
};

} // namespace engine::gameplay::rts::ai

export namespace ecs
{

template<>
struct ComponentTraits<engine::gameplay::rts::ai::ForceProductionController>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.ai.force_production_controller";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

} // namespace ecs
