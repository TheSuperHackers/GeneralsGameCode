module;

#include "production_door_store.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include <exception>
#include <limits>
#include <memory>
#include <stdexcept>

export module games.generalszh.adapters.legacy.production.doors.production_door_store;
import engine.ecs.core.world;
import games.generalszh.gameplay.production.doors.production_door;
import games.generalszh.simulation.gameplay_state;

extern "C++"
{
namespace generalszh::legacy
{
using generalszh::production::DoorClosing;
using generalszh::production::DoorHold;
using generalszh::production::DoorOpening;
using generalszh::production::DoorTiming;
using generalszh::production::DoorView;
using generalszh::production::DoorWaiting;

struct ProductionDoorStore::Impl
{
	std::unique_ptr<GameplayState> ownedState;
	GameplayState *state;
	std::size_t leases{0};

	Impl() : ownedState(std::make_unique<GameplayState>()), state(ownedState.get()) {}
	explicit Impl(GameplayState &injected) : state(&injected) {}

	ecs::World &World() const noexcept { return state->World(); }

	template<class T> T &Require(const ecs::Entity entity) const
	{
		if (World().IsScheduledExecutionActive())
			throw std::logic_error("Legacy production door leases cannot bypass scheduled component access");
		auto *component = World().Get<T>(entity);
		if (!component)
			throw std::logic_error("Stale production door lease or missing door component");
		return *component;
	}
};

namespace
{
ProductionDoorChanges ToLegacyChanges(const generalszh::production::DoorChanges changes) noexcept
{
	return {changes.clear, changes.set};
}

std::uint32_t NarrowLegacyTimestamp(const std::uint64_t tick)
{
	if (tick > (std::numeric_limits<std::uint32_t>::max)())
		throw std::overflow_error("Production door timestamp cannot fit the legacy frame field");
	return static_cast<std::uint32_t>(tick);
}
}

ProductionDoorStore::ProductionDoorStore() : m_impl(new Impl) {}
ProductionDoorStore::ProductionDoorStore(GameplayState &state) : m_impl(new Impl(state)) {}

ProductionDoorStore::~ProductionDoorStore() noexcept
{
	if (Count() != 0)
		std::terminate();
	delete m_impl;
}

std::size_t ProductionDoorStore::Count() const noexcept
{
	return m_impl->leases;
}

void ProductionDoorStore::Reset()
{
	if (Count() != 0)
		throw std::logic_error("Cannot reset production door storage with live leases");
	if (m_impl->ownedState)
		m_impl->state->Reset();
}

ProductionDoorLease::ProductionDoorLease(ProductionDoorStore &store) : m_store(store)
{
	const auto entity = store.m_impl->World().Create<DoorOpening, DoorWaiting, DoorClosing, DoorHold>();
	m_index = entity.index;
	m_generation = entity.generation;
	++store.m_impl->leases;
}

ProductionDoorLease::~ProductionDoorLease() noexcept
{
	if (!m_store.m_impl->World().Destroy(ecs::Entity{m_index, m_generation}))
		std::terminate();
	--m_store.m_impl->leases;
}

ProductionDoorChanges ProductionDoorLease::Advance(const std::uint32_t now,
	const std::uint32_t openingTime,
	const std::uint32_t waitingTime,
	const std::uint32_t closingTime)
{
	auto &impl = *m_store.m_impl;
	const ecs::Entity entity{m_index, m_generation};
	// Resolve every column before the modern operation can mutate any state.
	auto &opening = impl.Require<DoorOpening>(entity);
	auto &waiting = impl.Require<DoorWaiting>(entity);
	auto &closing = impl.Require<DoorClosing>(entity);
	auto &hold = impl.Require<DoorHold>(entity);
	DoorView view{opening, waiting, closing, hold};
	return ToLegacyChanges(generalszh::production::AdvanceDoor<std::uint32_t>(
		view, now, DoorTiming{openingTime, waitingTime, closingTime}));
}

ProductionDoorChanges ProductionDoorLease::RequestExit(const std::uint32_t now)
{
	auto &impl = *m_store.m_impl;
	const ecs::Entity entity{m_index, m_generation};
	auto &opening = impl.Require<DoorOpening>(entity);
	auto &waiting = impl.Require<DoorWaiting>(entity);
	auto &closing = impl.Require<DoorClosing>(entity);
	auto &hold = impl.Require<DoorHold>(entity);
	DoorView view{opening, waiting, closing, hold};
	return ToLegacyChanges(generalszh::production::RequestDoorForExit(
		view, static_cast<std::uint64_t>(now)));
}

ProductionDoorChanges ProductionDoorLease::SetHeld(const bool held, const std::uint32_t now)
{
	auto &impl = *m_store.m_impl;
	const ecs::Entity entity{m_index, m_generation};
	auto &opening = impl.Require<DoorOpening>(entity);
	auto &waiting = impl.Require<DoorWaiting>(entity);
	auto &closing = impl.Require<DoorClosing>(entity);
	auto &hold = impl.Require<DoorHold>(entity);
	DoorView view{opening, waiting, closing, hold};
	return ToLegacyChanges(generalszh::production::HoldDoor(
		view, held, static_cast<std::uint64_t>(now)));
}

bool ProductionDoorLease::Waiting() const
{
	const ecs::Entity entity{m_index, m_generation};
	return m_store.m_impl->Require<DoorWaiting>(entity).tick != 0;
}

ProductionDoorState ProductionDoorLease::ReadLegacy() const
{
	const ecs::Entity entity{m_index, m_generation};
	// Resolve every column before applying the cold legacy-width checks.
	const auto &opening = m_store.m_impl->Require<DoorOpening>(entity);
	const auto &waiting = m_store.m_impl->Require<DoorWaiting>(entity);
	const auto &closing = m_store.m_impl->Require<DoorClosing>(entity);
	const auto &hold = m_store.m_impl->Require<DoorHold>(entity);
	return {
		NarrowLegacyTimestamp(opening.tick),
		NarrowLegacyTimestamp(waiting.tick),
		NarrowLegacyTimestamp(closing.tick),
		hold.value};
}

void ProductionDoorLease::RestoreLegacy(const ProductionDoorState &state)
{
	auto &impl = *m_store.m_impl;
	const ecs::Entity entity{m_index, m_generation};
	// Resolve all columns before mutating any of them so a malformed binding is
	// nondestructive to the still-live components.
	auto &opening = impl.Require<DoorOpening>(entity);
	auto &waiting = impl.Require<DoorWaiting>(entity);
	auto &closing = impl.Require<DoorClosing>(entity);
	auto &hold = impl.Require<DoorHold>(entity);
	opening = {static_cast<std::uint64_t>(state.opening)};
	waiting = {static_cast<std::uint64_t>(state.waiting)};
	closing = {static_cast<std::uint64_t>(state.closing)};
	hold = {state.held};
}
}
}
