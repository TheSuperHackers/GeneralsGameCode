module;

#include <cstdint>
#include <string_view>
#include <type_traits>

export module games.generalszh.gameplay.upgrades.object_scope.components.object_upgrade_lifecycle;
export import engine.ecs.core.component_registry;
export import games.generalszh.gameplay.upgrades.object_scope.inputs.object_upgrade_creation;

export namespace generalszh::upgrades::object_scope
{
enum class ObjectUpgradeLifecycleStage : std::uint8_t
{
	Enrolling,
	AwaitingReadiness,
	RequestPublished,
	SuppressedDead
};

// The authored definition is retained with the real object until the
// producer-selected creation event has passed its visibility/readiness guard.
// Keeping the policy here prevents a runtime event from losing the
// UNDER_CONSTRUCTION exemption that justified it.
struct ObjectUpgradeLifecycle final
{
	ObjectUpgradeCreationDefinition definition{};
	ObjectUpgradeCreationEvent event{
		ObjectUpgradeCreationEvent::OnBuildCompleteAfterGuard};
	ObjectUpgradeLifecycleStage stage{
		ObjectUpgradeLifecycleStage::Enrolling};
};

static_assert(std::is_standard_layout_v<ObjectUpgradeLifecycle>);
static_assert(std::is_trivially_copyable_v<ObjectUpgradeLifecycle>);
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::upgrades::object_scope::ObjectUpgradeLifecycle>
{
	static constexpr std::string_view StableName =
		"games.generalszh.upgrades.object_scope.lifecycle";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
