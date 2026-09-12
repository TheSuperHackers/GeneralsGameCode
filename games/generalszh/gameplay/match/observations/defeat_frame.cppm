module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.match.observations.defeat_frame;
export import engine.ecs.core.entity;
export namespace generalszh::match
{
// Immutable during jobs; rebuilt from authoritative Contender columns at the
// elimination barrier. Dense generation-checked account indexing, no hot maps.
class DefeatFrame
{
public:
    explicit DefeatFrame(std::size_t capacity) : generations(capacity), defeated(capacity) {}
    std::size_t Capacity() const noexcept { return generations.size(); }
    void Clear() noexcept { std::fill(generations.begin(), generations.end(), 0); }
    void Set(ecs::Entity account, bool value)
    {
        if (account.index >= generations.size()) throw std::length_error("Match account index capacity exhausted");
        generations[account.index] = account.generation; defeated[account.index] = value;
    }
    bool IsDefeated(ecs::Entity account) const noexcept
    {
        return account.IsValid() && account.index < generations.size() &&
            generations[account.index] == account.generation && defeated[account.index];
    }
private:
    std::vector<std::uint32_t> generations;
    std::vector<std::uint8_t> defeated;
};
}
