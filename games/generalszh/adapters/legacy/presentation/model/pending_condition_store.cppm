module;

#include "pending_condition_store.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include <cassert>
#include <exception>
#include <limits>
#include <memory>
#include <stdexcept>

export module games.generalszh.adapters.legacy.presentation.model.pending_condition_store;
import engine.ecs.core.world;
import games.generalszh.presentation.model.pending_conditions;
import games.generalszh.simulation.gameplay_state;

extern "C++"
{
namespace generalszh::legacy
{
using generalszh::presentation::ConditionCount;
using generalszh::presentation::ModelCondition;
using generalszh::presentation::PendingConditionClear;
using generalszh::presentation::PendingConditionDirty;
using generalszh::presentation::PendingConditionSet;
using generalszh::presentation::PendingConditionView;

static_assert(ConditionCount > 64u && ConditionCount <= 128u);
static_assert(generalszh::presentation::ConditionWordCount == 2u);

struct PendingConditionStore::Impl
{
	std::unique_ptr<GameplayState> ownedState;
	GameplayState *state;
	std::size_t leases{0};

	Impl() : ownedState(std::make_unique<GameplayState>()), state(ownedState.get()) {}
	explicit Impl(GameplayState &injected) : state(&injected) {}

	ecs::World &World() const noexcept { return state->World(); }

	PendingConditionView Require(const ecs::Entity entity) const
	{
		if (World().IsScheduledExecutionActive())
			throw std::logic_error("Legacy pending-condition leases cannot bypass scheduled component access");

		auto *clear = World().Get<PendingConditionClear>(entity);
		auto *set = World().Get<PendingConditionSet>(entity);
		auto *dirty = World().Get<PendingConditionDirty>(entity);
		if (clear == nullptr || set == nullptr || dirty == nullptr)
			throw std::logic_error("Stale pending-condition lease or missing pending-condition component");
		return {*clear, *set, *dirty};
	}
};

namespace
{
constexpr std::uint64_t LastConditionWordMask() noexcept
{
	constexpr std::size_t remainder = ConditionCount % 64u;
	return remainder == 0u
		? (std::numeric_limits<std::uint64_t>::max)()
		: (std::uint64_t{1} << remainder) - std::uint64_t{1};
}

void ValidateSnapshot(const PendingConditionSnapshot &snapshot)
{
	const bool invalidClearBits =
		(snapshot.clear.back() & ~LastConditionWordMask()) != std::uint64_t{0};
	const bool invalidSetBits =
		(snapshot.set.back() & ~LastConditionWordMask()) != std::uint64_t{0};
	if (invalidClearBits || invalidSetBits)
		throw std::invalid_argument("Pending-condition snapshot contains unused condition bits");
}
}

PendingConditionStore::PendingConditionStore() : m_impl(new Impl) {}

PendingConditionStore::PendingConditionStore(GameplayState &state) : m_impl(new Impl(state)) {}

PendingConditionStore::~PendingConditionStore() noexcept
{
	if (Count() != 0)
		std::terminate();
	delete m_impl;
}

std::size_t PendingConditionStore::Count() const noexcept
{
	return m_impl->leases;
}

void PendingConditionStore::Reset()
{
	if (Count() != 0)
		throw std::logic_error("Cannot reset pending-condition storage with live leases");
	if (m_impl->ownedState)
		m_impl->state->Reset();
}

PendingConditionLease::PendingConditionLease(PendingConditionStore &store) : m_store(store)
{
	const auto entity = store.m_impl->World().Create<
		PendingConditionClear, PendingConditionSet, PendingConditionDirty>();
	m_index = entity.index;
	m_generation = entity.generation;
	++store.m_impl->leases;
}

PendingConditionLease::~PendingConditionLease() noexcept
{
	if (!m_store.m_impl->World().Destroy(ecs::Entity{m_index, m_generation}))
		std::terminate();
	--m_store.m_impl->leases;
}

void PendingConditionLease::QueueSet(const std::uint16_t canonicalCondition)
{
	assert(canonicalCondition < ConditionCount);
	const auto view = m_store.m_impl->Require(ecs::Entity{m_index, m_generation});
	generalszh::presentation::QueueSet(view,
		static_cast<ModelCondition>(canonicalCondition));
}

void PendingConditionLease::QueueClear(const std::uint16_t canonicalCondition)
{
	assert(canonicalCondition < ConditionCount);
	const auto view = m_store.m_impl->Require(ecs::Entity{m_index, m_generation});
	generalszh::presentation::QueueClear(view,
		static_cast<ModelCondition>(canonicalCondition));
}

void PendingConditionLease::MarkDirty()
{
	const auto view = m_store.m_impl->Require(ecs::Entity{m_index, m_generation});
	generalszh::presentation::MarkDirty(view);
}

PendingConditionSnapshot PendingConditionLease::Read() const
{
	const auto view = m_store.m_impl->Require(ecs::Entity{m_index, m_generation});
	return {view.clear.mask.words, view.set.mask.words, view.dirty.value};
}

void PendingConditionLease::Restore(const PendingConditionSnapshot &snapshot)
{
	// Validate both cold transfer arrays before resolving or mutating live state.
	ValidateSnapshot(snapshot);
	const auto view = m_store.m_impl->Require(ecs::Entity{m_index, m_generation});
	view.clear.mask.words = snapshot.clear;
	view.set.mask.words = snapshot.set;
	view.dirty.value = snapshot.dirty;
}

void PendingConditionLease::Clear()
{
	const auto view = m_store.m_impl->Require(ecs::Entity{m_index, m_generation});
	generalszh::presentation::ResetPending(view);
}
}
}
