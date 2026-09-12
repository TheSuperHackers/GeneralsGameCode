module;
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
export module games.generalszh.gameplay.economy.systems.account_system;
export import games.generalszh.gameplay.economy.transactions.account_batch;
export import games.generalszh.gameplay.economy.income_history;
export import engine.gameplay.rts.economy.components.resource_balance;
export import engine.ecs.system.system;
export namespace generalszh::economy
{
using engine::gameplay::rts::economy::ResourceBalance;
struct AccountSystem
{
    explicit AccountSystem(AccountBatch &batch) : batch(batch) {}
    // Write range denotes exclusive ownership of the corresponding external
    // result slice, as well as disjoint component rows. No shared counter.
    using Query = ecs::Query<ecs::Write<ResourceBalance>, ecs::Write<IncomeHistory>, ecs::Write<AccountRange>>;
    void SetInputs(std::span<const AccountRequest> input) { batch.StageInputs(input); }
    std::span<const AccountResult> Results() const { return batch.Published(); }
    // One account behaviour, with a scheduler-owned lifecycle around its
    // disjoint chunk work. No separate Prepare/Publish systems or child owners.
    void BeforeChunks(Query &query, ecs::SystemContext &)
    {
        try
        {
            if (batch.IsPublished()) batch.Release();
            batch.PrepareStaged();
            query.ForEachChunk([&](auto chunk) {
                const auto balances = chunk.template Get<ResourceBalance>();
                const auto histories = chunk.template Get<IncomeHistory>();
                auto ranges = chunk.template Get<AccountRange>();
                for (std::size_t row = 0; row != chunk.Count(); ++row)
                {
                    if (balances[row].quantity > UINT32_MAX || histories[row].current >= histories[row].buckets.size())
                        throw std::invalid_argument("Invalid Zero Hour account setup/restore state");
                    ranges[row] = batch.RangeFor(chunk.Entities()[row]);
                }
            });
        }
        catch (...) { batch.Fail(); throw; }
    }
    void AfterChunks(Query &, ecs::SystemContext &) { batch.Publish(); }
    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const noexcept
    {
        auto balances = chunk.Get<ResourceBalance>();
        auto histories = chunk.Get<IncomeHistory>();
        const auto ranges = chunk.Get<AccountRange>();
        const auto bucket = static_cast<std::uint32_t>((context.Tick() / context.Time().Step().TicksPerSecond()) % 60);
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            auto &balance = balances[row]; auto &history = histories[row];
            assert(balance.quantity <= UINT32_MAX);
            assert(history.current < history.buckets.size());
            // Explicit tick boundary owns ageing, never a UI getter.
            if (history.current != bucket)
            {
                history.total -= history.buckets[bucket];
                history.buckets[bucket] = 0; history.current = bucket;
            }
            const auto commands = batch.Commands(ranges[row]);
            auto results = batch.Results(ranges[row]);
            for (std::size_t i = 0; i != commands.size(); ++i)
            {
                const auto &command = commands[i]; auto &result = results[i];
                auto amount = command.amount;
                if (command.operation == AccountOperation::Deposit)
                {
                    balance.quantity = static_cast<std::uint32_t>(balance.quantity + amount);
                    if (amount && command.trackIncome)
                    {
                        history.buckets[bucket] += amount; history.total += amount;
                        result.recordedIncome = true;
                    }
                }
                else
                    amount = static_cast<std::uint32_t>(engine::gameplay::rts::economy::DebitUpTo(balance, amount));
                result.status = AccountResultStatus::Applied;
                result.amount = amount; result.balance = static_cast<std::uint32_t>(balance.quantity);
                result.playSound = command.playSound && amount != 0;
            }
        }
    }
private:
    AccountBatch &batch;
};
inline void RegisterAccountComponents(ecs::World &world)
{
    world.RegisterComponent<ResourceBalance>(); world.RegisterComponent<IncomeHistory>(); world.RegisterComponent<AccountRange>();
}
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::economy::AccountSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.economy.account_system";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
