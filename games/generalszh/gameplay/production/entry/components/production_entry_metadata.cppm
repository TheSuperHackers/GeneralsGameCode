module;
#include <cstdint>
#include <string_view>

export module games.generalszh.gameplay.production.entry.components.production_entry_metadata;
export import engine.ecs.core.component_registry;

export namespace generalszh::production
{
enum class EntryKind : std::int32_t
{
	Invalid = 0,
	Unit = 1,
	Upgrade = 2
};

struct ProductionEntryKind
{
	EntryKind value{EntryKind::Invalid};
};

// Per-producer request correlation at the current input boundary. This value
// is not Entity identity, an accepted-command sequence, or a network schema key.
// No counter or allocation order is inferred from it.
struct ProductionCorrelation
{
	std::int32_t value{1};
};
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::production::ProductionEntryKind>
{
	static constexpr std::string_view StableName = "games.generalszh.production.entry_kind";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<generalszh::production::ProductionCorrelation>
{
	static constexpr std::string_view StableName = "games.generalszh.production.entry_correlation";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
