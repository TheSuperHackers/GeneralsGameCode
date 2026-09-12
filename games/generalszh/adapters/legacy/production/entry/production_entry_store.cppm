module;
#include "production_entry_store.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include <exception>
#include <cassert>
#include <memory>
#include <limits>
#include <stdexcept>
#include <vector>

export module games.generalszh.adapters.legacy.production.entry.production_entry_store;
import engine.ecs.core.world;
import engine.gameplay.rts.production.quantity.production_quantity;
import engine.gameplay.rts.production.queue.production_queue;
import games.generalszh.gameplay.production.progress.production_progress;
import games.generalszh.gameplay.production.entry.production_entry_metadata;
import games.generalszh.gameplay.production.entry.exit_reservation;
import games.generalszh.simulation.gameplay_state;

extern "C++"
{
namespace generalszh::legacy
{
using engine::gameplay::rts::production::ProductionQuantity;
using engine::gameplay::rts::production::ProductionQueueMember;
using production::ProductionElapsed;
using production::ProductionProgress;
using production::ProductionEntryKind;
using production::ProductionCorrelation;
using production::ProductionExitReservation;
struct ProductionEntryStore::Impl
{
	std::unique_ptr<GameplayState> ownedState;
	GameplayState *state;
	std::size_t leases{0};
	struct Binding { ProductionEntry *entry{nullptr}; std::uint32_t generation{0}; };
	// Indexed lookup only: pointer values never participate in queue ordering.
	std::vector<Binding> bindings;
	Impl() : ownedState(std::make_unique<GameplayState>()), state(ownedState.get()) {}
	explicit Impl(GameplayState &injected) : state(&injected) {}
	ecs::World &World() const noexcept { return state->World(); }
	template<class T> T &Require(ecs::Entity entity) const
	{
		if (World().IsScheduledExecutionActive())
			throw std::logic_error("Legacy production leases cannot bypass scheduled component access");
		auto *component = World().Get<T>(entity);
		if (!component) throw std::logic_error("Stale production entry lease or missing component");
		return *component;
	}
};
ProductionEntryStore::ProductionEntryStore() : m_impl(new Impl) {}
ProductionEntryStore::ProductionEntryStore(GameplayState &state) : m_impl(new Impl(state)) {}
ProductionEntryStore::~ProductionEntryStore() noexcept
{
	if (Count() != 0) std::terminate();
	delete m_impl;
}
std::size_t ProductionEntryStore::Count() const noexcept { return m_impl->leases; }
GameplayState &ProductionEntryStore::Gameplay() const noexcept { return *m_impl->state; }
ProductionEntry *ProductionEntryStore::Resolve(std::uint32_t index, std::uint32_t generation) const
{
	if (m_impl->World().IsScheduledExecutionActive())
		throw std::logic_error("Legacy production mapping cannot bypass scheduled access");
	if (!m_impl->World().IsAlive({index, generation}) || index >= m_impl->bindings.size())
		return nullptr;
	const auto &binding = m_impl->bindings[index];
	return binding.generation == generation ? binding.entry : nullptr;
}
void ProductionEntryStore::Reset()
{
	if (Count() != 0) throw std::logic_error("Cannot reset production entry storage with live entries");
	if (m_impl->ownedState) m_impl->state->Reset();
}
ProductionEntryLease::ProductionEntryLease(ProductionEntryStore &store) : m_store(store)
{
	const auto entity = store.m_impl->World().Create<ProductionQuantity, ProductionElapsed, ProductionProgress, ProductionQueueMember, ProductionEntryKind, ProductionCorrelation, ProductionExitReservation>();
	m_index = entity.index;
	m_generation = entity.generation;
	++store.m_impl->leases;
}
ProductionEntryLease::~ProductionEntryLease() noexcept
{
	const auto &membership = m_store.m_impl->Require<ProductionQueueMember>({m_index, m_generation});
	if (membership.queue.IsValid()) std::terminate(); // Detach before deleting an entry.
	if (!m_store.m_impl->World().Destroy({m_index, m_generation})) std::terminate();
	if (m_index < m_store.m_impl->bindings.size()) m_store.m_impl->bindings[m_index] = {};
	--m_store.m_impl->leases;
}
void ProductionEntryLease::Bind(ProductionEntry *entry)
{
	(void)m_store.m_impl->Require<ProductionQueueMember>({m_index, m_generation});
	if (!entry) throw std::invalid_argument("Cannot bind a null production entry");
	auto &bindings = m_store.m_impl->bindings;
	if (m_index >= bindings.size()) bindings.resize(static_cast<std::size_t>(m_index) + 1);
	auto &binding = bindings[m_index];
	if (binding.entry && (binding.entry != entry || binding.generation != m_generation))
		throw std::logic_error("Production entry identity is already bound");
	binding = {entry, m_generation};
}
std::int32_t ProductionEntryLease::Type() const
{
	return static_cast<std::int32_t>(m_store.m_impl->Require<ProductionEntryKind>({m_index, m_generation}).value);
}
void ProductionEntryLease::SetType(std::int32_t type)
{
	assert(type >= static_cast<std::int32_t>(production::EntryKind::Invalid) &&
		type <= static_cast<std::int32_t>(production::EntryKind::Upgrade));
	m_store.m_impl->Require<ProductionEntryKind>({m_index, m_generation}).value =
		static_cast<production::EntryKind>(type);
}
std::int32_t ProductionEntryLease::Correlation() const
{
	return m_store.m_impl->Require<ProductionCorrelation>({m_index, m_generation}).value;
}
void ProductionEntryLease::SetCorrelation(std::int32_t correlation)
{
	m_store.m_impl->Require<ProductionCorrelation>({m_index, m_generation}).value = correlation;
}
std::int32_t ProductionEntryLease::Total() const
{
	return m_store.m_impl->Require<ProductionQuantity>({m_index, m_generation}).total;
}
std::int32_t ProductionEntryLease::Completed() const
{
	return m_store.m_impl->Require<ProductionQuantity>({m_index, m_generation}).completed;
}
std::int32_t ProductionEntryLease::Remaining() const
{
	return engine::gameplay::rts::production::Remaining(m_store.m_impl->Require<ProductionQuantity>({m_index, m_generation}));
}
void ProductionEntryLease::Set(std::int32_t total, std::int32_t completed)
{
	engine::gameplay::rts::production::SetQuantity(m_store.m_impl->Require<ProductionQuantity>({m_index, m_generation}), total, completed);
}
std::int32_t ProductionEntryLease::ExitDoor() const
{
	return static_cast<std::int32_t>(m_store.m_impl->Require<ProductionExitReservation>({m_index, m_generation}).value);
}
void ProductionEntryLease::SetExitDoor(std::int32_t exitDoor)
{
	assert(production::IsValidExitReservation(exitDoor));
	m_store.m_impl->Require<ProductionExitReservation>({m_index, m_generation}).value =
		static_cast<production::ExitReservation>(exitDoor);
}
void ProductionEntryLease::RestoreExitDoor(std::int32_t exitDoor)
{
	if (!production::IsValidExitReservation(exitDoor))
		throw std::invalid_argument("Invalid production exit reservation");
	m_store.m_impl->Require<ProductionExitReservation>({m_index, m_generation}).value =
		static_cast<production::ExitReservation>(exitDoor);
}
void ProductionEntryLease::CompleteOne()
{
	const ecs::Entity entity{m_index, m_generation};
	// Resolve both authoritative columns before either completion or reservation
	// state can be changed.
	auto &quantity = m_store.m_impl->Require<ProductionQuantity>(entity);
	auto &exitReservation = m_store.m_impl->Require<ProductionExitReservation>(entity);
	engine::gameplay::rts::production::CompleteOne(quantity);
	production::ClearExitReservation(exitReservation);
}
void ProductionEntryLease::AdvanceStep()
{
	production::AdvanceOneStep(m_store.m_impl->Require<ProductionElapsed>({m_index, m_generation}));
}
void ProductionEntryLease::RefreshProgress(std::int32_t requiredFrames)
{
	const auto &elapsed = m_store.m_impl->Require<ProductionElapsed>({m_index, m_generation});
	auto &progress = m_store.m_impl->Require<ProductionProgress>({m_index, m_generation});
	// Already truncated by the legacy template calculation. Do not round again.
	production::RefreshProgress(elapsed, progress, production::ProductionRequirement{requiredFrames});
}
float ProductionEntryLease::Percent() const
{
	return m_store.m_impl->Require<ProductionProgress>({m_index, m_generation}).percent;
}
std::int32_t ProductionEntryLease::LegacyElapsedFrames() const
{
	const auto ticks = m_store.m_impl->Require<ProductionElapsed>({m_index, m_generation}).ticks;
	// External format boundary, not a per-step numeric guard. Do not silently
	// truncate modern elapsed state into an unrepresentable legacy save field.
	if (ticks < (std::numeric_limits<std::int32_t>::min)() || ticks > (std::numeric_limits<std::int32_t>::max)())
		throw std::overflow_error("Production elapsed time cannot fit the legacy transfer field");
	return static_cast<std::int32_t>(ticks);
}
void ProductionEntryLease::RestoreProgress(std::int32_t elapsedFrames, float percent)
{
	auto &elapsed = m_store.m_impl->Require<ProductionElapsed>({m_index, m_generation});
	auto &progress = m_store.m_impl->Require<ProductionProgress>({m_index, m_generation});
	// The cached percentage is observable independently until the next update.
	elapsed = ProductionElapsed{elapsedFrames};
	progress = ProductionProgress{percent};
}
}
}
