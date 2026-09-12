module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.selling.components.sale_state;
export import engine.ecs.core.component_registry;
export namespace generalszh::selling
{
enum class SalePhase : std::uint8_t { Idle, Selling, Completed, CancelledDeath };
struct SaleState
{
    std::uint64_t deadline{}, acceptedTick{}, requestOrdinal{};
    std::uint32_t refund{};
    SalePhase phase{SalePhase::Idle};
};
// Root enrollment explicitly enables supported structures. Dynamic script or
// disabled-state policy may revoke eligibility before acceptance; it cannot
// cancel a sale already committed. No sale-cancellation command is provided.
struct SaleEligibility { bool allowed{false}; };
}
export namespace ecs
{
template<> struct ComponentTraits<generalszh::selling::SaleState>
{
    static constexpr std::string_view StableName="games.generalszh.selling.state";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<generalszh::selling::SaleEligibility>
{
    static constexpr std::string_view StableName="games.generalszh.selling.eligibility";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
