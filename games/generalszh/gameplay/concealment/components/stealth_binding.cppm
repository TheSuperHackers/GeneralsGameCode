module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.concealment.components.stealth_binding;
export import engine.ecs.core.component_registry;
export import games.generalszh.gameplay.concealment.definitions.stealth_policy;

export namespace generalszh::concealment
{
struct StealthPolicyBinding { StealthPolicyId policy{}; };
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::concealment::StealthPolicyBinding>
{
    static constexpr std::string_view StableName="games.generalszh.concealment.stealth_policy_binding";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
