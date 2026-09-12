module;

#include "production_marker_store.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include <exception>
#include <limits>
#include <memory>
#include <stdexcept>

export module games.generalszh.adapters.legacy.production.presentation.production_marker_store;
import engine.ecs.core.world;
import games.generalszh.gameplay.production.presentation.construction_marker;
import games.generalszh.simulation.gameplay_state;

extern "C++"
{
namespace generalszh::legacy
{
using generalszh::production::ConstructionMarker;
using generalszh::production::ConstructionMarkerTiming;

struct ProductionMarkerStore::Impl
{
	std::unique_ptr<GameplayState> ownedState;
	GameplayState *state;
	std::size_t leases{0};

	Impl() : ownedState(std::make_unique<GameplayState>()), state(ownedState.get()) {}
	explicit Impl(GameplayState &injected) : state(&injected) {}

	ecs::World &World() const noexcept { return state->World(); }

	ConstructionMarker &Require(const ecs::Entity entity) const
	{
		if (World().IsScheduledExecutionActive())
			throw std::logic_error("Legacy production marker leases cannot bypass scheduled component access");
		auto *marker = World().Get<ConstructionMarker>(entity);
		if (!marker)
			throw std::logic_error("Stale production marker lease or missing construction marker component");
		return *marker;
	}
};

namespace
{
std::uint32_t NarrowLegacyTimestamp(const std::uint64_t tick)
{
	if (tick > (std::numeric_limits<std::uint32_t>::max)())
		throw std::overflow_error("Construction marker timestamp cannot fit the legacy frame field");
	return static_cast<std::uint32_t>(tick);
}
}

ProductionMarkerStore::ProductionMarkerStore() : m_impl(new Impl) {}
ProductionMarkerStore::ProductionMarkerStore(GameplayState &state) : m_impl(new Impl(state)) {}

ProductionMarkerStore::~ProductionMarkerStore() noexcept
{
	if (Count() != 0)
		std::terminate();
	delete m_impl;
}

std::size_t ProductionMarkerStore::Count() const noexcept
{
	return m_impl->leases;
}

void ProductionMarkerStore::Reset()
{
	if (Count() != 0)
		throw std::logic_error("Cannot reset production marker storage with live leases");
	if (m_impl->ownedState)
		m_impl->state->Reset();
}

ProductionMarkerLease::ProductionMarkerLease(ProductionMarkerStore &store) : m_store(store)
{
	const auto entity = store.m_impl->World().Create<ConstructionMarker>();
	m_index = entity.index;
	m_generation = entity.generation;
	++store.m_impl->leases;
}

ProductionMarkerLease::~ProductionMarkerLease() noexcept
{
	if (!m_store.m_impl->World().Destroy(ecs::Entity{m_index, m_generation}))
		std::terminate();
	--m_store.m_impl->leases;
}

bool ProductionMarkerLease::Advance(const std::uint32_t now, const std::uint32_t duration)
{
	auto &marker = m_store.m_impl->Require(ecs::Entity{m_index, m_generation});
	return generalszh::production::AdvanceConstructionMarker<std::uint32_t>(
		marker, now, ConstructionMarkerTiming{static_cast<std::uint64_t>(duration)});
}

bool ProductionMarkerLease::Begin(const std::uint32_t now)
{
	auto &marker = m_store.m_impl->Require(ecs::Entity{m_index, m_generation});
	return generalszh::production::BeginConstructionMarker(marker, static_cast<std::uint64_t>(now));
}

std::uint32_t ProductionMarkerLease::ReadLegacy() const
{
	const auto &marker = m_store.m_impl->Require(ecs::Entity{m_index, m_generation});
	return NarrowLegacyTimestamp(marker.tick);
}

void ProductionMarkerLease::RestoreLegacy(const std::uint32_t tick)
{
	auto &marker = m_store.m_impl->Require(ecs::Entity{m_index, m_generation});
	marker = {static_cast<std::uint64_t>(tick)};
}
}
}
