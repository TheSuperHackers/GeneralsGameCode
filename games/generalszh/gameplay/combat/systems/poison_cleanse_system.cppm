module;
#include <string_view>
export module games.generalszh.gameplay.combat.systems.poison_cleanse_system;
export import engine.gameplay.combat.systems.healing_system;
export import engine.gameplay.combat.systems.periodic_damage_system;
export namespace generalszh::combat
{
struct PoisonCleanseSystem
{
    using Query=ecs::Query<ecs::Read<engine::gameplay::combat::LifeState>,ecs::Read<engine::gameplay::combat::HealingResult>,ecs::Write<engine::gameplay::combat::PeriodicDamage>>;
    void Execute(Query::Chunk chunk,ecs::SystemContext &) const noexcept
    {
        using namespace engine::gameplay::combat;
        const auto lives=chunk.Get<LifeState>();const auto healing=chunk.Get<HealingResult>();auto poison=chunk.Get<PeriodicDamage>();
        for(std::size_t row=0;row!=chunk.Count();++row) if(!lives[row].alive||healing[row].applied) poison[row]={};
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::combat::PoisonCleanseSystem>
{
    static constexpr std::string_view StableName="games.generalszh.combat.poison_cleanse";
    static constexpr SystemPhase Phase=SystemPhase::PostSimulation;
    using Before=SystemTypeList<>;using After=SystemTypeList<>;
};
}
