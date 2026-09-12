module;

#include "radar_store.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include "games/generalszh/simulation/lifecycle/world_binding.h"
#include "games/generalszh/simulation/execution/gameplay_workers.h"
#include <exception>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

export module games.generalszh.adapters.legacy.radar.radar_store;
import engine.ecs.core.world;
import engine.gameplay.rts.radar.availability.radar_availability;
import games.generalszh.simulation.radar.radar_simulation;
import games.generalszh.simulation.execution.gameplay_workers;
import games.generalszh.simulation.reset.persistent_world_reset;

extern "C++"
{
namespace generalszh::legacy
{
using engine::gameplay::rts::radar::RadarAvailability;
using engine::gameplay::rts::radar::RadarBatchRange;
using engine::gameplay::rts::radar::RadarAction;
using engine::gameplay::rts::radar::RadarTransition;

struct RadarStore::Impl
{
	static constexpr auto None = (std::numeric_limits<std::size_t>::max)();
	struct Slot { ecs::Entity entity{}; std::size_t next{None}; bool active{false}; };
	GameplayState &state;
	RadarExecutionConfig config;
	GameplayWorkers *workers;
	engine::gameplay::rts::radar::RadarBatch batch;
	// Reverse destruction order: cached execution dies before its World binding.
	std::optional<GameplayWorldBinding> worldBinding;
	std::unique_ptr<RadarSimulation> simulation;
	struct PresentationInput { bool eligible; std::int32_t player; };
	std::vector<PresentationInput> presentation;
	std::vector<RadarSound> sounds;
	std::vector<Slot> slots;
	std::size_t free{None}, count{0};
	bool failed{false}, prepared{false};
	std::vector<RadarValue> resetValues;
	std::vector<ecs::Entity> resetEntities;

	explicit Impl(GameplayState &injected, RadarExecutionConfig configuration, GameplayWorkers *shared = nullptr) :
		state(injected), config(configuration), workers(shared), batch(config.inputCapacity)
	{
		presentation.reserve(config.inputCapacity);
		sounds.reserve(config.inputCapacity);
		InitializeSimulation();
	}
	void InitializeSimulation()
	{
		worldBinding.emplace(state);
		if (workers)
			simulation = std::make_unique<RadarSimulation>(World(), batch, GameplayWorkerAccess::Jobs(*workers));
		else
			simulation = std::make_unique<RadarSimulation>(World(), batch, engine::jobs::JobSystemConfig{config.workers});
		simulation->Finalize(engine::time::FixedStep{config.ticksPerSecond});
	}
	ecs::World &World() const noexcept { return state.World(); }
	void Record(std::size_t index, RadarAction action, bool eligible, std::int32_t player)
	{
		Require(index);
		try
		{
			batch.Record(index, action);
			presentation.push_back({eligible, player}); // Reserved to the same hard limit.
		}
		catch (...) { failed = true; throw; }
	}

	RadarAvailability &Require(std::size_t index) const
	{
		if (failed || prepared || World().IsScheduledExecutionActive())
			throw std::logic_error("Radar storage is unavailable during/after failed reset");
		if (index >= slots.size() || !slots[index].active)
			throw std::logic_error("Invalid radar binding");
		auto *availability = World().Get<RadarAvailability>(slots[index].entity);
		if (!availability)
			throw std::logic_error("Missing bound radar entity/component");
		return *availability;
	}

	std::span<const ecs::Entity> Capture()
	{
		if (failed || prepared)
			throw std::logic_error("Cannot capture unavailable radar storage");
		resetValues.resize(slots.size());
		resetEntities.clear();
		resetEntities.reserve(count);
		for (std::size_t index = 0; index < slots.size(); ++index)
			if (slots[index].active)
			{
				const auto &availability = Require(index);
				resetValues[index] = {availability.producers, availability.resistantProducers, availability.suppressed};
				resetEntities.push_back(slots[index].entity);
			}
		prepared = true;
		return resetEntities;
	}

	void Release()
	{
		if (!prepared || failed)
			throw std::logic_error("Radar reset was not prepared");
		// No query may outlive the World it caches. Pending old-world input is
		// discarded only at this explicit reset/destruction boundary.
		simulation.reset();
		worldBinding.reset();
		batch.Clear();
		presentation.clear();
		sounds.clear();
		for (auto &slot : slots)
			if (slot.active)
			{
				if (!World().Destroy(slot.entity))
					throw std::logic_error("Missing radar entity during reset release");
				slot.entity = {};
			}
	}

	void Restore()
	{
		if (!prepared || failed)
			throw std::logic_error("Radar reset was not prepared");
		for (std::size_t index = 0; index < slots.size(); ++index)
			if (slots[index].active)
			{
				auto &slot = slots[index];
				// Publish the new binding as soon as creation succeeds. This keeps
				// teardown safe if a later reset participant fails.
				slot.entity = World().Create<RadarAvailability, RadarBatchRange>();
				const auto &value = resetValues[index];
				*World().Get<RadarAvailability>(slot.entity) = {
					value.producers, value.resistantProducers, value.suppressed};
			}
		InitializeSimulation();
	}

	void Discard() noexcept
	{
		resetValues.clear();
		resetEntities.clear();
		prepared = false;
	}
};

RadarStore::RadarStore(GameplayState &state, RadarExecutionConfig config) : m_impl(new Impl(state, config)) {}
RadarStore::RadarStore(GameplayState &state, GameplayWorkers &workers, RadarExecutionConfig config) :
	m_impl(new Impl(state, config, &workers)) {}

RadarStore::~RadarStore() noexcept
{
	if (Count() || m_impl->prepared)
		std::terminate();
	delete m_impl;
}

std::size_t RadarStore::Count() const noexcept { return m_impl->count; }

std::span<const RadarSound> RadarStore::Execute(std::uint64_t tick)
{
	auto &impl = *m_impl;
	if (impl.failed || impl.prepared || impl.World().IsScheduledExecutionActive())
		throw std::logic_error("Cannot execute unavailable radar simulation");
	try
	{
		const auto ranges = impl.batch.Prepare(impl.slots.size());
		for (std::size_t index = 0; index < impl.slots.size(); ++index)
			if (impl.slots[index].active)
			{
				auto *range = impl.World().Get<RadarBatchRange>(impl.slots[index].entity);
				if (!range) throw std::logic_error("Missing bound radar batch range");
				*range = ranges[index];
			}
		impl.simulation->Execute(engine::time::SimulationTime{tick, engine::time::FixedStep{impl.config.ticksPerSecond}});
		impl.sounds.clear();
		for (std::size_t sequence = 0; sequence < impl.batch.Size(); ++sequence)
		{
			const auto transition = impl.batch.Result(sequence);
			const auto input = impl.presentation[sequence];
			if (input.eligible && transition != RadarTransition::None)
				impl.sounds.push_back({input.player, transition == RadarTransition::Online});
		}
		impl.batch.Clear();
		impl.presentation.clear();
		return impl.sounds;
	}
	catch (...)
	{
		// A prefix of state may have changed. Never replay or publish this run.
		impl.failed = true;
		impl.batch.Clear();
		impl.presentation.clear();
		impl.sounds.clear();
		throw;
	}
}

PersistentResetParticipant RadarStore::ResetParticipant()
{
	return {"games.generalszh.radar", &m_impl->state, m_impl,
		+[](void *context) { return static_cast<Impl *>(context)->Capture(); },
		+[](void *context) { static_cast<Impl *>(context)->Release(); },
		+[](void *context) { static_cast<Impl *>(context)->Restore(); },
		+[](void *context) noexcept { static_cast<Impl *>(context)->Discard(); },
		+[](void *context) noexcept { static_cast<Impl *>(context)->failed = true; }, 1};
}

RadarLease::RadarLease(RadarStore &store, RadarValue initial) : m_store(store)
{
	auto &impl = *store.m_impl;
	if (impl.failed || impl.prepared)
		throw std::logic_error("Cannot bind unavailable radar storage");
	const bool append = impl.free == RadarStore::Impl::None;
	if (append)
		impl.slots.emplace_back(); // Allocate bookkeeping before live state.
	m_slot = append ? impl.slots.size() - 1 : impl.free;
	ecs::Entity entity;
	try
	{
		entity = impl.World().Create<RadarAvailability, RadarBatchRange>();
	}
	catch (...)
	{
		if (append)
			impl.slots.pop_back();
		throw;
	}
	if (!append)
		impl.free = impl.slots[m_slot].next;
	impl.slots[m_slot] = {entity, RadarStore::Impl::None, true};
	*impl.World().Get<RadarAvailability>(entity) = {
		initial.producers, initial.resistantProducers, initial.suppressed};
	++impl.count;
}

RadarLease::~RadarLease() noexcept
{
	auto &impl = *m_store.m_impl;
	if (impl.prepared)
		std::terminate();
	auto &slot = impl.slots[m_slot];
	impl.batch.Cancel(m_slot);
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

RadarValue RadarLease::State() const
{
	const auto &availability = m_store.m_impl->Require(m_slot);
	return {availability.producers, availability.resistantProducers, availability.suppressed};
}

void RadarLease::Restore(RadarValue value)
{
	m_store.m_impl->Require(m_slot) = {value.producers, value.resistantProducers, value.suppressed};
	m_store.m_impl->batch.Cancel(m_slot);
}

void RadarLease::Reset()
{
	m_store.m_impl->Require(m_slot) = {};
	m_store.m_impl->batch.Cancel(m_slot);
}

bool RadarLease::HasRadar() const
{
	return engine::gameplay::rts::radar::HasRadar(m_store.m_impl->Require(m_slot));
}

void RadarLease::AddProvider(bool resistant, bool soundEligible, std::int32_t playerIndex)
{
	m_store.m_impl->Record(m_slot, resistant ? RadarAction::AddResistant : RadarAction::Add, soundEligible, playerIndex);
}

void RadarLease::RemoveProvider(bool resistant, bool soundEligible, std::int32_t playerIndex)
{
	m_store.m_impl->Record(m_slot, resistant ? RadarAction::RemoveResistant : RadarAction::Remove, soundEligible, playerIndex);
}

void RadarLease::SetSuppressed(bool suppressed, bool soundEligible, std::int32_t playerIndex)
{
	m_store.m_impl->Record(m_slot, suppressed ? RadarAction::Suppress : RadarAction::Unsuppress, soundEligible, playerIndex);
}
}
}
