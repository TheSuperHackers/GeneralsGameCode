module;

#include "upgrade_execution_store.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include <exception>
#include <stdexcept>

export module games.generalszh.adapters.legacy.upgrades.execution.upgrade_execution_store;

import engine.ecs.core.world;
import engine.gameplay.rts.upgrades.state.upgrade_execution;
import games.generalszh.simulation.gameplay_state;

extern "C++"
{
namespace generalszh::legacy
{
using engine::gameplay::rts::upgrades::UpgradeExecutionState;

struct UpgradeExecutionStore::Impl
{
	GameplayState &state;
	std::size_t leases{};
	explicit Impl(GameplayState &injected) : state(injected) {}
	ecs::World &World() const noexcept { return state.World(); }
	void EnsureNotScheduled() const
	{
		if (World().IsScheduledExecutionActive())
			throw std::logic_error("Legacy upgrade execution access cannot bypass scheduled component access");
	}
	UpgradeExecutionState &Require(ecs::Entity entity) const
	{
		EnsureNotScheduled();
		auto *value = World().Get<UpgradeExecutionState>(entity);
		if (!value) throw std::logic_error("Stale or missing upgrade execution binding");
		return *value;
	}
};

UpgradeExecutionStore::UpgradeExecutionStore(GameplayState &state) : m_impl(nullptr)
{
	if (state.World().IsScheduledExecutionActive())
		throw std::logic_error("Cannot construct legacy upgrade execution storage during scheduled execution");
	m_impl = new Impl(state);
}
UpgradeExecutionStore::~UpgradeExecutionStore() noexcept
{
	if (m_impl->leases != 0 || m_impl->World().IsScheduledExecutionActive()) std::terminate();
	delete m_impl;
}
std::size_t UpgradeExecutionStore::Count() const noexcept { return m_impl->leases; }

UpgradeExecutionLease::UpgradeExecutionLease(UpgradeExecutionStore &store) : m_store(store)
{
	auto &impl = *m_store.m_impl;
	impl.EnsureNotScheduled();
	const auto entity = impl.World().Create<UpgradeExecutionState>();
	m_index = entity.index;
	m_generation = entity.generation;
	++impl.leases;
}
UpgradeExecutionLease::~UpgradeExecutionLease() noexcept
{
	auto &impl = *m_store.m_impl;
	if (impl.World().IsScheduledExecutionActive() || impl.leases == 0) std::terminate();
	if (!impl.World().Destroy(ecs::Entity{m_index, m_generation})) std::terminate();
	--impl.leases;
}
bool UpgradeExecutionLease::Get() const
{
	return m_store.m_impl->Require(ecs::Entity{m_index, m_generation}).executed;
}
void UpgradeExecutionLease::Set(const bool executed)
{
	m_store.m_impl->Require(ecs::Entity{m_index, m_generation}).executed = executed;
}
}
}
