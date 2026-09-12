module;
#include <cstdint>
#include <type_traits>
export module games.generalszh.gameplay.production.doors.algorithms.legacy_door_transitions;
export import games.generalszh.gameplay.production.doors.components.legacy_door_state;
export import games.generalszh.gameplay.production.doors.definitions.door_timing;
export namespace generalszh::production
{
struct DoorView
{
	DoorOpening &opening;
	DoorWaiting &waiting;
	DoorClosing &closing;
	DoorHold &hold;
};

inline constexpr std::uint8_t DoorOpeningFlag = 1;
inline constexpr std::uint8_t DoorWaitingFlag = 2;
inline constexpr std::uint8_t DoorClosingFlag = 4;

struct DoorChanges
{
	std::uint8_t clear{0};
	std::uint8_t set{0};
};

template<typename Clock = std::uint64_t>
	requires (std::is_same_v<Clock, std::uint32_t> || std::is_same_v<Clock, std::uint64_t>)
inline DoorChanges AdvanceDoor(DoorView view,
	const std::type_identity_t<Clock> now,
	const DoorTiming &timing) noexcept
{
	if (view.opening.tick != 0)
	{
		const Clock age = static_cast<Clock>(now - static_cast<Clock>(view.opening.tick));
		if (age > timing.opening)
		{
			view.opening.tick = 0;
			view.waiting.tick = static_cast<std::uint64_t>(now);
			return DoorChanges{DoorOpeningFlag, DoorWaitingFlag};
		}
	}
	else if (view.waiting.tick != 0)
	{
		const Clock age = static_cast<Clock>(now - static_cast<Clock>(view.waiting.tick));
		if (age > timing.waiting && !view.hold.value)
		{
			view.waiting.tick = 0;
			view.closing.tick = static_cast<std::uint64_t>(now);
			return DoorChanges{DoorWaitingFlag, DoorClosingFlag};
		}
	}
	else if (view.closing.tick != 0 && !view.hold.value)
	{
		const Clock age = static_cast<Clock>(now - static_cast<Clock>(view.closing.tick));
		if (age > timing.closing)
		{
			view.closing.tick = 0;
			return DoorChanges{DoorClosingFlag, 0};
		}
	}

	return {};
}

inline DoorChanges RequestDoorForExit(DoorView view, const std::uint64_t now) noexcept
{
	if (view.opening.tick == 0 && view.waiting.tick == 0 && view.closing.tick == 0)
	{
		view.opening.tick = now;
		return DoorChanges{0, DoorOpeningFlag};
	}

	if (view.waiting.tick != 0)
	{
		view.waiting.tick = now;
		return {};
	}

	if (view.closing.tick != 0)
	{
		view.waiting.tick = now;
		return DoorChanges{
			static_cast<std::uint8_t>(DoorOpeningFlag | DoorClosingFlag),
			DoorWaitingFlag};
	}

	return {};
}

inline DoorChanges HoldDoor(DoorView view, const bool hold, const std::uint64_t now) noexcept
{
	view.hold.value = hold;
	if (hold && view.opening.tick == 0 && view.waiting.tick == 0 && view.closing.tick == 0)
	{
		view.opening.tick = now;
		return DoorChanges{0, DoorOpeningFlag};
	}

	return {};
}

}
