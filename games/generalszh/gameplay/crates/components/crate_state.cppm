module;

#include <cstdint>
#include <string_view>

export module games.generalszh.gameplay.crates.components.crate_state;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;

export namespace generalszh::crates
{
enum class CratePlacement : std::uint8_t
{
	Grounded,
	Airborne
};

// A crate owns its placement and authored definition binding.  The owner is
// intentionally present because legacy ForbidOwnerPlayer compares the crate's
// controlling player; no pre-existing modern crate owner exists to duplicate.
// Spawn seeds placement, and future physics is the only producer of changes.
struct CrateState final
{
	std::uint32_t definition{};
	ecs::Entity ownerAccount{};
	// VeterancyCrateCollide executes only for the crate's own AI goal object;
	// this full entity identity is the explicit spawn/order-owned equivalent.
	ecs::Entity intendedRecipient{};
	CratePlacement placement{CratePlacement::Grounded};
	std::uint32_t veterancyLevel{};
};
} // namespace generalszh::crates

export namespace ecs
{
template<> struct ComponentTraits<generalszh::crates::CrateState>
{
	static constexpr std::string_view StableName = "games.generalszh.crates.state";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
} // namespace ecs
