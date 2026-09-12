module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.economy.systems.economy_system;
export import games.generalszh.gameplay.economy.systems.account_system;
export import games.generalszh.gameplay.economy.income.income_account;
export import games.generalszh.gameplay.economy.income.income_components;
export import games.generalszh.gameplay.harvesting.revenue.harvest_revenue;
export import games.generalszh.gameplay.bounty.inputs.cash_bounty_batch;
export import games.generalszh.gameplay.crates.inputs.crate_reward_batch;
export namespace generalszh
{
struct EconomyLimits
{
    std::size_t incomeSources{65536}, inputRequests{65536}, supplyDeliveries{}, bountyRequests{}, crateRequests{};
};
struct IncomeCalculation
{
    ecs::Entity source, account;
    economy::Payout payout;
    std::size_t transaction{(std::numeric_limits<std::size_t>::max)()};
};
struct EconomyStepResult
{
    std::span<const economy::AccountResult> accounts;
    std::span<const IncomeCalculation> income;
};
// Source-local economic behaviour followed by a canonical account-request
// reduction. AccountSystem settles shared balances in a dependent wave.
class EconomySystem
{
public:
    using IncomeRate = engine::gameplay::rts::economy::IncomeRate;
    using IncomeSchedule = engine::gameplay::rts::economy::IncomeSchedule;
    using IncomeEligibility = engine::gameplay::rts::economy::IncomeEligibility;
    using IncomeDispatch = engine::gameplay::rts::economy::IncomeDispatch;
    using IncomePulse = engine::gameplay::rts::economy::IncomePulse;
    using Query = ecs::Query<ecs::Read<economy::IncomeDefinition>, ecs::Read<economy::IncomeObservation>,
        ecs::Read<economy::IncomeAccount>, ecs::Read<IncomeRate>, ecs::Write<IncomeSchedule>,
        ecs::Write<IncomeEligibility>, ecs::Write<IncomeDispatch>, ecs::Write<IncomePulse>,
        ecs::Write<economy::CaptureRewardState>, ecs::Write<economy::IncomePayout>>;
    using Sources = ecs::Query<ecs::Read<economy::IncomeDefinition>, ecs::Read<economy::IncomeObservation>,
        ecs::Optional<economy::IncomeAccount>, ecs::Optional<IncomeRate>, ecs::Optional<IncomeSchedule>,
        ecs::Optional<IncomeEligibility>, ecs::Optional<IncomeDispatch>, ecs::Optional<IncomePulse>,
        ecs::Optional<economy::CaptureRewardState>, ecs::Optional<economy::IncomePayout>>;
    using AuxiliaryAccess=harvesting::HarvestRevenue::Access;
    static std::size_t Capacity(EconomyLimits limits)
    {
        if (limits.incomeSources > (std::numeric_limits<std::size_t>::max)() - limits.inputRequests)
            throw std::length_error("Economy capacity overflow");
        const auto subtotal=limits.incomeSources+limits.inputRequests;
        if(limits.supplyDeliveries>(std::numeric_limits<std::size_t>::max)()-subtotal)
            throw std::length_error("Economy capacity overflow");
        const auto withSupply = subtotal + limits.supplyDeliveries;
        if (limits.bountyRequests > (std::numeric_limits<std::size_t>::max)() - withSupply)
            throw std::length_error("Economy capacity overflow");
        const auto withBounty = withSupply + limits.bountyRequests;
        if (limits.crateRequests > (std::numeric_limits<std::size_t>::max)() - withBounty)
            throw std::length_error("Economy capacity overflow");
        return withBounty + limits.crateRequests;
    }
    EconomySystem(ecs::World &world, economy::AccountBatch &batch, EconomyLimits limits = {},
        harvesting::HarvestRevenue *supply=nullptr, bounty::CashBountyBatch *bounty=nullptr,
        crates::CrateRewardBatch *crateRewards=nullptr) :
        world(world), limits(limits), batch(batch),supply(supply),bounty(bounty),
        crateRewards(crateRewards), requestCapacity(Capacity(limits))
    {
        if (bounty && bounty->Capacity() > limits.bountyRequests)
            throw std::invalid_argument("Economy bounty capacity exceeds its reserved request bound");
        if (crateRewards && crateRewards->Capacity() > limits.crateRequests)
            throw std::invalid_argument("Economy crate capacity exceeds its reserved request bound");
        requests.reserve(requestCapacity);
        calculated.reserve(limits.incomeSources);
    }
    void Configure(engine::time::FixedStep, crates::CrateRewardBatch *configuredCrateRewards=nullptr)
    {
        if (sources) throw std::logic_error("Economy configuration is one-shot");
        if (configuredCrateRewards)
        {
            if (configuredCrateRewards->Capacity() > limits.crateRequests)
                throw std::invalid_argument("Economy crate capacity exceeds its reserved request bound");
            crateRewards = configuredCrateRewards;
        }
        sources = std::make_unique<Sources>(world);
    }
    void SetInputs(std::span<const economy::AccountRequest> value)
    {
        if (world.IsScheduledExecutionActive()) throw std::logic_error("Economy inputs require a joined boundary");
        if (value.size() > limits.inputRequests) throw std::length_error("Economy input capacity exhausted");
        for (const auto &request : value)
            if (request.command.operation != economy::AccountOperation::Deposit && request.command.operation != economy::AccountOperation::Withdraw)
                throw std::invalid_argument("Unknown account operation");
        input = value;
    }
    EconomyStepResult Results() const { return {batch.Published(), calculated}; }
    void BeforeChunks(Query &, ecs::SystemContext &)
    {
        if (!sources) throw std::logic_error("Economy is not configured");
        std::size_t sourceCount = 0;
        sources->ForEachChunk([&](auto chunk) {
            if (chunk.template Get<economy::IncomeAccount>().empty() || chunk.template Get<IncomeRate>().empty()
                || chunk.template Get<IncomeSchedule>().empty() || chunk.template Get<IncomeEligibility>().empty()
                || chunk.template Get<IncomeDispatch>().empty() || chunk.template Get<IncomePulse>().empty()
                || chunk.template Get<economy::CaptureRewardState>().empty() || chunk.template Get<economy::IncomePayout>().empty())
                throw std::invalid_argument("Incomplete modern income source");
            if (chunk.Count() > limits.incomeSources - sourceCount) throw std::length_error("Economy source capacity exhausted");
            sourceCount += chunk.Count();
            const auto definitions = chunk.template Get<economy::IncomeDefinition>();
            const auto observations = chunk.template Get<economy::IncomeObservation>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
                economy::ValidateIncomeAmount(definitions[row].base, observations[row].boost);
        });
    }
    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const
    {
        const auto definitions = chunk.Get<economy::IncomeDefinition>();
        const auto observations = chunk.Get<economy::IncomeObservation>();
        const auto rates = chunk.Get<IncomeRate>();
        auto schedules = chunk.Get<IncomeSchedule>();
        auto eligibility = chunk.Get<IncomeEligibility>();
        auto dispatch = chunk.Get<IncomeDispatch>();
        auto pulses = chunk.Get<IncomePulse>();
        auto rewards = chunk.Get<economy::CaptureRewardState>();
        auto payouts = chunk.Get<economy::IncomePayout>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto &observation = observations[row];
            dispatch[row].enabled = observation.active && observation.dispatchEnabled;
            eligibility[row].enabled = economy::CanAccrue(observation.neutral, observation.constructionComplete, definitions[row].base);
            pulses[row] = dispatch[row].enabled
                ? engine::gameplay::rts::economy::EvaluateIncome(schedules[row], rates[row], eligibility[row], context.Tick())
                : IncomePulse{};
            payouts[row] = {};
            if (!pulses[row].due) continue;
            economy::OnFirstIncomeDue(rewards[row]);
            if (pulses[row].quantity == 0) continue;
            payouts[row].value = economy::PlanValidatedIncome(definitions[row].base, observation.boost,
                definitions[row].actualMoney, observation.visible);
        }
    }
    void AfterChunks(Query &query, ecs::SystemContext &)
    {
        calculated.clear(); requests.assign(input.begin(), input.end()); input = {};
        query.ForEachChunk([&](auto chunk) {
            const auto recipients = chunk.template Get<economy::IncomeAccount>();
            const auto payouts = chunk.template Get<economy::IncomePayout>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
                if (payouts[row].value.pay || payouts[row].value.show)
                    calculated.push_back({chunk.Entities()[row], recipients[row].entity, payouts[row].value});
        });
        std::sort(calculated.begin(), calculated.end(), [](const auto &a, const auto &b) {
            return std::tie(a.source.index, a.source.generation) < std::tie(b.source.index, b.source.generation);
        });
        for (auto &output : calculated)
            if (output.payout.pay)
            {
                output.transaction = requests.size();
                AppendRequest({output.account, {economy::AccountOperation::Deposit, output.payout.money}});
            }
        if(supply)
        {
            const auto deliveries=supply->Collect();
            if(deliveries.size()>limits.supplyDeliveries) throw std::length_error("Economy supply delivery capacity exhausted");
            AppendBounded(deliveries);
        }
        // Canonical merge position: external input, periodic income, supply
        // delivery, cash-bounty settlement, then crate rewards. AccountSystem
        // receives one deterministic request span and remains the sole
        // balance/history writer. Published producer batches are consumed only
        // after their requests have been staged successfully.
        if (bounty)
        {
            const auto bountyRequests = bounty->Requests();
            if (bountyRequests.size() > limits.bountyRequests)
                throw std::length_error("Economy bounty request capacity exhausted");
            AppendBounded(bountyRequests);
        }
        if (crateRewards)
        {
            if (!crateRewards->IsPublished())
                throw std::logic_error("Economy crate rewards are not published");
            const auto crateRequests = crateRewards->Requests();
            if (crateRequests.size() > limits.crateRequests)
                throw std::length_error("Economy crate request capacity exhausted");
            AppendBounded(crateRequests);
        }
        batch.StageInputs(requests);
        if (bounty) bounty->Consume();
        if (crateRewards) crateRewards->Release();
    }
private:
    void AppendRequest(const economy::AccountRequest value)
    {
        if (requests.size() >= requestCapacity)
            throw std::length_error("Economy combined request capacity exhausted");
        requests.push_back(value);
    }

