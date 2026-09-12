module;
#include <cstdint>
#include <string_view>

export module engine.gameplay.lifetime.components.expiration_eligibility;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::lifetime
{
// Derived input at an explicit simulation boundary. Disabling postpones
// expiration dispatch; it does not pause time or shift the stored deadline.
struct ExpirationEligibility
{
	bool enabled{true};
};
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::lifetime::ExpirationEligibility>
{
	static constexpr std::string_view StableName = "engine.gameplay.lifetime.expiration_eligibility";
	static constexpr std::uint32_t Version = 1;
	// Rebuild from authoritative eligibility inputs before running expiration.
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
