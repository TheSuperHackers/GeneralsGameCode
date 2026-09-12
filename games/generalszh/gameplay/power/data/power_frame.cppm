module;
#include <cstddef>
#include <cstdint>
#include <vector>
#include <stdexcept>
export module games.generalszh.gameplay.power.data.power_frame;
export import engine.ecs.core.entity;
export import games.generalszh.gameplay.power.components.power_state;
export import games.generalszh.gameplay.power.definitions.power_config;
export namespace generalszh::power
{
// Derived, generation-checked publication. ECS account components own the state.
class PowerFrame
{
    struct Entry { ecs::Entity account{}; PowerState state{}; };
    std::vector<Entry> entries;
public:
    explicit PowerFrame(std::size_t entityCapacity = 65536) : entries(entityCapacity) {}
    std::size_t Capacity() const noexcept { return entries.size(); }
    const PowerState *Find(ecs::Entity account) const noexcept
    {
        if (!account.IsValid() || account.index >= entries.size()) return nullptr;
        const auto &entry = entries[account.index];
        return entry.account == account ? &entry.state : nullptr;
    }
    // PowerSystem publishes only at its joined boundary; consumers borrow const.
    void Clear() noexcept { for (auto &entry : entries) entry = {}; }
    void Publish(ecs::Entity account, PowerState state)
    {
        if (!account.IsValid() || account.index >= entries.size())
            throw std::length_error("Power snapshot entity capacity exhausted");
        entries[account.index] = {account, state};
    }
};
}
