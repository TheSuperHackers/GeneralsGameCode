module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.progression.components.veterancy_award_binding;
export import engine.ecs.core.component_registry;
export namespace generalszh::progression
{
// Authored binding for entities that participate in Zero Hour veterancy
// awards. Presence is independent of ProgressionEligibility: a regular,
// non-trainable victim can still carry an award value. The awardable bit is
// only the unit-side non-ignored/award-value eligibility decision. Playable
// membership belongs to the owning account's match state, and recipient
// acceptance belongs to the source's progression state; ownership and
// completion remain authoritative in their ECS components.
struct VeterancyAwardBinding
{
    std::uint32_t definition{};
    bool awardable{};
};
}
export namespace ecs
{
template<> struct ComponentTraits<generalszh::progression::VeterancyAwardBinding>
{
    static constexpr std::string_view StableName="games.generalszh.progression.veterancy_award_binding";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
