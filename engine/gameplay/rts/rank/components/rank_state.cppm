module;

#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.rank.components.rank_state;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::rank
{
struct RankState final
{
	std::uint64_t skillPoints{};
	std::uint32_t level{};
};
} // namespace engine::gameplay::rts::rank

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::rank::RankState>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.rank.state";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
} // namespace ecs
