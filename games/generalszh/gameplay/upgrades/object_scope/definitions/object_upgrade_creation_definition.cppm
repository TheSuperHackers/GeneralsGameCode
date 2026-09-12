module;

export module games.generalszh.gameplay.upgrades.object_scope.definitions.object_upgrade_creation_definition;
export import engine.gameplay.rts.upgrades.object_scope.inputs.object_upgrade_batch;

export namespace generalszh::upgrades::object_scope
{
// Immutable authored policy. It records only the GrantUpgradeCreate
// exemption represented by this bounded slice; the production lifecycle guard
// remains a separate runtime decision.
struct ObjectUpgradeCreationPolicy
{
	bool exemptUnderConstruction{};
};

struct ObjectUpgradeCreationDefinition
{
	engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeBinding binding{};
	ObjectUpgradeCreationPolicy policy{};
};
}
