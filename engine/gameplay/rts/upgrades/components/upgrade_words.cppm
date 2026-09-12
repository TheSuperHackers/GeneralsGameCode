module;

#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.upgrades.components.upgrade_words;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;

export namespace engine::gameplay::rts::upgrades
{
struct UpgradeWordOwner
{
	ecs::Entity value{};
};

struct UpgradeWordOrdinal
{
	std::uint32_t value{};
};

struct CompletedUpgradeWord
{
	std::uint64_t value{};
};

struct InProgressUpgradeWord
{
	std::uint64_t value{};
};
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::upgrades::UpgradeWordOwner>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.upgrades.word_owner";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<> struct ComponentTraits<engine::gameplay::rts::upgrades::UpgradeWordOrdinal>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.upgrades.word_ordinal";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<> struct ComponentTraits<engine::gameplay::rts::upgrades::CompletedUpgradeWord>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.upgrades.completed_word";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<> struct ComponentTraits<engine::gameplay::rts::upgrades::InProgressUpgradeWord>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.upgrades.in_progress_word";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