    void AppendBounded(const std::span<const economy::AccountRequest> values)
    {
        if (requests.size() > requestCapacity || values.size() > requestCapacity - requests.size())
            throw std::length_error("Economy combined request capacity exhausted");
        requests.insert(requests.end(), values.begin(), values.end());
    }

    ecs::World &world;
    EconomyLimits limits;
    economy::AccountBatch &batch;
    harvesting::HarvestRevenue *supply;
    bounty::CashBountyBatch *bounty;
    crates::CrateRewardBatch *crateRewards;
    std::size_t requestCapacity;
    std::unique_ptr<Sources> sources;
    std::span<const economy::AccountRequest> input;
    std::vector<economy::AccountRequest> requests;
    std::vector<IncomeCalculation> calculated;
};
inline void RegisterEconomyComponents(ecs::World &world)
{
    using namespace engine::gameplay::rts::economy;
    economy::RegisterAccountComponents(world);
    world.RegisterComponent<economy::IncomeAccount>(); world.RegisterComponent<economy::IncomeDefinition>();
    world.RegisterComponent<economy::IncomeObservation>(); world.RegisterComponent<economy::IncomePayout>();
    world.RegisterComponent<economy::CaptureRewardState>(); world.RegisterComponent<IncomeRate>();
    world.RegisterComponent<IncomeSchedule>(); world.RegisterComponent<IncomeEligibility>();
    world.RegisterComponent<IncomeDispatch>(); world.RegisterComponent<IncomePulse>();
    world.RegisterComponent<engine::gameplay::rts::harvesting::HarvestDelivery>();
    world.RegisterComponent<production::ProducedUnit>(); world.RegisterComponent<harvesting::SupplyDropoffOwner>();
    world.RegisterComponent<engine::gameplay::rts::harvesting::SupplyDropoff>();
    world.RegisterComponent<engine::gameplay::combat::Health>(); world.RegisterComponent<engine::gameplay::combat::LifeState>();
}
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::EconomySystem>
{
    static constexpr std::string_view StableName = "games.generalszh.economy.income";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<generalszh::economy::AccountSystem>;
    using After = SystemTypeList<>;
};
}
