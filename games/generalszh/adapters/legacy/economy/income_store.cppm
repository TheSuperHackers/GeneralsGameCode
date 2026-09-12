module;
#include "income_store.h"
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
export module games.generalszh.adapters.legacy.economy.income_store;
import games.generalszh.simulation.income.income_simulation;
import games.generalszh.simulation.reset.persistent_world_reset;
extern "C++"
{
namespace generalszh::legacy
{
using namespace engine::gameplay::rts::economy;
using namespace generalszh::economy;
struct IncomeStore::Impl
{
    using Inputs = ecs::Query<ecs::Read<IncomeSource>, ecs::Write<IncomeObservation>, ecs::Write<CaptureRange>,
        ecs::Read<IncomeSchedule>, ecs::Read<IncomeRate>, ecs::Read<IncomeEligibility>, ecs::Read<IncomeDispatch>,
        ecs::Read<IncomePulse>, ecs::Read<CaptureRewardState>, ecs::Read<IncomeDefinition>, ecs::Read<IncomePayout>>;
    using Outputs = ecs::Query<ecs::Read<IncomeSource>, ecs::Read<IncomeObservation>, ecs::Read<IncomePayout>>;
    struct Slot { ecs::Entity entity{}; bool live{}; };
    struct Request { std::size_t slot; ecs::Entity entity; CaptureRequest value; std::uint32_t object; };
    enum class Stage { Idle, Inputs, Effects, Reset };
    GameplayState &state;
    IncomeExecutionConfig config;
    std::unique_ptr<GameplayWorkers> ownedWorkers;
    GameplayWorkers *workers;
    CaptureBatch batch;
    std::optional<GameplayWorldBinding> binding;
    std::unique_ptr<IncomeSimulation> simulation;
    std::unique_ptr<Inputs> inputsQuery;
    std::unique_ptr<Outputs> outputsQuery;
    std::vector<Slot> slots;
    std::vector<std::size_t> free;
    std::vector<IncomeInput> inputs;
    std::vector<ecs::Entity> identities;
    std::vector<Request> pending, captured;
    struct Periodic { ecs::Entity entity; IncomeEffect effect; };
    std::vector<Periodic> periodic;
    std::vector<IncomeEffect> effects;
    std::size_t count{}, cursor{};
    Stage stage{Stage::Idle};
    bool failed{false}, hasTick{false};
    std::uint64_t lastTick{};
    Impl(GameplayState &owner, GameplayWorkers *shared, IncomeExecutionConfig settings) :
        state(owner), config(settings), ownedWorkers(shared ? nullptr : std::make_unique<GameplayWorkers>(1)),
        workers(shared ? shared : ownedWorkers.get()), batch(settings.capacity)
    {
        if (config.capacity > (std::numeric_limits<std::size_t>::max)() / 2)
            throw std::length_error("Income capacity exceeds representable output bounds");
        slots.reserve(config.capacity); free.reserve(config.capacity);
        inputs.reserve(config.capacity); identities.reserve(config.capacity);
        pending.reserve(config.capacity); captured.reserve(config.capacity);
        periodic.reserve(config.capacity); effects.reserve(config.capacity * 2);
        Initialize();
    }
    ecs::World &World() const noexcept { return state.World(); }
    void Require() const
    {
        if (failed || stage == Stage::Reset || World().IsScheduledExecutionActive())
            throw std::logic_error("Income unavailable during execution/reset or after failure");
    }
    void Initialize()
    {
        binding.emplace(state);
        simulation = std::make_unique<IncomeSimulation>(World(), batch, *workers);
        simulation->Finalize(engine::time::FixedStep{config.ticksPerSecond});
        inputsQuery = std::make_unique<Inputs>(World());
        outputsQuery = std::make_unique<Outputs>(World());
        hasTick = false;
    }
    void Release()
    {
        outputsQuery.reset(); inputsQuery.reset(); simulation.reset(); binding.reset();
        batch.Clear(); pending.clear(); captured.clear(); effects.clear(); periodic.clear();
    }
    template<class T> T &Component(ecs::Entity entity) const
    {
        Require();
        auto *value = World().Get<T>(entity);
        if (!value) throw std::logic_error("Stale income lease or missing component");
        return *value;
    }
};
IncomeStore::IncomeStore(GameplayState &state) : m_impl(new Impl(state, nullptr, {})) {}
IncomeStore::IncomeStore(GameplayState &state, GameplayWorkers &workers, IncomeExecutionConfig config) :
    m_impl(new Impl(state, &workers, config)) {}
IncomeStore::~IncomeStore() noexcept
{
    if (Count()) std::terminate();
    delete m_impl;
}
std::size_t IncomeStore::Count() const noexcept { return m_impl->count; }
void IncomeStore::Fail() noexcept { m_impl->failed = true; }
PersistentResetParticipant IncomeStore::ResetParticipant()
{
    return {"games.generalszh.income", &m_impl->state, m_impl,
        +[](void *context) -> std::span<const ecs::Entity> {
            auto &impl = *static_cast<Impl *>(context);
            impl.Require();
            if (impl.count || impl.stage != Impl::Stage::Idle)
                throw std::logic_error("Release income sources and finish effects before reset");
            impl.stage = Impl::Stage::Reset; return {};
        },
        +[](void *context) { static_cast<Impl *>(context)->Release(); },
        +[](void *context) { static_cast<Impl *>(context)->Initialize(); },
        +[](void *context) noexcept { static_cast<Impl *>(context)->stage = Impl::Stage::Idle; },
        +[](void *context) noexcept { static_cast<Impl *>(context)->failed = true; }, 1};
}
std::span<IncomeInput> IncomeStore::PrepareInputs()
{
    auto &impl = *m_impl;
    impl.Require();
    if (impl.stage != Impl::Stage::Idle) throw std::logic_error("Income boundary already open");
    try
    {
        impl.inputs.clear(); impl.identities.clear();
        impl.captured.swap(impl.pending); impl.pending.clear();
        std::erase_if(impl.captured, [&](const auto &request) {
            return request.slot >= impl.slots.size() || !impl.slots[request.slot].live
                || impl.slots[request.slot].entity != request.entity;
        });
        for (const auto &request : impl.captured) impl.batch.Record(request.slot, request.value);
        const auto ranges = impl.batch.Prepare(impl.slots.size());
        impl.inputsQuery->ForEachChunk([&](auto chunk) {
            const auto sources = chunk.template Get<IncomeSource>();
            auto componentRanges = chunk.template Get<CaptureRange>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                const auto &source = sources[row];
                if (impl.inputs.size() == impl.config.capacity || source.slot >= impl.slots.size()
                    || !impl.slots[source.slot].live || impl.slots[source.slot].entity != chunk.Entities()[row])
                    throw std::logic_error("Income source ownership/capacity mismatch");
                componentRanges[row] = ranges[source.slot];
                impl.inputs.push_back({source.object, source.moduleTag});
                impl.identities.push_back(chunk.Entities()[row]);
            }
        });
        if (impl.inputs.size() != impl.count) throw std::logic_error("Income sources missing query components");
        impl.stage = Impl::Stage::Inputs;
        return impl.inputs;
    }
    catch (...) { Fail(); throw; }
}
void IncomeStore::Execute(std::uint64_t tick)
{
    auto &impl = *m_impl;
    impl.Require();
    if (impl.stage != Impl::Stage::Inputs) throw std::logic_error("Prepare income inputs first");
    try
    {
        if (impl.hasTick && tick <= impl.lastTick) throw std::logic_error("Income requires strictly increasing ticks");
        for (const auto &request : impl.captured)
            if (request.value.tick > tick) throw std::logic_error("Future capture input at income boundary");
        std::size_t sequence = 0;
        impl.inputsQuery->ForEachChunk([&](auto chunk) {
            const auto sources = chunk.template Get<IncomeSource>();
            const auto definitions = chunk.template Get<IncomeDefinition>();
            auto observations = chunk.template Get<IncomeObservation>();
            for (std::size_t row = 0; row != chunk.Count(); ++row, ++sequence)
            {
                if (sequence >= impl.inputs.size() || chunk.Entities()[row] != impl.identities[sequence])
                    throw std::logic_error("Income identities changed while preparing input");
                const auto &input = impl.inputs[sequence];
                if (sources[row].object != input.object || sources[row].moduleTag != input.moduleTag)
                    throw std::logic_error("Income input identity changed");
                if (input.active && !input.neutral && input.player < 0)
                    throw std::invalid_argument("Active non-neutral income source has no recipient");
                ValidateIncomeAmount(definitions[row].base, input.boost);
                observations[row] = {input.player, input.boost, input.active, input.neutral, input.constructionComplete, input.visible, input.dispatchEnabled};
            }
        });
        if (sequence != impl.inputs.size()) throw std::logic_error("Income input cardinality changed");
        impl.simulation->Execute({tick, engine::time::FixedStep{impl.config.ticksPerSecond}});
        impl.effects.clear(); impl.periodic.clear(); impl.cursor = 0;
        for (std::size_t index = 0; index != impl.captured.size(); ++index)
        {
            const auto &request = impl.captured[index];
            const auto &payout = impl.batch.Result(index);
            if (payout.pay || payout.show)
                impl.effects.push_back({request.object, payout.money, request.value.player,
                    payout.score, payout.displayAmount, payout.pay, payout.show, true});
        }
        impl.outputsQuery->ForEachChunk([&](auto chunk) {
            const auto sources = chunk.template Get<IncomeSource>();
            const auto observations = chunk.template Get<IncomeObservation>();
            const auto payouts = chunk.template Get<IncomePayout>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                const auto &payout = payouts[row].value;
                if (payout.pay || payout.show)
                    impl.periodic.push_back({chunk.Entities()[row], {sources[row].object, payout.money,
                        observations[row].player, payout.score, payout.displayAmount, payout.pay, payout.show, false}});
            }
        });
        std::sort(impl.periodic.begin(), impl.periodic.end(), [](const auto &a, const auto &b) {
            return std::tie(a.effect.object, a.entity.index, a.entity.generation)
                < std::tie(b.effect.object, b.entity.index, b.entity.generation);
        });
        for (const auto &record : impl.periodic) impl.effects.push_back(record.effect);
        impl.lastTick = tick; impl.hasTick = true; impl.stage = Impl::Stage::Effects;
    }
    catch (...) { Fail(); impl.effects.clear(); throw; }
}
bool IncomeStore::NextEffect(IncomeEffect &effect)
{
    auto &impl = *m_impl;
    impl.Require();
    if (impl.stage != Impl::Stage::Effects) throw std::logic_error("No income effects available");
    if (impl.cursor == impl.effects.size()) return false;
    effect = impl.effects[impl.cursor++];
    return true;
}
void IncomeStore::FinishEffects()
{
    auto &impl = *m_impl;
    impl.Require();
    if (impl.stage != Impl::Stage::Effects || impl.cursor != impl.effects.size())
    { Fail(); throw std::logic_error("Income effects must be consumed exactly once"); }
    impl.batch.Clear(); impl.captured.clear(); impl.effects.clear(); impl.periodic.clear();
    impl.stage = Impl::Stage::Idle;
}
IncomeLease::IncomeLease(IncomeStore &store, std::uint32_t frame, std::uint32_t interval,
    std::uint32_t object, std::uint32_t tag, std::int32_t base, std::int32_t bonus, bool actual) : m_store(store)
{
    auto &impl = *store.m_impl;
    impl.Require();
    if (impl.stage == IncomeStore::Impl::Stage::Inputs) throw std::logic_error("Cannot bind income while preparing inputs");
    if (impl.count == impl.config.capacity) throw std::length_error("Income source capacity exhausted");
    if (interval > UINT32_MAX - frame) throw std::overflow_error("Native income deadline exhausted");
    const auto entity = impl.World().Create<IncomeSchedule, IncomeRate, IncomeEligibility, IncomeDispatch,
        IncomePulse, CaptureRewardState, IncomeDefinition, IncomeSource, IncomeObservation, IncomePayout, CaptureRange>();
    m_index = entity.index; m_generation = entity.generation;
    m_slot = impl.free.empty() ? impl.slots.size() : impl.free.back();
    if (impl.free.empty()) impl.slots.push_back({entity, true});
    else { impl.free.pop_back(); impl.slots[m_slot] = {entity, true}; }
    *impl.World().Get<IncomeSource>(entity) = {object, tag, m_slot};
    *impl.World().Get<IncomeDefinition>(entity) = {base, bonus, actual};
    *impl.World().Get<IncomeRate>(entity) = {interval, base > 0 ? static_cast<std::uint64_t>(base) : 0};
    *impl.World().Get<IncomeSchedule>(entity) = {static_cast<std::uint64_t>(frame) + interval};
    ++impl.count;
}
IncomeLease::~IncomeLease() noexcept
{
    auto &impl = *m_store.m_impl;
    if ((!impl.failed && impl.stage == IncomeStore::Impl::Stage::Inputs)
        || !impl.World().Destroy({m_index, m_generation})) std::terminate();
    impl.slots[m_slot].live = false; impl.free.push_back(m_slot); --impl.count;
}
bool IncomeLease::BeginUpdate(std::uint32_t, std::uint32_t)
{ throw std::logic_error("Income dispatch belongs to the ECS execution boundary"); }
void IncomeLease::Capture(std::uint32_t frame, std::int32_t player)
{
    auto &impl = *m_store.m_impl;
    impl.Require();
    if (impl.stage == IncomeStore::Impl::Stage::Inputs) throw std::logic_error("Capture recorded during input preparation");
    try
    {
        if (impl.pending.size() == impl.config.capacity) throw std::length_error("Capture input capacity exhausted");
        if (impl.Component<IncomeRate>({m_index, m_generation}).intervalTicks > UINT32_MAX - frame)
            throw std::overflow_error("Native capture deadline exhausted");
        impl.pending.push_back({m_slot, {m_index, m_generation}, {frame, player},
            impl.Component<IncomeSource>({m_index, m_generation}).object});
    }
    catch (...) { m_store.Fail(); throw; }
}
void IncomeLease::Rearm(std::uint32_t frame, std::uint32_t interval)
{
    auto &impl = *m_store.m_impl;
    impl.Require();
    if (impl.stage != IncomeStore::Impl::Stage::Idle) throw std::logic_error("Rearm only outside an income boundary");
    if (interval > UINT32_MAX - frame) throw std::overflow_error("Native income deadline exhausted");
    impl.Component<IncomeSchedule>({m_index, m_generation}).nextTick = static_cast<std::uint64_t>(frame) + interval;
}
void IncomeLease::ConsumeCaptureBonus()
{
    auto &impl = *m_store.m_impl;
    impl.Require();
    if (impl.stage != IncomeStore::Impl::Stage::Idle) throw std::logic_error("Direct reward setup is forbidden during a boundary");
    impl.Component<CaptureRewardState>({m_index, m_generation}).available = false;
}
void IncomeLease::SetObject(std::uint32_t object)
{
    auto &impl = *m_store.m_impl;
    impl.Require();
    if (impl.stage != IncomeStore::Impl::Stage::Idle) throw std::logic_error("Remap income only outside execution");
    std::erase_if(impl.pending, [&](const auto &request) { return request.entity == ecs::Entity{m_index, m_generation}; });
    impl.Component<IncomeSource>({m_index, m_generation}).object = object;
}
IncomeStateRecord IncomeLease::State() const
{
    auto &impl = *m_store.m_impl;
    const ecs::Entity entity{m_index, m_generation};
    const auto &schedule = impl.Component<IncomeSchedule>(entity);
    const auto &reward = impl.Component<CaptureRewardState>(entity);
    if (schedule.nextTick > UINT32_MAX) throw std::overflow_error("Income deadline cannot cross the native frame boundary");
    return {static_cast<std::uint32_t>(schedule.nextTick), reward.available, reward.initialized};
}
void IncomeLease::Restore(IncomeStateRecord state)
{
    auto &impl = *m_store.m_impl;
    impl.Require();
    if (impl.stage != IncomeStore::Impl::Stage::Idle) throw std::logic_error("Restore income only outside execution");
    const ecs::Entity entity{m_index, m_generation};
    auto &schedule = impl.Component<IncomeSchedule>(entity);
    auto &reward = impl.Component<CaptureRewardState>(entity);
    schedule = {state.deadline}; reward = {state.bonusAvailable, state.initialized};
    std::erase_if(impl.pending, [&](const auto &request) { return request.entity == entity; });
}
}
}
