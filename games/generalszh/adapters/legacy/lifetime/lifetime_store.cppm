module;
#include "lifetime_store.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include "games/generalszh/simulation/execution/gameplay_workers.h"
#include "games/generalszh/simulation/lifecycle/world_binding.h"
#include <algorithm>
#include <exception>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <vector>
export module games.generalszh.adapters.legacy.lifetime.lifetime_store;
import games.generalszh.simulation.lifetime.lifetime_simulation;
import games.generalszh.simulation.reset.persistent_world_reset;

extern "C++"
{
namespace generalszh::legacy
{
using namespace engine::gameplay::lifetime;
using namespace generalszh::lifetime;

struct LifetimeStore::Impl
{
	using InputQuery = ecs::Query<ecs::Read<LifetimeTarget>, ecs::Write<ExpirationEligibility>>;
	using ResultQuery = ecs::Query<ecs::Read<LifetimeTarget>, ecs::Read<LifetimeDispatch>>;
	enum class Stage { Idle, Inputs, Effects, Reset };
	std::unique_ptr<GameplayWorkers> ownedWorkers;
	std::unique_ptr<GameplayState> ownedState;
	GameplayWorkers *workers;
	GameplayState *state;
	LifetimeExecutionConfig config;
	std::optional<GameplayWorldBinding> binding;
	std::unique_ptr<LifetimeSimulation> simulation;
	std::unique_ptr<InputQuery> inputQuery;
	std::unique_ptr<ResultQuery> resultQuery;
	std::vector<LifetimeInput> inputs;
	std::vector<ecs::Entity> inputEntities;
	std::vector<LifetimeEffect> effects;
	std::size_t leases{};
	std::uint64_t boundary{}; // Survives World replacement; old effects cannot become valid again.
	Stage stage{Stage::Idle};
	bool failed{false};
	Impl(GameplayState *injectedState, GameplayWorkers *injectedWorkers, LifetimeExecutionConfig configuration) :
		ownedWorkers(injectedWorkers ? nullptr : std::make_unique<GameplayWorkers>(1)),
		ownedState(injectedState ? nullptr : std::make_unique<GameplayState>()),
		workers(injectedWorkers ? injectedWorkers : ownedWorkers.get()),
		state(injectedState ? injectedState : ownedState.get()), config(configuration)
	{
		inputs.reserve(config.capacity);
		inputEntities.reserve(config.capacity);
		effects.reserve(config.capacity);
		Initialize();
	}
	ecs::World &World() const noexcept { return state->World(); }
	void Initialize()
	{
		binding.emplace(*state);
		simulation = std::make_unique<LifetimeSimulation>(World(), *workers);
		simulation->Finalize(engine::time::FixedStep{config.ticksPerSecond});
		inputQuery = std::make_unique<InputQuery>(World());
		resultQuery = std::make_unique<ResultQuery>(World());
	}
	void ReleaseExecution()
	{
		resultQuery.reset(); inputQuery.reset(); simulation.reset(); binding.reset();
		inputs.clear(); inputEntities.clear(); effects.clear();
	}
	void Require() const
	{
		if (failed || stage == Stage::Reset || World().IsScheduledExecutionActive())
			throw std::logic_error("Lifetime storage unavailable during execution/reset or after failure");
	}
};

LifetimeStore::LifetimeStore() : m_impl(new Impl(nullptr, nullptr, {})) {}
LifetimeStore::LifetimeStore(GameplayState &state) : m_impl(new Impl(&state, nullptr, {})) {}
LifetimeStore::LifetimeStore(GameplayState &state, GameplayWorkers &workers, LifetimeExecutionConfig config) :
	m_impl(new Impl(&state, &workers, config)) {}
LifetimeStore::~LifetimeStore() noexcept
{
	if (Count() != 0) std::terminate();
	delete m_impl;
}
std::size_t LifetimeStore::Count() const noexcept { return m_impl->leases; }
void LifetimeStore::Fail() noexcept { m_impl->failed = true; }
void LifetimeStore::Reset()
{
	m_impl->Require();
	if (Count() || m_impl->stage != Impl::Stage::Idle)
		throw std::logic_error("Release lifetime leases and complete effects before reset");
	if (m_impl->ownedState)
	{
		const auto participant = ResetParticipant();
		PersistentWorldReset::Execute(*m_impl->state, {&participant, 1});
	}
}
PersistentResetParticipant LifetimeStore::ResetParticipant()
{
	return {"games.generalszh.lifetime", m_impl->state, m_impl,
		+[](void *context) -> std::span<const ecs::Entity> {
			auto &impl = *static_cast<Impl *>(context);
			impl.Require();
			if (impl.leases || impl.stage != Impl::Stage::Idle)
				throw std::logic_error("Lifetime execution is not ready for world reset");
			impl.stage = Impl::Stage::Reset;
			return {};
		},
		+[](void *context) { static_cast<Impl *>(context)->ReleaseExecution(); },
		+[](void *context) { static_cast<Impl *>(context)->Initialize(); },
		+[](void *context) noexcept { static_cast<Impl *>(context)->stage = Impl::Stage::Idle; },
		+[](void *context) noexcept { static_cast<Impl *>(context)->failed = true; }, 1};
}

std::span<LifetimeInput> LifetimeStore::PrepareInputs()
{
	auto &impl = *m_impl;
	impl.Require();
	if (impl.stage != Impl::Stage::Idle) throw std::logic_error("Lifetime boundary already open");
	if (impl.boundary == (std::numeric_limits<std::uint64_t>::max)())
		throw std::overflow_error("Lifetime boundary identity exhausted");
	try
	{
		impl.inputs.clear(); impl.inputEntities.clear();
		impl.inputQuery->ForEachChunk([&](auto chunk) {
			const auto targets = chunk.template Get<LifetimeTarget>();
			for (std::size_t row = 0; row != targets.size(); ++row)
			{
				if (impl.inputs.size() == impl.config.capacity)
					throw std::length_error("Lifetime input capacity exhausted");
				impl.inputs.push_back({targets[row].object, false});
				impl.inputEntities.push_back(chunk.Entities()[row]);
			}
		});
		if (impl.inputs.size() != impl.leases)
			throw std::logic_error("Lifetime execution requires exclusive timer ownership in its World");
		impl.stage = Impl::Stage::Inputs;
		++impl.boundary;
		return impl.inputs;
	}
	catch (...) { Fail(); throw; }
}

std::span<const LifetimeEffect> LifetimeStore::Execute(std::uint64_t tick)
{
	auto &impl = *m_impl;
	impl.Require();
	if (impl.stage != Impl::Stage::Inputs) throw std::logic_error("Prepare lifetime inputs before execution");
	try
	{
		std::size_t sequence = 0;
		impl.inputQuery->ForEachChunk([&](auto chunk) {
			const auto targets = chunk.template Get<LifetimeTarget>();
			auto eligibility = chunk.template Get<ExpirationEligibility>();
			for (std::size_t row = 0; row != targets.size(); ++row, ++sequence)
			{
				if (sequence >= impl.inputs.size() || chunk.Entities()[row] != impl.inputEntities[sequence]
					|| targets[row].object != impl.inputs[sequence].object)
					throw std::logic_error("Lifetime identities changed while inputs were prepared");
				eligibility[row].enabled = impl.inputs[sequence].enabled;
			}
		});
		if (sequence != impl.inputs.size()) throw std::logic_error("Lifetime inputs no longer match world state");
		impl.simulation->Execute({tick, engine::time::FixedStep{impl.config.ticksPerSecond}});
		impl.effects.clear();
		impl.resultQuery->ForEachChunk([&](auto chunk) {
			const auto targets = chunk.template Get<LifetimeTarget>();
			const auto dispatch = chunk.template Get<LifetimeDispatch>();
			for (std::size_t row = 0; row != targets.size(); ++row)
				if (dispatch[row].ready)
				{
					if (impl.effects.size() == impl.config.capacity)
						throw std::length_error("Lifetime effect capacity exhausted");
					const auto entity = chunk.Entities()[row];
					impl.effects.push_back({targets[row].object, entity.index, entity.generation,
						dispatch[row].revision, impl.boundary, targets[row].action == LifetimeAction::Destroy});
				}
		});
		// Logical deaths before cleanup; explicit identity order within each kind.
		std::sort(impl.effects.begin(), impl.effects.end(), [](const auto &a, const auto &b) {
			return std::tie(a.destroy, a.object, a.index, a.generation)
				< std::tie(b.destroy, b.object, b.index, b.generation);
		});
		impl.stage = Impl::Stage::Effects;
		return impl.effects;
	}
	catch (...) { Fail(); impl.effects.clear(); throw; }
}

bool LifetimeStore::Consume(const LifetimeEffect &effect)
{
	auto &impl = *m_impl;
	impl.Require();
	if (impl.stage != Impl::Stage::Effects) throw std::logic_error("No lifetime effects open for commit");
	if (effect.boundary != impl.boundary) return false;
	const ecs::Entity entity{effect.index, effect.generation};
	auto *dispatch = impl.World().Get<LifetimeDispatch>(entity);
	const auto *target = impl.World().Get<LifetimeTarget>(entity);
	auto *expiration = impl.World().Get<Expiration>(entity);
	if (!dispatch || !target || !expiration || dispatch->revision != effect.revision || !dispatch->ready)
		return false; // Destroyed/reused, canceled/rearmed, or already consumed.
	if (target->object != effect.object || (target->action == LifetimeAction::Destroy) != effect.destroy)
		throw std::logic_error("Lifetime effect identity was modified");
	dispatch->ready = false;
	// One-shot state has a single authoritative owner. The transient output
	// column can be rebuilt without redelivering a consumed expiration.
	expiration->state = ExpirationState::Inactive;
	return true;
}
void LifetimeStore::FinishEffects()
{
	m_impl->Require();
	if (m_impl->stage != Impl::Stage::Effects) throw std::logic_error("No lifetime effects to finish");
	// Every surviving result must be acknowledged, even if its game target is gone.
	for (const auto &effect : m_impl->effects)
	{
		const auto *dispatch = m_impl->World().Get<LifetimeDispatch>({effect.index, effect.generation});
		if (dispatch && dispatch->revision == effect.revision && dispatch->ready)
		{
			Fail();
			throw std::logic_error("Unconsumed lifetime effects cannot be silently dropped");
		}
	}
	m_impl->stage = Impl::Stage::Idle;
	m_impl->effects.clear();
}

LifetimeLease::LifetimeLease(LifetimeStore &store, std::uint32_t object, bool destroy) : m_store(store)
{
	auto &impl = *store.m_impl;
	impl.Require();
	if (impl.stage == LifetimeStore::Impl::Stage::Inputs)
		throw std::logic_error("Cannot bind a timer during input preparation");
	if (impl.leases == impl.config.capacity) throw std::length_error("Lifetime entity capacity exhausted");
	const auto entity = impl.World().Create<Expiration, ExpirationEligibility, LifetimeTarget, LifetimeDispatch>();
	*impl.World().Get<LifetimeTarget>(entity) = {object, destroy ? LifetimeAction::Destroy : LifetimeAction::Kill};
	m_index = entity.index; m_generation = entity.generation;
	++impl.leases;
}
LifetimeLease::~LifetimeLease() noexcept
{
	auto &impl = *m_store.m_impl;
	if ((!impl.failed && impl.stage == LifetimeStore::Impl::Stage::Inputs)
		|| !impl.World().Destroy({m_index, m_generation})) std::terminate();
	--impl.leases;
}
std::uint32_t LifetimeLease::Deadline() const
{
	m_store.m_impl->Require();
	const auto *expiration = m_store.m_impl->World().Get<Expiration>({m_index, m_generation});
	if (!expiration) throw std::logic_error("Stale lifetime lease");
	return static_cast<std::uint32_t>(expiration->deadline);
}
void LifetimeLease::SetDeadline(std::uint32_t frame)
{
	auto &impl = *m_store.m_impl;
	impl.Require();
	const ecs::Entity entity{m_index, m_generation};
	auto *expiration = impl.World().Get<Expiration>(entity);
	auto *dispatch = impl.World().Get<LifetimeDispatch>(entity);
	if (!expiration || !dispatch) throw std::logic_error("Stale lifetime lease");
	if (dispatch->revision == (std::numeric_limits<std::uint64_t>::max)())
		throw std::overflow_error("Lifetime revision exhausted; refusing ABA reuse");
	*dispatch = {dispatch->revision + 1, false};
	*expiration = {frame, ExpirationState::Armed};
}
void LifetimeLease::SetObject(std::uint32_t object)
{
	auto &impl = *m_store.m_impl;
	impl.Require();
	if (impl.stage == LifetimeStore::Impl::Stage::Inputs)
		throw std::logic_error("Cannot remap a timer while inputs are being prepared");
	const ecs::Entity entity{m_index, m_generation};
	auto *target = impl.World().Get<LifetimeTarget>(entity);
	auto *dispatch = impl.World().Get<LifetimeDispatch>(entity);
	if (!target || !dispatch) throw std::logic_error("Stale lifetime binding");
	if (dispatch->revision == (std::numeric_limits<std::uint64_t>::max)())
		throw std::overflow_error("Lifetime revision exhausted; refusing ABA reuse");
	++dispatch->revision;
	dispatch->ready = false;
	target->object = object;
}
}
}
