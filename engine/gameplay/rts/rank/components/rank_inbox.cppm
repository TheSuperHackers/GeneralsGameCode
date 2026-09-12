module;

#include <cstddef>
#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.rank.components.rank_inbox;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::rank
{
// Tick-local routing metadata into the root-owned RankBatch. It is not an
// additional rank-award owner and is never persisted.
struct RankInbox final
{
	std::size_t begin{};
	std::size_t count{};
};
} // namespace engine::gameplay::rts::rank

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::rank::RankInbox>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.rank.inbox";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
} // namespace ecs
