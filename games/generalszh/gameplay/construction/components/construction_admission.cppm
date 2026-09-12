module;
#include <cstddef>
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.construction.components.construction_admission;
export import engine.ecs.core.component_registry;
export namespace generalszh::construction
{
struct ConstructionAdmission { std::size_t receipt{}; };
}
export namespace ecs
{
template<> struct ComponentTraits<generalszh::construction::ConstructionAdmission>
{
    static constexpr std::string_view StableName="games.generalszh.construction.admission";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Transient;
};
}
