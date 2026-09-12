module;

#include "upgrade_status_store.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include <cassert>
#include <exception>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

export module games.generalszh.adapters.legacy.upgrades.state.upgrade_status_store;

import engine.ecs.core.world;
import games.generalszh.gameplay.upgrades.state.upgrade_status;
import games.generalszh.simulation.gameplay_state;
import games.generalszh.simulation.reset.persistent_world_reset;

extern "C++"
{
namespace generalszh::legacy
{
using generalszh::upgrades::IsValidStatus;
using generalszh::upgrades::Status;
using generalszh::upgrades::UpgradeInstanceStatus;

struct UpgradeStatusStore::Impl
{
	static void SetValue(UpgradeInstanceStatus &status, const std::int32_t value) noexcept
	{
		assert(IsValidStatus(value));
		status.value = static_cast<Status>(value);
	}
	static constexpr auto None = (std::numeric_limits<std::size_t>::max)();
	struct Slot
	{
		ecs::Entity entity{};
		std::size_t next{None};
		bool active{false};
	};

	GameplayState &state;
	std::vector<Slot> slots;
	std::size_t free{None};
	std::size_t count{0};
	bool failed{false};
	bool prepared{false};
	std::vector<std::int32_t> resetValues;
	std::vector<ecs::Entity> resetEntities;

	explicit Impl(GameplayState &injected) : state(injected) {}
	ecs::World &World() const noexcept { return state.World(); }

	void EnsureNotScheduled() const
	{
		if (World().IsScheduledExecutionActive())
			throw std::logic_error(
				"Legacy upgrade status access cannot bypass scheduled component access");
	}

	UpgradeInstanceStatus &Require(const std::size_t index) const
	{
		EnsureNotScheduled();
		if (failed || prepared)
			throw std::logic_error(
				"Upgrade status storage is unavailable during/after failed reset");
		if (index >= slots.size() || !slots[index].active)
			throw std::logic_error("Invalid upgrade status binding");
		auto *status = World().Get<UpgradeInstanceStatus>(slots[index].entity);
		if (!status)
			throw std::logic_error(
				"Missing bound upgrade status entity/component");
		return *status;
	}

	ecs::Entity Create()
	{
		EnsureNotScheduled();
		return World().Create<UpgradeInstanceStatus>();
	}

	std::span<const ecs::Entity> Capture()
	{
		EnsureNotScheduled();
		if (failed || prepared)
			throw std::logic_error(
				"Cannot capture unavailable upgrade status storage");
		resetValues.resize(slots.size());
		resetEntities.clear();
		resetEntities.reserve(count);
		for (std::size_t index = 0; index < slots.size(); ++index)
			if (slots[index].active)
			{
				const auto &status = Require(index);
				resetValues[index] = static_cast<std::int32_t>(status.value);
				resetEntities.push_back(slots[index].entity);
			}
		prepared = true;
		return resetEntities;
	}

	void Release()
	{
		if (!prepared || failed)
			throw std::logic_error("Upgrade status reset was not prepared");
		for (auto &slot : slots)
			if (slot.active)
			{
				if (!World().Destroy(slot.entity))
					throw std::logic_error(
						"Missing upgrade status entity during reset release");
				slot.entity = {};
			}
	}

	void Restore()
	{
		if (!prepared || failed)
			throw std::logic_error("Upgrade status reset was not prepared");
		for (std::size_t index = 0; index < slots.size(); ++index)
			if (slots[index].active)
			{
				auto &slot = slots[index];
				slot.entity = Create();
				SetValue(*World().Get<UpgradeInstanceStatus>(slot.entity),
					resetValues[index]);
			}
	}

	void Discard() noexcept
	{
		resetValues.clear();
		resetEntities.clear();
		prepared = false;
	}
};

UpgradeStatusStore::UpgradeStatusStore(GameplayState &state) : m_impl(new Impl(state)) {}

UpgradeStatusStore::~UpgradeStatusStore() noexcept
{
	if (Count() || m_impl->prepared)
		std::terminate();
	delete m_impl;
}

std::size_t UpgradeStatusStore::Count() const noexcept
{
	return m_impl->count;
}

PersistentResetParticipant UpgradeStatusStore::ResetParticipant()
{
	return {"games.generalszh.upgrades", &m_impl->state, m_impl,
		+[](void *context) { return static_cast<Impl *>(context)->Capture(); },
		+[](void *context) { static_cast<Impl *>(context)->Release(); },
		+[](void *context) { static_cast<Impl *>(context)->Restore(); },
		+[](void *context) noexcept { static_cast<Impl *>(context)->Discard(); },
		+[](void *context) noexcept { static_cast<Impl *>(context)->failed = true; }};
}

UpgradeStatusLease::UpgradeStatusLease(UpgradeStatusStore &store) : m_store(store)
{
	auto &impl = *store.m_impl;
	if (impl.failed || impl.prepared)
		throw std::logic_error("Cannot bind unavailable upgrade status storage");
	// Keep the scheduled-execution preflight ahead of all slot/free-list changes.
	impl.EnsureNotScheduled();
	const bool append = impl.free == UpgradeStatusStore::Impl::None;
	if (append)
		impl.slots.emplace_back();
	m_slot = append ? impl.slots.size() - 1 : impl.free;
	ecs::Entity entity;
	try
	{
		entity = impl.Create();
	}
	catch (...)
	{
		if (append)
			impl.slots.pop_back();
		throw;
	}
	if (!append)
		impl.free = impl.slots[m_slot].next;
	impl.slots[m_slot] = {entity, UpgradeStatusStore::Impl::None, true};
	++impl.count;
}

UpgradeStatusLease::~UpgradeStatusLease() noexcept
{
	auto &impl = *m_store.m_impl;
	if (impl.prepared)
		std::terminate();
	auto &slot = impl.slots[m_slot];
	if (impl.World().IsAlive(slot.entity))
	{
		if (!impl.World().Destroy(slot.entity))
			std::terminate();
	}
	else if (!impl.failed)
		std::terminate();
	slot = {{}, impl.free, false};
	impl.free = m_slot;
	--impl.count;
}

std::int32_t UpgradeStatusLease::Get() const
{
	return static_cast<std::int32_t>(m_store.m_impl->Require(m_slot).value);
}

void UpgradeStatusLease::Set(const std::int32_t value)
{
	UpgradeStatusStore::Impl::SetValue(m_store.m_impl->Require(m_slot), value);
}

void UpgradeStatusLease::Restore(const std::int32_t value)
{
	if (!IsValidStatus(value))
		throw std::invalid_argument("Invalid upgrade status value");
	UpgradeStatusStore::Impl::SetValue(m_store.m_impl->Require(m_slot), value);
}
}
}
