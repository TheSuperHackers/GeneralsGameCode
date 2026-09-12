module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>
export module games.generalszh.gameplay.match.systems.victory_system;
export import games.generalszh.gameplay.match.components.match_state;
export import games.generalszh.gameplay.match.systems.defeat_system;
export namespace generalszh::match
{
// One victory decision: asset counting, latched elimination and outcome are
// implementation steps around a required parallel join, not child systems.
class VictorySystem
{
    using Contender = engine::gameplay::rts::match::Contender;
    using Count = engine::gameplay::rts::match::SurvivalCount;
    using Members = ecs::Query<ecs::Read<MatchMember>, ecs::Write<Contender>, ecs::Write<Count>>;
public:
    using Query = ecs::Query<ecs::Read<VictoryAsset>, ecs::Read<engine::gameplay::combat::LifeState>>;
    using AuxiliaryAccess = ecs::Query<ecs::Read<MatchMember>, ecs::Write<Contender>,
        ecs::Write<Count>, ecs::Write<MatchState>>;
    VictorySystem(ecs::World &world, const ecs::Entity &matchEntity, DefeatFrame &frame,
        std::size_t capacity = 65536) : world(world), matchEntity(matchEntity), frame(frame),
        members(world), indexToOrdinal(capacity, UINT32_MAX)
    {
        if (capacity > UINT32_MAX || frame.Capacity() < capacity)
            throw std::invalid_argument("Victory capacity exceeds entity/frame limits");
    }
    void BeforeChunks(Query &query, ecs::SystemContext &)
    {
        frame.Clear(); running = false;
        if (!matchEntity.IsValid()) return;
        const auto *state = world.Get<MatchState>(matchEntity);
        if (!state) throw std::logic_error("Match state was removed externally");
        if (state->outcome.status == MatchStatus::Setup) return;
        if (state->outcome.status != MatchStatus::Running) throw std::logic_error("Finished match cannot advance");
        const auto size = state->rosterSize;
        if (size < 2 || size > indexToOrdinal.size()) throw std::logic_error("Invalid sealed match roster size");
        roster.assign(size, ecs::Entity{});
        std::fill(indexToOrdinal.begin(), indexToOrdinal.end(), UINT32_MAX);
        members.ForEachChunk([&](auto chunk) {
            const auto membership = chunk.template Get<MatchMember>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                const auto &member = membership[row];
                if (member.match != matchEntity) continue;
                const auto account = chunk.Entities()[row];
                if (account.index >= indexToOrdinal.size() || member.ordinal >= size || roster[member.ordinal].IsValid())
                    throw std::logic_error("Invalid or duplicate match membership");
                roster[member.ordinal] = account; indexToOrdinal[account.index] = member.ordinal;
            }
        });
        for (std::size_t i = 0; i != roster.size(); ++i)
            if (!roster[i].IsValid() || (i && roster[i-1].index >= roster[i].index))
                throw std::logic_error("Match roster state was removed or reordered externally");
        if (query.PreparedChunkCount() > (std::numeric_limits<std::size_t>::max)() / size)
            throw std::length_error("Victory reduction capacity overflow");
        counts.assign(query.PreparedChunkCount() * size, 0);
        running = true;
    }
    void Execute(Query::Chunk chunk, ecs::SystemContext &context) noexcept
    {
        if (!running) return;
        const auto assets = chunk.Get<VictoryAsset>();
        const auto life = chunk.Get<engine::gameplay::combat::LifeState>();
        const auto offset = static_cast<std::size_t>(context.ChunkOrder()) * roster.size();
        assert(offset <= counts.size() && roster.size() <= counts.size() - offset);
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto account = assets[row].account;
            if (!life[row].alive || account.index >= indexToOrdinal.size()) continue;
            const auto ordinal = indexToOrdinal[account.index];
            if (ordinal != UINT32_MAX && roster[ordinal] == account) ++counts[offset + ordinal];
        }
    }
    void AfterChunks(Query &query, ecs::SystemContext &context)
    {
        if (!running) return;
        std::size_t survivors = 0; ecs::Entity last{};
        // Canonical account ordinal then logical chunk order. Counts cannot
        // exceed the ECS uint32 entity-index space; uint64 addition is defined.
        for (std::size_t ordinal = 0; ordinal != roster.size(); ++ordinal)
        {
            std::uint64_t total = 0;
            for (std::size_t chunk = 0; chunk != query.PreparedChunkCount(); ++chunk)
                total += counts[chunk * roster.size() + ordinal];
            const auto account = roster[ordinal];
            auto &contender = *world.Get<Contender>(account);
            world.Get<Count>(account)->value = total;
            contender.defeated |= total == 0;
            frame.Set(account, contender.defeated);
            if (!contender.defeated) { ++survivors; last = account; }
        }
        if (survivors < 2)
            world.Get<MatchState>(matchEntity)->outcome = {survivors ? MatchStatus::Won : MatchStatus::Draw,
                survivors ? last : ecs::Entity{}, context.Tick()};
    }
private:
    ecs::World &world;
    const ecs::Entity &matchEntity; // Root-owned identity only; ECS owns all match state.
    DefeatFrame &frame;
    Members members;
    std::vector<std::uint32_t> indexToOrdinal;
    std::vector<ecs::Entity> roster; // Rebuilt from membership every tick, never an authoritative roster.
    std::vector<std::uint64_t> counts;
    bool running{}; // Immutable during chunk work; sampled from ECS in BeforeChunks.
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::match::VictorySystem>
{
    static constexpr std::string_view StableName = "games.generalszh.match.victory";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
