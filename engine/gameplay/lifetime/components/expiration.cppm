module;
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>

export module engine.gameplay.lifetime.components.expiration;
export import engine.ecs.core.component_registry;
export import engine.time.simulation_time;

namespace engine::gameplay::lifetime::detail
{
// Module-owned cold throw preserves worker exception_ptr copy metadata on Clang.
class ExpirationOverflow final : public std::overflow_error
{
public:
    ExpirationOverflow() : std::overflow_error("Expiration deadline exceeds the simulation tick range") {}
};
[[noreturn]] void ThrowExpirationOverflow() { throw ExpirationOverflow{}; }
}

export namespace engine::gameplay::lifetime
{
enum class ExpirationState : std::uint8_t { Inactive, Armed, Elapsed };

struct Expiration
{
	std::uint64_t deadline{0};
	ExpirationState state{ExpirationState::Inactive};
};

// Pure setup/reset operation. A zero duration means due at this tick. Adapters
// impose any game-specific minimum duration before calling this function.
inline void ArmAfter(Expiration &expiration, std::uint64_t now, std::uint64_t duration)
{
	if (duration > (std::numeric_limits<std::uint64_t>::max)() - now)
		detail::ThrowExpirationOverflow();
	expiration = Expiration{now + duration, ExpirationState::Armed};
}

// Author in typed time; quantize once at setup/reset, not during chunk iteration.
inline void ArmAfter(Expiration &expiration, engine::time::SimulationTime now, engine::time::Duration duration)
{
	ArmAfter(expiration, now.Tick(), now.Step().TicksFor(duration));
}
}

export namespace ecs
{
template<>
struct ComponentTraits<engine::gameplay::lifetime::Expiration>
{
	static constexpr std::string_view StableName = "engine.gameplay.lifetime.expiration";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
