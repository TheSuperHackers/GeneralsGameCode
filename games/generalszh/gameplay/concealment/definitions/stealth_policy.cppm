module;
#include <cassert>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.concealment.definitions.stealth_policy;

export namespace generalszh::concealment
{
enum class StealthForbiddenCondition : std::uint8_t
{
    Moving = 1U,
    Firing = 2U
};

struct StealthPolicy
{
    bool innateStealth{};
    std::uint8_t forbiddenConditions{};
    friend constexpr bool operator==(const StealthPolicy &, const StealthPolicy &) noexcept = default;
};

class StealthPolicyId
{
public:
    constexpr StealthPolicyId() noexcept = default;
    constexpr bool operator==(const StealthPolicyId &) const noexcept = default;
private:
    explicit constexpr StealthPolicyId(std::uint32_t value) noexcept : value(value) {}
    std::uint32_t value{(std::numeric_limits<std::uint32_t>::max)()};
    friend class StealthPolicies;
};

struct CompiledStealthPolicy
{
    bool innateStealth{};
    std::uint8_t forbiddenConditions{};

    constexpr bool Forbids(StealthForbiddenCondition condition) const noexcept
    {
        return (forbiddenConditions & static_cast<std::uint8_t>(condition)) != 0U;
    }
};

class StealthPolicies
{
public:
    explicit StealthPolicies(std::span<const StealthPolicy> authored) : entries(authored.begin(), authored.end())
    {
        if (authored.size() >= (std::numeric_limits<std::uint32_t>::max)())
            throw std::length_error("Too many stealth policies");
        for (const auto &policy : entries)
            if ((policy.forbiddenConditions & ~static_cast<std::uint8_t>(
                static_cast<std::uint8_t>(StealthForbiddenCondition::Moving) |
                static_cast<std::uint8_t>(StealthForbiddenCondition::Firing))) != 0U)
                throw std::invalid_argument("Unsupported stealth forbidden condition");
    }

    StealthPolicyId Id(std::uint32_t index) const
    {
        if (index >= entries.size()) throw std::out_of_range("Stealth policy index");
        return StealthPolicyId{index};
    }

    StealthPolicyId Id(const StealthPolicy &policy) const
    {
        for (std::uint32_t index = 0; index != entries.size(); ++index)
            if (entries[index] == policy) return StealthPolicyId{index};
        throw std::out_of_range("Stealth policy is absent from catalog");
    }

    CompiledStealthPolicy Get(StealthPolicyId id) const
    {
        assert(id.value < entries.size());
        return {entries[id.value].innateStealth, entries[id.value].forbiddenConditions};
    }

private:
    std::vector<StealthPolicy> entries;
};
}
