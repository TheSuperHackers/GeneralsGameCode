module;
#include <array>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <vector>
module games.generalszh.hosts.headless.headless_match;

import games.generalszh.gameplay.match.components.match_state;
import games.generalszh.composition.game_session;
import games.generalszh.gameplay.production.components.production_state;
import games.generalszh.gameplay.production.doors.components.production_exit;
import engine.gameplay.combat.components.health;
import engine.gameplay.combat.components.weapon;
import engine.gameplay.navigation.components.movement;
import engine.gameplay.rts.economy.components.resource_balance;
namespace generalszh::headless
{
// Small authored demonstration, not an original-map loader or substitute game
// implementation. All actions use the production GameSession composition.
MatchReport RunMatch(std::size_t workers, std::uint64_t tickLimit)
{
    using namespace std::chrono_literals;
    using namespace engine::gameplay::combat;
    using namespace engine::gameplay::navigation;
    if (!workers || workers > 64 || !tickLimit) throw std::invalid_argument("Headless host requires 1..64 workers and a positive tick limit");
    const std::array<std::uint8_t,9> terrain{1,1,1,1,1,1,1,1,1}; NavigationGrid grid(9,1,terrain);
    const std::array definitions{
        production::BuildDefinition{1,100,0ms,production::EntryKind::Unit,1,100,20,WeaponConfig{100,2,3,100ms,300ms,50ms,true},{true,true}},
        production::BuildDefinition{2,100,0ms,production::EntryKind::Unit,1,100,20}};
    ecs::World world; GameSession::RegisterComponents(world); world.FinalizeComponents();
    engine::jobs::JobSystem jobs({workers});
    GameSession game(world,jobs,definitions,grid,4096); const engine::time::FixedStep step{20}; game.Finalize(step);
    const auto red = game.CreateAccount(200), blue = game.CreateAccount(200);
    const auto redBase = game.CreateProducer(red,9,300,0), blueBase = game.CreateProducer(blue,9,300,8);
    const std::array players{red,blue}; game.BeginMatch(players);
    const std::array build{BuildInput{redBase,1},BuildInput{blueBase,2}};
    game.Execute({0,step},{{},{},build});
    ecs::Entity attacker{};
    ecs::Query<ecs::Read<production::ProducedUnit>> units(world);
    units.ForEachChunk([&](auto chunk) {
        const auto owners = chunk.template Get<production::ProducedUnit>();
        for (std::size_t row = 0; row != chunk.Count(); ++row) if (owners[row].account == red) attacker = chunk.Entities()[row];
    });
    if (!attacker.IsValid()) throw std::runtime_error("Headless scenario failed to produce its attacker");
    const std::array moves{MoveInput{attacker,8}};
    for (std::uint64_t tick = 1; tick < tickLimit && game.Outcome().status == MatchStatus::Running; ++tick)
        game.Execute({tick,step},tick == 1 ? GameInputs{{},{},{},moves} : GameInputs{});
    MatchReport report{game.Outcome(),attacker,blueBase,world.Get<Health>(blueBase)->current};
    const std::array actors{redBase,blueBase,attacker};
    for (const auto actor : actors)
        report.state.insert(report.state.end(),{actor.index,actor.generation,world.Get<Health>(actor)->current,
            world.Get<LifeState>(actor)->alive,world.Get<GridPosition>(actor)->cell});
    report.state.insert(report.state.end(), {world.Get<WeaponState>(attacker)->ammo,
        world.Get<engine::gameplay::rts::economy::ResourceBalance>(red)->quantity,
        world.Get<engine::gameplay::rts::economy::ResourceBalance>(blue)->quantity});
    ecs::Query<ecs::Read<production::ProducedUnit>,ecs::Read<LifeState>> living(world);
    living.ForEachChunk([&](auto chunk) { for (const auto life : chunk.template Get<LifeState>()) report.survivingUnits += life.alive; });
    return report;
}
}


