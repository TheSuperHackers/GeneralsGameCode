module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.rts.production.definitions.prerequisite_definition;
export namespace engine::gameplay::rts::production
{
// One authored Object prerequisite line. Alternatives are OR'd within the
// group; groups in a PrerequisiteDefinition are AND'd. The constructor copies
// and canonicalizes the input so runtime consumers cannot mutate the compiled
// definition.
struct PrerequisiteGroup
{
    std::vector<std::uint32_t> alternatives;
};

class PrerequisiteDefinition final
{
public:
    struct Group
    {
        std::uint32_t first{};
        std::uint32_t count{};
    };

    PrerequisiteDefinition() = default;
    explicit PrerequisiteDefinition(std::span<const PrerequisiteGroup> source)
    {
        std::size_t total = 0;
        for (const auto &group : source)
        {
            if (group.alternatives.empty() || group.alternatives.size() > (std::numeric_limits<std::uint32_t>::max)()
                || total > (std::numeric_limits<std::uint32_t>::max)() - group.alternatives.size())
                throw std::invalid_argument("Invalid Object prerequisite group");
            total += group.alternatives.size();
        }
        alternatives.reserve(total); groups.reserve(source.size());
        for (const auto &sourceGroup : source)
        {
            auto values = sourceGroup.alternatives;
            std::sort(values.begin(), values.end());
            values.erase(std::unique(values.begin(), values.end()), values.end());
            if (values.empty() || alternatives.size() > (std::numeric_limits<std::uint32_t>::max)() - values.size())
                throw std::invalid_argument("Invalid Object prerequisite alternatives");
            const auto first = static_cast<std::uint32_t>(alternatives.size());
            alternatives.insert(alternatives.end(), values.begin(), values.end());
            groups.push_back({first, static_cast<std::uint32_t>(values.size())});
        }
    }

    std::span<const Group> Groups() const noexcept { return groups; }
    std::span<const std::uint32_t> Alternatives() const noexcept { return alternatives; }
    bool Empty() const noexcept { return groups.empty(); }

private:
    std::vector<std::uint32_t> alternatives;
    std::vector<Group> groups;
};
}
