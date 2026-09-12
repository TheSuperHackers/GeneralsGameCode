module;
#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module engine.gameplay.rts.construction.observations.builder_frame;
export import engine.gameplay.rts.construction.components.construction_components;
export namespace engine::gameplay::rts::construction
{
// Transient joined observations; ECS assignments/positions remain authoritative.
class BuilderFrame
{
public:
    struct Row { ecs::Entity builder{}, site{}; navigation::Cell cell{}; bool alive{}; };
    explicit BuilderFrame(std::size_t capacity) : capacity(capacity) { rows.reserve(capacity); }
    void Clear() noexcept { rows.clear(); }
    void Append(Row row)
    {
        if (rows.size() == capacity) throw std::length_error("Construction builder frame exhausted");
        rows.push_back(row);
    }
    void Finalize()
    {
        std::sort(rows.begin(), rows.end(), [](const auto &a, const auto &b) {
            return std::tie(a.builder.index,a.builder.generation)<std::tie(b.builder.index,b.builder.generation);
        });
    }
    const Row *Find(ecs::Entity builder) const noexcept
    {
        const auto it=std::lower_bound(rows.begin(),rows.end(),builder,[](const auto &a,auto b) {
            return std::tie(a.builder.index,a.builder.generation)<std::tie(b.index,b.generation);
        });
        return it!=rows.end() && it->builder==builder ? &*it : nullptr;
    }
private:
    std::size_t capacity;
    std::vector<Row> rows;
};

}
