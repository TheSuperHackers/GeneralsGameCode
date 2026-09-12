module;

#include "production_queue_store.h"
#include "games/generalszh/adapters/legacy/production/entry/production_entry_store.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include <exception>
#include <memory>
#include <stdexcept>

export module games.generalszh.adapters.legacy.production.queue.production_queue_store;
import engine.ecs.core.world;
import engine.gameplay.rts.production.queue.production_queue;
import games.generalszh.simulation.gameplay_state;

extern "C++"
{
namespace generalszh::legacy
{
using engine::gameplay::rts::production::ProductionQueueStorage;

struct ProductionQueueStore::Impl
{
	GameplayState *state;
	ProductionEntryStore &entries;
	ProductionQueueStorage storage;

	Impl(GameplayState &injectedState, ProductionEntryStore &injectedEntries) :
		state(&injectedState),
		entries(injectedEntries)
	{
	}

	ecs::World &World() const noexcept
	{
		return state->World();
	}

	void RequireReadAccess() const
	{
		if (World().IsScheduledExecutionActive())
			throw std::logic_error("Legacy production queue reads cannot bypass scheduled component access");
	}
};

ProductionQueueStore::ProductionQueueStore(ProductionEntryStore &entries) :
	m_impl(new Impl(entries.Gameplay(), entries))
{
}

ProductionQueueStore::ProductionQueueStore(GameplayState &state,
	ProductionEntryStore &entries) :
	m_impl(nullptr)
{
	if (&state != &entries.Gameplay())
		throw std::invalid_argument("Production queue state must match its entry store");
	m_impl = new Impl(state, entries);
}

ProductionQueueStore::~ProductionQueueStore() noexcept
{
	if (Count() != 0)
		std::terminate();
	delete m_impl;
}

std::size_t ProductionQueueStore::Count() const noexcept
{
	return m_impl->storage.LiveQueueCount();
}

ProductionQueueLease::ProductionQueueLease(ProductionQueueStore &store,
	const std::uint32_t initialCapacity) :
	m_store(store),
	m_index(ecs::Entity::InvalidIndex),
	m_generation(ecs::Entity::InvalidGeneration)
{
	const ecs::Entity queue = store.m_impl->storage.Create(store.m_impl->World(), initialCapacity);
	m_index = queue.index;
	m_generation = queue.generation;
}

ProductionQueueLease::~ProductionQueueLease() noexcept
{
	auto &impl = *m_store.m_impl;
	const ecs::Entity queue{m_index, m_generation};
	if (!impl.storage.Entries(impl.World(), queue).empty())
		std::terminate();
	if (!impl.storage.Destroy(impl.World(), queue))
		std::terminate();
}

std::uint32_t ProductionQueueLease::Count() const
{
	auto &impl = *m_store.m_impl;
	impl.RequireReadAccess();
	return static_cast<std::uint32_t>(
		impl.storage.Entries(impl.World(), ecs::Entity{m_index, m_generation}).size());
}

ProductionEntry *ProductionQueueLease::First() const
{
	auto &impl = *m_store.m_impl;
	impl.RequireReadAccess();
	const ecs::Entity entry = impl.storage.First(impl.World(), ecs::Entity{m_index, m_generation});
	if (!entry.IsValid())
		return nullptr;
	ProductionEntry *resolved = impl.entries.Resolve(entry.index, entry.generation);
	if (resolved == nullptr)
		throw std::logic_error("Queued production entry has no valid legacy binding");
	return resolved;
}

ProductionEntry *ProductionQueueLease::Next(const ProductionEntryLease &entry) const
{
	auto &impl = *m_store.m_impl;
	impl.RequireReadAccess();
	if (&entry.m_store != &impl.entries)
		throw std::invalid_argument("Production queue entry belongs to a different entry store");
	if (&entry.m_store.Gameplay() != impl.state)
		throw std::invalid_argument("Production queue entry belongs to a different gameplay state");
	const ecs::Entity queue{m_index, m_generation};
	const ecs::Entity current{entry.m_index, entry.m_generation};
	if (impl.entries.Resolve(entry.m_index, entry.m_generation) == nullptr)
	{
		if (impl.storage.Contains(impl.World(), queue, current))
			throw std::logic_error("Queued production entry has no valid legacy binding");
		return nullptr;
	}
	const ecs::Entity next = impl.storage.Next(impl.World(),
		queue, current);
	if (!next.IsValid())
		return nullptr;
	ProductionEntry *resolved = impl.entries.Resolve(next.index, next.generation);
	if (resolved == nullptr)
		throw std::logic_error("Queued production entry has no valid legacy binding");
	return resolved;
}

bool ProductionQueueLease::Contains(const ProductionEntryLease &entry) const
{
	auto &impl = *m_store.m_impl;
	impl.RequireReadAccess();
	if (&entry.m_store != &impl.entries)
		throw std::invalid_argument("Production queue entry belongs to a different entry store");
	if (&entry.m_store.Gameplay() != impl.state)
		throw std::invalid_argument("Production queue entry belongs to a different gameplay state");
	if (impl.entries.Resolve(entry.m_index, entry.m_generation) == nullptr)
		return false;
	return impl.storage.Contains(impl.World(),
		ecs::Entity{m_index, m_generation},
		ecs::Entity{entry.m_index, entry.m_generation});
}

void ProductionQueueLease::Append(ProductionEntryLease &entry)
{
	auto &impl = *m_store.m_impl;
	if (&entry.m_store != &impl.entries)
		throw std::invalid_argument("Production queue entry belongs to a different entry store");
	if (&entry.m_store.Gameplay() != impl.state)
		throw std::invalid_argument("Production queue entry belongs to a different gameplay state");
	if (impl.entries.Resolve(entry.m_index, entry.m_generation) == nullptr)
		throw std::invalid_argument("Cannot append an unbound or stale production entry");

	const ecs::Entity queue{m_index, m_generation};
	const ecs::Entity item{entry.m_index, entry.m_generation};
	if (impl.storage.Contains(impl.World(), queue, item))
		throw std::logic_error("Production entry is already a member of this queue");
	impl.storage.Append(impl.World(), queue, item);
}

void ProductionQueueLease::Remove(ProductionEntryLease &entry)
{
	auto &impl = *m_store.m_impl;
	if (&entry.m_store != &impl.entries)
		throw std::invalid_argument("Production queue entry belongs to a different entry store");
	if (&entry.m_store.Gameplay() != impl.state)
		throw std::invalid_argument("Production queue entry belongs to a different gameplay state");
	if (impl.entries.Resolve(entry.m_index, entry.m_generation) == nullptr)
		throw std::invalid_argument("Cannot remove an unbound or stale production entry");
	if (!impl.storage.Remove(impl.World(),
		ecs::Entity{m_index, m_generation},
		ecs::Entity{entry.m_index, entry.m_generation}))
		throw std::logic_error("Production entry is not a member of this queue");
}
}
}
