module;
#include <cstddef>
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.production.components.build_admission;
export import engine.ecs.core.component_registry;
export namespace generalszh::production
{
struct BuildAdmission { std::size_t receipt{}; };
}
export namespace ecs
{
template<> struct ComponentTraits<generalszh::production::BuildAdmission>
{
    static constexpr std::string_view StableName = "games.generalszh.production.admission";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
