module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.production.doors.components.legacy_door_state;
export import engine.ecs.core.component_registry;
export namespace generalszh::production
{
struct DoorOpening
{
	std::uint64_t tick{0};
};

struct DoorWaiting
{
	std::uint64_t tick{0};
};

struct DoorClosing
{
	std::uint64_t tick{0};
};

struct DoorHold
{
	bool value{false};
};

}

export namespace ecs
{
template<>
struct ComponentTraits<generalszh::production::DoorOpening>
{
	static constexpr std::string_view StableName = "games.generalszh.production.door.opening";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<>
struct ComponentTraits<generalszh::production::DoorWaiting>
{
	static constexpr std::string_view StableName = "games.generalszh.production.door.waiting";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<>
struct ComponentTraits<generalszh::production::DoorClosing>
{
	static constexpr std::string_view StableName = "games.generalszh.production.door.closing";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<>
struct ComponentTraits<generalszh::production::DoorHold>
{
	static constexpr std::string_view StableName = "games.generalszh.production.door.hold";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
