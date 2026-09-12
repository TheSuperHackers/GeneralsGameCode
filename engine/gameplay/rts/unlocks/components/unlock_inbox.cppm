module;

#include <cstddef>
#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.unlocks.components.unlock_inbox;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::unlocks
{
// A transient range into the root-owned UnlockBatch. It is routing metadata,
// never an additional request owner or persistent unlock state.
struct UnlockInbox final
{
	std::size_t begin{};
	std::size_t count{};
};
} // namespace engine::gameplay::rts::unlocks

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::unlocks::UnlockInbox>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.unlocks.inbox";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
} // namespace ecs
