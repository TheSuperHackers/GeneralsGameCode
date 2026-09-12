module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.lifetime.components.lifetime_dispatch;
export import engine.ecs.core.component_registry;

export namespace generalszh::lifetime
{
enum class LifetimeAction : std::uint8_t { Kill, Destroy };
struct LifetimeTarget
{
	std::uint32_t object{}; // Game adapter identity, never a process pointer.
	LifetimeAction action{LifetimeAction::Kill};
};
struct LifetimeDispatch
{
	std::uint64_t revision{};
	bool ready{false};
};
}
export namespace ecs
{
template<> struct ComponentTraits<generalszh::lifetime::LifetimeTarget>
{
	static constexpr std::string_view StableName = "games.generalszh.lifetime.target";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
template<> struct ComponentTraits<generalszh::lifetime::LifetimeDispatch>
{
	static constexpr std::string_view StableName = "games.generalszh.lifetime.dispatch";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
