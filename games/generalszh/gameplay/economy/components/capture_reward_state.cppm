module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.economy.components.capture_reward_state;
export import engine.ecs.core.component_registry;

export namespace generalszh::economy
{
struct CaptureRewardState
{
	bool available{false};
	bool initialized{false};
};
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::economy::CaptureRewardState>
{
	static constexpr std::string_view StableName = "games.generalszh.economy.capture_reward";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
