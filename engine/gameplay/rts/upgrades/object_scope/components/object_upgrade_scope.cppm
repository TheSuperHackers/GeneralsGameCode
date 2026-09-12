module;

#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.upgrades.object_scope.components.object_upgrade_scope;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;

export namespace engine::gameplay::rts::upgrades::object_scope
{
// This component is attached to the real object entity.  The catalog schema
// and word count are immutable after startup; completed bits live only in the
// owned word-row entities below.
struct ObjectUpgradeTarget
{
	std::uint32_t wordCount{};
	std::uint64_t schemaHash{};
};

// A word row is object-scoped only when this marker is present.  The target
// handle remains authoritative in UpgradeWordOwner; duplicating it here would
// create a second identity owner.
struct ObjectUpgradeWordMarker
{
};
}

export namespace ecs
{
template<>
struct ComponentTraits<engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeTarget>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.upgrades.object_scope.target";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<>
struct ComponentTraits<engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeWordMarker>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.upgrades.object_scope.word_marker";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
