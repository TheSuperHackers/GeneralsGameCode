module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.economy.transactions.account_batch;
export import games.generalszh.gameplay.economy.components.account_range;
export import engine.ecs.core.entity;
export import engine.ecs.core.component_registry;
export namespace generalszh::economy
{
enum class AccountOperation : std::uint8_t { Deposit, Withdraw };
struct AccountCommand
{
    AccountOperation operation{AccountOperation::Deposit};
    std::uint32_t amount{};
    bool playSound{true}, trackIncome{true};
};
struct AccountRequest { ecs::Entity account; AccountCommand command; };
enum class AccountResultStatus : std::uint8_t { InvalidTarget, Applied };
struct AccountResult
{
    ecs::Entity account;
    AccountOperation operation{};
    AccountResultStatus status{AccountResultStatus::InvalidTarget};
    std::uint32_t amount{}, balance{};
    bool playSound{}, recordedIncome{};
    bool operator==(const AccountResult &) const = default;
};

// Partition already canonically ordered input by account, preserving its local
// sequence. Producer/worker timing must never establish the input span's order.
// Metadata, command values and results have separate contiguous allocations.
class AccountBatch
{
public:
    explicit AccountBatch(std::size_t capacity) : capacity(capacity), indices(capacity),
        targets(capacity), commands(capacity), groupedResults(capacity), outputs(capacity) {}
    AccountBatch(const AccountBatch &) = delete;
    AccountBatch &operator=(const AccountBatch &) = delete;
    // Borrowed for one tick; sorting and preparation execute in the graph.
    void StageInputs(std::span<const AccountRequest> requests)
    {
        if (stage == Stage::Failed || stage == Stage::Prepared)
            throw std::logic_error("Account inputs cannot be staged in the current batch state");
        // Validate before replacing: rejected input preserves the prior staged span.
        if (requests.size() > capacity) throw std::length_error("Account request capacity exhausted");
        for (const auto &request : requests)
            if (request.command.operation != AccountOperation::Deposit && request.command.operation != AccountOperation::Withdraw)
                throw std::invalid_argument("Unknown account operation");
        staged = requests;
    }
    void PrepareStaged()
    {
        const auto input = staged;
        staged = {};
        Prepare(input);
    }
    void Prepare(std::span<const AccountRequest> requests)
    {
        if (stage != Stage::Empty) throw std::logic_error("Account batch is not empty");
        try
        {
            if (requests.size() > capacity) throw std::length_error("Account request capacity exhausted");
            count = requests.size();
            for (std::size_t i = 0; i != count; ++i)
            {
                const auto op = requests[i].command.operation;
                if (op != AccountOperation::Deposit && op != AccountOperation::Withdraw)
                    throw std::invalid_argument("Unknown account operation");
                indices[i] = i;
            }
            std::sort(indices.begin(), indices.begin() + count, [&](auto a, auto b) {
                return std::tie(requests[a].account.index, requests[a].account.generation, a)
                    < std::tie(requests[b].account.index, requests[b].account.generation, b);
            });
            for (std::size_t i = 0; i != count; ++i)
            {
                const auto &request = requests[indices[i]];
                targets[i] = request.account; commands[i] = request.command;
                groupedResults[i] = {request.account, request.command.operation};
            }
            stage = Stage::Prepared;
        }
        catch (...) { Fail(); throw; }
    }
    AccountRange RangeFor(ecs::Entity account) const noexcept
    {
        assert(stage == Stage::Prepared);
        const auto less = [](ecs::Entity a, ecs::Entity b) {
            return std::tie(a.index, a.generation) < std::tie(b.index, b.generation);
        };
        const auto first = std::lower_bound(targets.begin(), targets.begin() + count, account, less);
        const auto last = std::upper_bound(first, targets.begin() + count, account, less);
        return {static_cast<std::size_t>(first - targets.begin()), static_cast<std::size_t>(last - first)};
    }
    std::span<const AccountCommand> Commands(AccountRange range) const noexcept
    {
        assert(stage == Stage::Prepared && range.first <= count && range.count <= count - range.first);
        return std::span<const AccountCommand>(commands).subspan(range.first, range.count);
    }
    std::span<AccountResult> Results(AccountRange range) noexcept
    {
        assert(stage == Stage::Prepared && range.first <= count && range.count <= count - range.first);
        return std::span<AccountResult>(groupedResults).subspan(range.first, range.count);
    }
    void Publish()
    {
        if (stage != Stage::Prepared) throw std::logic_error("Account batch was not prepared");
        for (std::size_t i = 0; i != count; ++i) outputs[indices[i]] = groupedResults[i];
        stage = Stage::Published;
    }
    std::span<const AccountResult> Published() const
    {
        if (stage != Stage::Published) throw std::logic_error("Account results not published");
        return {outputs.data(), count};
    }
    bool IsPublished() const noexcept { return stage == Stage::Published; }
    void Release()
    {
        if (stage != Stage::Published) throw std::logic_error("Only published account results can be released");
        count = 0; stage = Stage::Empty;
    }
    void Fail() noexcept { count = 0; staged = {}; stage = Stage::Failed; }
private:
    enum class Stage { Empty, Prepared, Published, Failed };
    Stage stage{Stage::Empty};
    std::size_t capacity, count{};
    std::span<const AccountRequest> staged;
    std::vector<std::size_t> indices;
    std::vector<ecs::Entity> targets;
    std::vector<AccountCommand> commands;
    std::vector<AccountResult> groupedResults, outputs;
};
}
