module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module engine.gameplay.concealment.systems.concealment_system;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.concealment.algorithms.detection_lease;
export import engine.gameplay.concealment.algorithms.detection_index;
export import engine.gameplay.concealment.components.concealment_binding;
export import engine.gameplay.concealment.components.concealment_eligibility;
export import engine.gameplay.concealment.components.concealment_state;
export import engine.gameplay.concealment.definitions.concealment_definition;
export import engine.gameplay.concealment.definitions.detection_definition;
export import engine.gameplay.concealment.inputs.uncloak_input;

export namespace engine::gameplay::concealment
{
class ConcealmentSystem
{
public:
    using Query = ecs::Query<ecs::Read<combat::LifeState>, ecs::Read<ConcealmentBinding>,
        ecs::Read<ConcealmentEligibility>, ecs::Write<ConcealmentState>>;
    using AuxiliaryAccess = DetectionIndex::Access;

    ConcealmentSystem(ecs::World &world, const ConcealmentDefinitions &concealmentDefinitions,
        const DetectionDefinitions &detectionDefinitions, DetectionIndex &detection, std::size_t inputCapacity) :
        world(world), concealmentDefinitions(concealmentDefinitions), detectionDefinitions(detectionDefinitions),
        detection(detection), inputCapacity(inputCapacity)
    {
        inputs.reserve(inputCapacity);
    }

    void SetInputs(std::span<const UncloakInput> value)
    {
        if (world.IsScheduledExecutionActive()) throw std::logic_error("Uncloak inputs require a joined boundary");
        if (value.size() > inputCapacity) throw std::length_error("Uncloak input capacity exhausted");
        inputs.assign(value.begin(), value.end());
        std::sort(inputs.begin(), inputs.end(), [](const UncloakInput &left, const UncloakInput &right) {
            return Less(left.actor, right.actor);
        });
    }

    void BeforeChunks(Query &query, ecs::SystemContext &context)
    {
        assert(context.Time().Step() == concealmentDefinitions.Step() &&
            context.Time().Step() == detectionDefinitions.Step());
        ApplyUncloak(query, context.Tick());
        detection.Rebuild(context.Tick(), detectionDefinitions);
        inputs.clear();
    }

    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const
    {
        const auto lives = chunk.Get<combat::LifeState>();
        const auto bindings = chunk.Get<ConcealmentBinding>();
        const auto eligibility = chunk.Get<ConcealmentEligibility>();
        auto states = chunk.Get<ConcealmentState>();
        const auto tick = context.Tick();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            auto &state = states[row];
            const auto &definition = concealmentDefinitions.Get(bindings[row].definition);
            if (!state.initialized) state = StartConcealment(definition, tick);
            if (!lives[row].alive)
            {
                state.concealed = false;
                state.detectedUntilTick = 0;
                state.concealAllowedTick = 0;
                continue;
            }

            state.detectedUntilTick = (std::max)(state.detectedUntilTick, detection.DetectedUntil(chunk.Entities()[row]));
            if (state.detectedUntilTick && tick >= state.detectedUntilTick) state.detectedUntilTick = 0;

            if (!state.enabled)
            {
                state.concealed = false;
                continue;
            }
            if (!eligibility[row].allowed)
            {
                state.concealed = false;
                state.concealAllowedTick = ConcealmentDeadline(tick, definition.stealthDelayTicks);
                continue;
            }
            if (!state.concealed && tick >= state.concealAllowedTick) state.concealed = true;
        }
    }

private:
    static bool Less(ecs::Entity left, ecs::Entity right) noexcept
    {
        return std::tie(left.index, left.generation) < std::tie(right.index, right.generation);
    }

    void ApplyUncloak(Query &query, std::uint64_t tick)
    {
        if (inputs.empty()) return;
        query.ForEachChunk([&](auto chunk) {
            const auto entities = chunk.Entities();
            const auto bindingRows = chunk.template Get<ConcealmentBinding>();
            auto stateRows = chunk.template Get<ConcealmentState>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                const auto entity = entities[row];
                const auto first = std::lower_bound(inputs.begin(), inputs.end(), entity,
                    [](const UncloakInput &input, ecs::Entity candidate) { return Less(input.actor, candidate); });
                for (auto it = first; it != inputs.end() && it->actor == entity; ++it)
                {
                    const auto &definition = concealmentDefinitions.Get(bindingRows[row].definition);
                    if (!stateRows[row].initialized) stateRows[row] = StartConcealment(definition, tick);
                    stateRows[row].detectedUntilTick = (std::max)(stateRows[row].detectedUntilTick,
                        ConcealmentDeadline(tick, definition.stealthDelayTicks));
                }
            }
        });
    }

    ecs::World &world;
    const ConcealmentDefinitions &concealmentDefinitions;
    const DetectionDefinitions &detectionDefinitions;
    DetectionIndex &detection;
    std::size_t inputCapacity;
    std::vector<UncloakInput> inputs;
};
}

export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::concealment::ConcealmentSystem>
{
    static constexpr std::string_view StableName="engine.gameplay.concealment";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<>;
    using After=SystemTypeList<>;
};
}
