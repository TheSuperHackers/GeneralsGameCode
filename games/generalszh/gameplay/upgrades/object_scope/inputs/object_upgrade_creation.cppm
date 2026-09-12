module;

#include <cstdint>
#include <stdexcept>

export module games.generalszh.gameplay.upgrades.object_scope.inputs.object_upgrade_creation;
export import engine.gameplay.rts.upgrades.object_scope.inputs.object_upgrade_batch;
export import games.generalszh.gameplay.upgrades.object_scope.definitions.object_upgrade_creation_definition;

export namespace generalszh::upgrades::object_scope
{
enum class ObjectUpgradeCreationEvent : std::uint8_t
{
	OnCreateWhenComplete,
	OnBuildCompleteAfterGuard
};
// Entity identity enters only at the spawned/completed-object boundary.
struct ObjectUpgradeCreationInput
{
	ecs::Entity target{};
	engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeBinding binding{};
	ObjectUpgradeCreationEvent event{ObjectUpgradeCreationEvent::OnBuildCompleteAfterGuard};
};

inline ObjectUpgradeCreationInput MakeObjectUpgradeCreationInput(
	const ecs::Entity spawnedOrCompletedObject,
	const ObjectUpgradeCreationDefinition &definition,
	const ObjectUpgradeCreationEvent event)
{
	if (!spawnedOrCompletedObject.IsValid())
		throw std::invalid_argument("Object upgrade creation input requires a live object handle");
	engine::gameplay::rts::upgrades::object_scope::ValidateCanonicalBinding(definition.binding);
	if (event == ObjectUpgradeCreationEvent::OnCreateWhenComplete &&
		!definition.policy.exemptUnderConstruction)
		throw std::invalid_argument(
			"Only an UNDER_CONSTRUCTION exemption may use the immediate object-create grant");
	if (event != ObjectUpgradeCreationEvent::OnCreateWhenComplete &&
		event != ObjectUpgradeCreationEvent::OnBuildCompleteAfterGuard)
		throw std::invalid_argument("Object upgrade creation event is not represented");
	return {spawnedOrCompletedObject, definition.binding, event};
}
}
