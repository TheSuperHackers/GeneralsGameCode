module;
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
export module engine.gameplay.rts.economy.components.resource_balance;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::rts::economy
{
struct ResourceBalance { std::uint64_t quantity{0}; };
inline void Credit(ResourceBalance &balance, std::uint64_t amount)
{
	if (amount > (std::numeric_limits<std::uint64_t>::max)() - balance.quantity)
		throw std::overflow_error("Resource balance overflow");
	balance.quantity += amount;
}
inline std::uint64_t DebitUpTo(ResourceBalance &balance, std::uint64_t requested) noexcept
{
	const auto taken = std::min(balance.quantity, requested);
	balance.quantity -= taken;
	return taken;
}
}
export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::economy::ResourceBalance>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.economy.resource_balance";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
