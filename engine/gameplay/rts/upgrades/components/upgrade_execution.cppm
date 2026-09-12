module;

#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.upgrades.components.upgrade_execution;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::upgrades
{
struct UpgradeExecutionState
{
	// Marker only; callers choose publication and reset does not undo applied effects.
	bool executed{false};
};
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::upgrades::UpgradeExecutionState>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.upgrades.execution_state";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
