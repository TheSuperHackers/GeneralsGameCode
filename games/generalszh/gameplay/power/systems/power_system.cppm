module;
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>
export module games.generalszh.gameplay.power.systems.power_system;
export import games.generalszh.gameplay.power.data.power_frame;
export import engine.gameplay.rts.power.components.power_ledger;
export import engine.gameplay.rts.power.algorithms.power_projection;
export import engine.gameplay.combat.systems.health_system;
export namespace generalszh::power
{
using engine::gameplay::rts::power::PowerLedger;
using engine::gameplay::rts::power::PowerSource;
using engine::gameplay::rts::power::PowerContribution;
// One power behaviour: parallel source projection, then canonical account reduction.
// The shared scheduler owns the join; no child systems or intermediate commits.
class PowerSystem
{
    using Accounts = ecs::Query<ecs::Write<PowerLedger>, ecs::Write<PowerState>, ecs::Optional<PowerPolicy>>;
    Accounts accounts;
    struct Total { ecs::Entity account{}; std::uint64_t production{}, consumption{}; };
    std::vector<Total> totals;
    PowerFrame &frame;
    const PowerConfig config;
public:
    using Query = ecs::Query<ecs::Read<PowerSource>, ecs::Write<PowerContribution>,
        ecs::Optional<engine::gameplay::combat::LifeState>>;
    using AuxiliaryAccess = Accounts;
    PowerSystem(ecs::World &world, PowerFrame &snapshot, PowerConfig definition = {}) :
        accounts(world), totals(snapshot.Capacity()), frame(snapshot), config(definition)
    {
        if (config.minimumSpeed > config.maximumSpeed || config.maximumSpeed > 10000)
            throw std::invalid_argument("Power speed bounds must be ordered within 0..10000");
    }
    void BeforeChunks(Query &, ecs::SystemContext &)
    {
        std::fill(totals.begin(), totals.end(), Total{});
        accounts.ForEachChunk([&](auto chunk) {
            for (auto entity : chunk.Entities())
            {
                if (entity.index >= totals.size())
                    throw std::length_error("Power snapshot entity capacity exhausted");
                totals[entity.index].account = entity;
            }
        });
    }
    void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
    {
        const auto sources = chunk.Get<PowerSource>();
        const auto lives = chunk.Get<engine::gameplay::combat::LifeState>();
        auto values = chunk.Get<PowerContribution>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
            values[row] = engine::gameplay::rts::power::ProjectPower(sources[row], lives.empty() || lives[row].alive);
    }
    void AfterChunks(Query &sources, ecs::SystemContext &)
    {
        sources.ForEachChunk([&](auto chunk) {
            const auto source = chunk.template Get<PowerSource>();
            const auto value = chunk.template Get<PowerContribution>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                const auto account = source[row].account;
                if (!account.IsValid() || account.index >= totals.size() || totals[account.index].account != account) continue; // Stale/destroyed owners cannot credit reused IDs.
                auto &entry = totals[account.index];
                entry.production += value[row].production;
                entry.consumption += value[row].consumption;
            }
        });
        // Never wrap or silently drop authoritative power. Validate joined totals
        // before publishing any account; the scheduler treats failure as terminal.
        for (const auto &entry : totals)
            if (entry.production > static_cast<std::uint64_t>((std::numeric_limits<std::int32_t>::max)())
                || entry.consumption > static_cast<std::uint64_t>((std::numeric_limits<std::int32_t>::max)()))
                throw std::overflow_error("Power ledger capacity exhausted");
        frame.Clear();
        accounts.ForEachChunk([&](auto chunk) {
            auto ledgers = chunk.template Get<PowerLedger>();
            auto states = chunk.template Get<PowerState>();
            const auto policies = chunk.template Get<PowerPolicy>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                auto &entry = totals[chunk.Entities()[row].index];
                ledgers[row] = {static_cast<std::int32_t>(entry.production), static_cast<std::int32_t>(entry.consumption)};
                auto &state = states[row]; state = {};
                state.brownout = entry.production < entry.consumption;
                if (!policies.empty() && policies[row].affected)
                {
                    // Energy::getEnergySupplyRatio uses raw production when draw
                    // is zero. Preserve its 0/0 -> 0 rate while brownout stays false.
                    const auto denominator = std::max(entry.consumption, std::uint64_t{1});
                    if (entry.production < denominator)
                    {
                        const auto shortage = denominator - entry.production;
                        // Both factors are bounded by uint32; multiplication fits uint64.
                        const auto penalty = shortage * config.penaltyModifier;
                        const auto full = denominator * 10000;
                        auto speed = penalty >= full ? std::uint64_t{0} : (full - penalty) / denominator;
                        speed = std::clamp(speed, std::uint64_t{config.minimumSpeed}, std::uint64_t{config.maximumSpeed});
                        state.speedNumerator = static_cast<std::uint32_t>(speed == 0 ? 100 : speed);
                    }
                }
                frame.Publish(chunk.Entities()[row], state);
            }
        });
    }
};

}
export namespace ecs
{
template<> struct SystemTraits<generalszh::power::PowerSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.power.system";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
