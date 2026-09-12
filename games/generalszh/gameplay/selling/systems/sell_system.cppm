module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.selling.systems.sell_system;
export import games.generalszh.gameplay.selling.definitions.sell_definition;
export import games.generalszh.gameplay.selling.components.sale_state;
export import games.generalszh.gameplay.selling.inputs.sell_batch;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import games.generalszh.gameplay.economy.transactions.account_transactions;

export namespace generalszh::selling
{
class SellSystem
{
    using Structure=construction::Structure;
    using Health=engine::gameplay::combat::Health;
    using Life=engine::gameplay::combat::LifeState;
    using Producer=production::Producer;
    using Queue=engine::gameplay::rts::production::ProductionQueue;
    using Weapon=engine::gameplay::combat::WeaponDefinition;
    using Speed=engine::gameplay::navigation::MoveSpeed;
    using Dropoff=engine::gameplay::rts::harvesting::SupplyDropoff;
    using Site=engine::gameplay::rts::construction::ConstructionSite;
    using Balance=engine::gameplay::rts::economy::ResourceBalance;
    struct Completion
    {
        SellResult result{};
        std::uint64_t acceptedTick{},ordinal{};
        bool ready{};
    };
public:
    using Query=ecs::Query<ecs::Read<Structure>,ecs::Read<Health>,ecs::Read<Life>,
        ecs::Read<SaleEligibility>,ecs::Write<SaleState>,ecs::OptionalWrite<Producer>,
        ecs::Optional<Queue>,ecs::Optional<Weapon>,ecs::Optional<Speed>,ecs::Optional<Dropoff>,ecs::Optional<Site>>;
    using AuxiliaryAccess=ecs::Query<ecs::Write<Balance>>;
    SellSystem(const SellCatalog &catalog,SellBatch &batch,std::size_t structureCapacity=65536)
        : catalog(catalog),batch(batch),completions(structureCapacity)
    {
        if (batch.ResultCapacity()<structureCapacity) throw std::invalid_argument("Sale results must cover structure capacity");
        offsets.reserve(structureCapacity); ready.reserve(structureCapacity);
    }
    void BeforeChunks(Query &query,ecs::SystemContext &context)
    {
        assert(context.Time().Step()==catalog.Step());
        if (lastTick && context.Tick()<=*lastTick) throw std::invalid_argument("Sale ticks must advance");
        lastTick=context.Tick();
        offsets.clear(); rowCount=0; ready.clear();
        query.ForEachPreparedChunk([&](Query::Chunk chunk) {
            if (chunk.Count()>completions.size()-rowCount) throw std::length_error("Sale structure capacity exhausted");
            offsets.push_back(rowCount); rowCount+=chunk.Count();
        });
        auto &world=context.GetWorld();
        const auto inputs=batch.BeginTick();
        for (std::size_t index=0;index<inputs.size();++index)
        {
            const auto &input=inputs[index];
            auto status=SellAcceptance::InvalidTarget;
            const auto *structure=world.Get<Structure>(input.structure);
            const auto *health=world.Get<Health>(input.structure);
            const auto *life=world.Get<Life>(input.structure);
            const auto *eligibility=world.Get<SaleEligibility>(input.structure);
            auto *state=world.Get<SaleState>(input.structure);
            if (structure && health && life && eligibility && state)
            {
                const auto *producer=world.Get<Producer>(input.structure);
                if (!structure->complete || structure->supplyDropoff || world.Get<Site>(input.structure)
                    || world.Get<Weapon>(input.structure) || world.Get<Speed>(input.structure) || world.Get<Dropoff>(input.structure)
                    || (structure->queueLimit && (!producer || !world.Get<Queue>(input.structure)))
                    || (producer && (!world.Get<Queue>(input.structure) || producer->account!=structure->account)))
                    status=SellAcceptance::UnsupportedSignature;
                else if (input.account!=structure->account) status=SellAcceptance::WrongOwner;
                else if (!life->alive || !health->current) status=SellAcceptance::Dead;
                else if (state->phase!=SalePhase::Idle) status=SellAcceptance::AlreadySelling;
                else if (!eligibility->allowed) status=SellAcceptance::Ineligible;
                else if (!world.Get<Balance>(structure->account)) status=SellAcceptance::MissingAccount;
                else if (const auto *definition=catalog.Find(structure->definition))
                {
                    const auto deadline=SaleDeadline(context.Tick(),definition->delayTicks);
                    *state={deadline,context.Tick(),index,definition->refund,SalePhase::Selling};
                    status=SellAcceptance::Accepted;
                }
                else status=SellAcceptance::UnknownDefinition;
            }
            batch.Accept(index,{input.structure,status});
        }
    }
    void Execute(Query::Chunk chunk,ecs::SystemContext &context) noexcept
    {
        const auto structures=chunk.Get<Structure>(); const auto health=chunk.Get<Health>();
        const auto life=chunk.Get<Life>(); auto states=chunk.Get<SaleState>(); auto producers=chunk.Get<Producer>();
        assert(context.ChunkOrder()<offsets.size());
        const auto offset=offsets[context.ChunkOrder()];
        for (std::size_t row=0;row<chunk.Count();++row)
        {
            auto &output=completions[offset+row]; output.ready=false;
            auto &state=states[row];
            if (state.phase!=SalePhase::Selling) continue;
            // ProductionAdmissionSystem runs after us and performs its existing
            // immediate queue cancellation/refund. Life stays true until damage
            // kills the structure: selling is not invulnerability or death.
            if (!producers.empty()) producers[row].active=false;
            if (!life[row].alive || !health[row].current)
            {
                // Deliberate modern difference: reference sell-list update only
                // checks object existence. A logically dead, retained corpse
                // receives no sale payout even before its eventual destruction.
                state.phase=SalePhase::CancelledDeath;
                output={{chunk.Entities()[row],structures[row].account,context.Tick(),0,SellOutcome::CancelledDeath},
                    state.acceptedTick,state.requestOrdinal,true};
            }
            else if (context.Tick()>=state.deadline)
            {
                state.phase=SalePhase::Completed;
                output={{chunk.Entities()[row],structures[row].account,context.Tick(),state.refund,SellOutcome::Sold},
                    state.acceptedTick,state.requestOrdinal,true};
            }
        }
    }
    void AfterChunks(Query &,ecs::SystemContext &context)
    {
        for (std::size_t i=0;i<rowCount;++i) if (completions[i].ready) ready.push_back(i);
        // Reference sellObject pushes front: later accepted sales are visited
        // first. Entity identity breaks ties without worker/completion timing.
        std::sort(ready.begin(),ready.end(),[&](auto a,auto b) {
            const auto &x=completions[a]; const auto &y=completions[b];
            return std::tuple{x.acceptedTick,x.ordinal,x.result.structure.index,x.result.structure.generation}>
                std::tuple{y.acceptedTick,y.ordinal,y.result.structure.index,y.result.structure.generation};
        });
        for (const auto index:ready)
        {
            auto result=completions[index].result;
            if (result.outcome==SellOutcome::Sold)
            {
                auto *balance=context.GetWorld().Get<Balance>(result.account);
                if (!balance) { result.refunded=0; result.outcome=SellOutcome::MissingAccount; }
                else if (result.refunded)
                {
                    // Same uint32 modular addition as ApplyDeposit(...,false).
                    // No fabricated IncomeHistory and no income mutation.
                    const auto current=economy::BalanceValue(*balance);
                    balance->quantity=static_cast<std::uint32_t>(static_cast<std::uint64_t>(current)+result.refunded);
                }
                context.Commands().Destroy(result.structure);
            }
            batch.AddResult(result);
        }
        batch.Publish();
    }
private:
    const SellCatalog &catalog;
    SellBatch &batch;
    std::vector<Completion> completions;
    std::vector<std::size_t> offsets,ready;
    std::size_t rowCount{};
    std::optional<std::uint64_t> lastTick;
};
// Non-owning startup registration. Root registers the system and semantic edges.
inline void RegisterSellComponents(ecs::World &world)
{
    world.RegisterComponent<SaleState>(); world.RegisterComponent<SaleEligibility>();
    world.RegisterComponent<construction::Structure>();
    world.RegisterComponent<engine::gameplay::combat::Health>(); world.RegisterComponent<engine::gameplay::combat::LifeState>();
    world.RegisterComponent<production::Producer>(); world.RegisterComponent<engine::gameplay::rts::production::ProductionQueue>();
    world.RegisterComponent<engine::gameplay::combat::WeaponDefinition>(); world.RegisterComponent<engine::gameplay::navigation::MoveSpeed>();
    world.RegisterComponent<engine::gameplay::rts::harvesting::SupplyDropoff>();
    world.RegisterComponent<engine::gameplay::rts::construction::ConstructionSite>();
    world.RegisterComponent<engine::gameplay::rts::economy::ResourceBalance>();
}
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::selling::SellSystem>
{
    static constexpr std::string_view StableName="games.generalszh.selling.sell";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<>; using After=SystemTypeList<>;
};
}
