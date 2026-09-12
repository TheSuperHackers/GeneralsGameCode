module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.demolition.inputs.demolition_trap_inputs;
export import engine.ecs.core.entity;
export import games.generalszh.gameplay.demolition.components.demolition_trap;

export namespace generalszh::demolition
{
enum class DemolitionTrapCommand : std::uint8_t
{
    Detonate,
    SetProximityMode,
    SetManualMode
};

struct DemolitionTrapInput
{
    ecs::Entity trap{};
    DemolitionTrapCommand command{DemolitionTrapCommand::Detonate};
};

class DemolitionTrapInputBatch
{
public:
    explicit DemolitionTrapInputBatch(std::size_t capacity) : capacity_(capacity)
    { inputs_.reserve(capacity); normalized_.reserve(capacity); }

    void SetInputs(std::span<const DemolitionTrapInput> inputs)
    {
        if (inputs.size()>capacity_) throw std::length_error("Demolition trap input capacity exhausted");
        inputs_.assign(inputs.begin(),inputs.end());
        normalized_.clear();
        for (const auto &input:inputs_)
        {
            if (input.command!=DemolitionTrapCommand::Detonate &&
                input.command!=DemolitionTrapCommand::SetProximityMode &&
                input.command!=DemolitionTrapCommand::SetManualMode)
                throw std::invalid_argument("Unknown demolition trap command");
            const auto found=std::find_if(normalized_.begin(),normalized_.end(),[&](const auto &request) {
                return request.trap==input.trap;
            });
            const bool newRequest=found==normalized_.end();
            if (newRequest)
                normalized_.push_back({input.trap,false,false,DemolitionDetonationMode::Proximity});
            auto &request=newRequest?normalized_.back():*found;
            if (input.command==DemolitionTrapCommand::Detonate) request.detonate=true;
            else { request.modeSet=true; request.mode=input.command==DemolitionTrapCommand::SetManualMode
                ?DemolitionDetonationMode::Manual:DemolitionDetonationMode::Proximity; }
        }
        std::sort(normalized_.begin(),normalized_.end(),[](const auto &left,const auto &right) {
            return std::tie(left.trap.index,left.trap.generation)<std::tie(right.trap.index,right.trap.generation);
        });
    }

    [[nodiscard]] bool Requested(ecs::Entity trap) const noexcept
    {
        const auto *request=Find(trap);
        return request && request->detonate;
    }

    [[nodiscard]] bool ModeCommand(ecs::Entity trap,DemolitionDetonationMode &mode) const noexcept
    {
        const auto *request=Find(trap);
        if (!request || !request->modeSet) return false;
        mode=request->mode; return true;
    }

    void Clear() noexcept
    {
        inputs_.clear();
        normalized_.clear();
    }

private:
    struct Request
    {
        ecs::Entity trap{};
        bool detonate{};
        bool modeSet{};
        DemolitionDetonationMode mode{DemolitionDetonationMode::Proximity};
    };
    const Request *Find(ecs::Entity trap) const noexcept
    {
        const auto found=std::lower_bound(normalized_.begin(),normalized_.end(),trap,[](const auto &request,const auto entity) {
            return std::tie(request.trap.index,request.trap.generation)<std::tie(entity.index,entity.generation);
        });
        return found==normalized_.end() || found->trap!=trap ? nullptr : &*found;
    }
    std::size_t capacity_{};
    std::vector<DemolitionTrapInput> inputs_;
    std::vector<Request> normalized_;
};
}
