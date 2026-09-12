module;
#include "power_store.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include "games/generalszh/simulation/execution/gameplay_workers.h"
#include "games/generalszh/simulation/lifecycle/world_binding.h"
#include <algorithm>
#include <cassert>
#include <exception>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>

export module games.generalszh.adapters.legacy.power.power_store;
import engine.ecs.core.world;
import engine.gameplay.rts.power.ledger.power_ledger;
import games.generalszh.gameplay.power.suppression.power_suppression;
import games.generalszh.simulation.reset.persistent_world_reset;
import games.generalszh.simulation.power.power_simulation;

extern "C++"
{
namespace generalszh::legacy
{
using engine::gameplay::rts::power::PowerLedger;
using power::PowerSuppression;
using power::PowerRecoveryOwner;
using power::PowerRecovery;

struct PowerStore::Impl
{
	static constexpr auto None = (std::numeric_limits<std::size_t>::max)();
	struct Slot { ecs::Entity entity{}; std::size_t next{None}; bool active{false}; std::uint64_t revision{}; };
	GameplayState &state;
	std::vector<Slot> slots;
	std::size_t free{None}, count{0};
	bool failed{false}, prepared{false};
	struct ResetValue { PowerTotals totals; PowerSuppression suppression; PowerRecoveryOwner owner; };
	std::vector<ResetValue> resetValues;
	std::vector<ecs::Entity> resetEntities;
	std::unique_ptr<GameplayWorkers> ownedWorkers;
	GameplayWorkers *workers;
	std::uint32_t ticksPerSecond;
	std::optional<GameplayWorldBinding> binding;
	std::unique_ptr<PowerSimulation> simulation;
	using Query = ecs::Query<ecs::Read<PowerLedger>, ecs::Read<PowerSuppression>, ecs::Read<PowerRecoveryOwner>, ecs::Read<PowerRecovery>>;
	std::unique_ptr<Query> query;
	struct Effect { ecs::Entity entity; std::size_t slot; std::uint64_t revision; std::int32_t player; };
	std::vector<Effect> effects;
	std::size_t cursor{};
	bool awaiting{false};
	explicit Impl(GameplayState &injected, GameplayWorkers *shared, std::uint32_t rate) : state(injected),
		ownedWorkers(shared ? nullptr : std::make_unique<GameplayWorkers>(1)),
		workers(shared ? shared : ownedWorkers.get()), ticksPerSecond(rate) { Initialize(); }
	void Initialize()
	{
		binding.emplace(state);
		simulation = std::make_unique<PowerSimulation>(World(), *workers);
		simulation->Finalize(engine::time::FixedStep{ticksPerSecond});
		query = std::make_unique<Query>(World());
	}
	void Check() const
	{
		if (failed || prepared || World().IsScheduledExecutionActive())
			throw std::logic_error("Power storage unavailable during execution/reset or after failure");
	}
	void Revise(std::size_t index)
	{
		Check();
		if (slots[index].revision == UINT64_MAX) { failed = true; throw std::overflow_error("Power binding revision exhausted"); }
		++slots[index].revision;
	}
	ecs::World &World() const noexcept { return state.World(); }
	template<class T = PowerLedger> T &Require(std::size_t index) const
	{
		Check();
		if (index >= slots.size() || !slots[index].active) throw std::logic_error("Invalid power binding");
		auto *ledger = World().Get<T>(slots[index].entity);
		if (!ledger) throw std::logic_error("Missing bound power entity/component");
		return *ledger;
	}
	std::span<const ecs::Entity> Capture()
	{
		Check();
		if (awaiting) throw std::logic_error("Finish power recovery before reset");
		resetValues.resize(slots.size());
		resetEntities.clear();
		resetEntities.reserve(count);
		for (std::size_t index = 0; index < slots.size(); ++index)
			if (slots[index].active)
			{
				const auto &ledger = Require(index);
				resetValues[index] = {{ledger.production, ledger.consumption}, Require<PowerSuppression>(index), Require<PowerRecoveryOwner>(index)};
				resetEntities.push_back(slots[index].entity);
			}
		prepared = true;
		return resetEntities;
	}
	void Release()
	{
		if (!prepared || failed) throw std::logic_error("Power reset was not prepared");
		query.reset(); simulation.reset(); binding.reset();
		for (auto &slot : slots)
			if (slot.active)
			{
				if (!World().Destroy(slot.entity)) throw std::logic_error("Missing power entity during reset release");
				slot.entity = {};
			}
	}
	void Restore()
	{
		if (!prepared || failed) throw std::logic_error("Power reset was not prepared");
		for (std::size_t index = 0; index < slots.size(); ++index)
			if (slots[index].active)
			{
				auto &slot = slots[index];
				slot.entity = World().Create<PowerLedger, PowerSuppression, PowerRecoveryOwner, PowerRecovery>();
				*World().Get<PowerLedger>(slot.entity) = {resetValues[index].totals.production, resetValues[index].totals.consumption};
				*World().Get<PowerSuppression>(slot.entity) = resetValues[index].suppression;
				*World().Get<PowerRecoveryOwner>(slot.entity) = resetValues[index].owner;
			}
		Initialize();
	}
	void Discard() noexcept { resetValues.clear(); resetEntities.clear(); prepared = false; }
};

PowerStore::PowerStore(GameplayState &state) : m_impl(new Impl(state, nullptr, 30)) {}
PowerStore::PowerStore(GameplayState &state, GameplayWorkers &workers, std::uint32_t rate) : m_impl(new Impl(state, &workers, rate)) {}
PowerStore::~PowerStore() noexcept
{
	if (Count() || m_impl->prepared) std::terminate();
	delete m_impl;
}
std::size_t PowerStore::Count() const noexcept { return m_impl->count; }
PersistentResetParticipant PowerStore::ResetParticipant()
{
	return {"games.generalszh.power", &m_impl->state, m_impl,
		+[](void *context) { return static_cast<Impl *>(context)->Capture(); },
		+[](void *context) { static_cast<Impl *>(context)->Release(); },
		+[](void *context) { static_cast<Impl *>(context)->Restore(); },
		+[](void *context) noexcept { static_cast<Impl *>(context)->Discard(); },
		+[](void *context) noexcept { static_cast<Impl *>(context)->failed = true; }, 1};
}

void PowerStore::Fail() noexcept { m_impl->failed = true; }
void PowerStore::ExecuteRecovery(std::uint64_t tick)
{
	auto &impl = *m_impl;
	impl.Check();
	if (impl.awaiting) throw std::logic_error("Power recovery effects are still pending");
	try
	{
		// Validate the complete production schema/ownership before consuming deadlines.
		std::size_t matched = 0;
		impl.query->ForEachChunk([&](auto chunk) {
			const auto owners = chunk.template Get<PowerRecoveryOwner>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				const auto slot = owners[row].slot;
				if (slot >= impl.slots.size() || !impl.slots[slot].active || impl.slots[slot].entity != chunk.Entities()[row])
					throw std::logic_error("Power recovery ownership mismatch");
				++matched;
			}
		});
		if (matched != impl.count) throw std::logic_error("Power recovery source is missing query components");
		impl.simulation->Execute({tick, engine::time::FixedStep{impl.ticksPerSecond}});
		impl.effects.clear(); impl.cursor = 0;
		impl.query->ForEachChunk([&](auto chunk) {
			const auto owners = chunk.template Get<PowerRecoveryOwner>();
			const auto recoveries = chunk.template Get<PowerRecovery>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
				if (recoveries[row].recovered && owners[row].player >= 0)
					impl.effects.push_back({chunk.Entities()[row], owners[row].slot, impl.slots[owners[row].slot].revision, owners[row].player});
		});
		std::sort(impl.effects.begin(), impl.effects.end(), [](const auto &a, const auto &b) {
			return std::tie(a.player, a.entity.index, a.entity.generation) < std::tie(b.player, b.entity.index, b.entity.generation);
		});
		impl.awaiting = true;
	}
	catch (...) { Fail(); impl.effects.clear(); throw; }
}
bool PowerStore::NextRecovery(std::int32_t &player)
{
	auto &impl = *m_impl; impl.Check();
	if (!impl.awaiting) throw std::logic_error("No power recovery execution to consume");
	while (impl.cursor != impl.effects.size())
	{
		const auto &effect = impl.effects[impl.cursor++];
		const auto &slot = impl.slots[effect.slot];
		if (!slot.active || slot.entity != effect.entity || slot.revision != effect.revision) continue;
		player = effect.player; return true;
	}
	return false;
}
void PowerStore::FinishRecovery()
{
	auto &impl = *m_impl; impl.Check();
	if (!impl.awaiting || impl.cursor != impl.effects.size())
	{ Fail(); throw std::logic_error("Power recovery effects must be consumed exactly once"); }
	impl.effects.clear(); impl.awaiting = false;
}

PowerLease::PowerLease(PowerStore &store, PowerTotals initial) : m_store(store)
{
	auto &impl = *store.m_impl;
	impl.Check();
	const bool append = impl.free == PowerStore::Impl::None;
	if (append)
	{
		// Grow reusable output capacity only during source construction, never per tick.
		if (impl.effects.capacity() < impl.slots.size() + 1)
			impl.effects.reserve((std::max)(impl.slots.size() + 1, impl.effects.capacity() * 2));
		impl.slots.emplace_back();
	}
	m_slot = append ? impl.slots.size() - 1 : impl.free;
	ecs::Entity entity;
	try { entity = impl.World().Create<PowerLedger, PowerSuppression, PowerRecoveryOwner, PowerRecovery>(); }
	catch (...) { if (append) impl.slots.pop_back(); throw; }
	if (!append) impl.free = impl.slots[m_slot].next;
	impl.slots[m_slot] = {entity, PowerStore::Impl::None, true};
	*impl.World().Get<PowerLedger>(entity) = {initial.production, initial.consumption};
	*impl.World().Get<PowerRecoveryOwner>(entity) = {m_slot, -1};
	++impl.count;
}
PowerLease::~PowerLease() noexcept
{
	auto &impl = *m_store.m_impl;
	if (impl.prepared) std::terminate();
	auto &slot = impl.slots[m_slot];
	if (impl.World().IsAlive(slot.entity))
	{
		if (!impl.World().Destroy(slot.entity)) std::terminate();
	}
	else if (!impl.failed) std::terminate();
	slot = {{}, impl.free, false};
	impl.free = m_slot;
	--impl.count;
}
PowerTotals PowerLease::State() const
{
	const auto &ledger = m_store.m_impl->Require(m_slot);
	return {ledger.production, ledger.consumption};
}
void PowerLease::Restore(PowerTotals value) { m_store.m_impl->Require(m_slot) = {value.production, value.consumption}; }
void PowerLease::Reset()
{
	// Resolve both columns before mutation. Restoring totals alone intentionally
	// leaves suppression unchanged; an explicit reset clears both owned states.
	auto &impl = *m_store.m_impl;
	auto &ledger = impl.Require(m_slot);
	auto &suppression = impl.Require<PowerSuppression>(m_slot);
	impl.Revise(m_slot);
	ledger = {};
	suppression = {};
}
void PowerLease::SetPlayer(std::int32_t player)
{
	auto &impl = *m_store.m_impl;
	auto &owner = impl.Require<PowerRecoveryOwner>(m_slot);
	impl.Revise(m_slot); owner.player = player;
}
void PowerLease::AddProduction(std::int32_t delta)
{
	engine::gameplay::rts::power::ApplyProduction(m_store.m_impl->Require(m_slot), delta);
}
void PowerLease::AddConsumption(std::int32_t delta)
{
	engine::gameplay::rts::power::ApplyConsumption(m_store.m_impl->Require(m_slot), delta);
}
std::uint32_t PowerLease::SabotagedUntilFrame() const
{
	const auto deadline = m_store.m_impl->Require<PowerSuppression>(m_slot).untilTick;
	assert(deadline <= (std::numeric_limits<std::uint32_t>::max)());
	return static_cast<std::uint32_t>(deadline);
}
void PowerLease::SetSabotagedUntilFrame(std::uint32_t deadline)
{
	auto &impl = *m_store.m_impl;
	auto &suppression = impl.Require<PowerSuppression>(m_slot);
	impl.Revise(m_slot); power::SetSuppressionUntil(suppression, deadline);
}
bool PowerLease::IsSuppressedAt(std::uint32_t frame) const
{
	return power::IsSuppressed(m_store.m_impl->Require<PowerSuppression>(m_slot), frame);
}
bool PowerLease::RecoverSuppressionAt(std::uint32_t)
{
	throw std::logic_error("Power recovery belongs to the explicit ECS execution boundary");
}
}
}
