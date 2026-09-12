module;
#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.repair.components.manual_repair;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export import engine.gameplay.navigation.grid.navigation_grid;
export import engine.gameplay.rts.repair.components.repair_state;

export namespace engine::gameplay::rts::repair
{
// Manual repair deliberately has its own binding.  RepairBinding/RepairState
// remain the independent automatic/base-repair capability on the target.
struct ManualRepairRateBinding
{
	std::uint32_t definition{};
};

// This is a serializable wrapper around a value-shaped RepairState.  It is not
// an ECS RepairState component and therefore cannot alias BaseRepairSystem's
// authoritative target state.
struct ManualRepairProgress
{
	RepairState state{};
};

// A task assignment is an actor-owned destination, not a destination lease.
// The destination remains contestable while actors travel toward it.
struct ManualRepairAssignment
{
	ecs::Entity target{};
	navigation::Cell cell{navigation::InvalidCell};
};

// The benefactor lease is target-owned and finite.  expiryTick is an
// exclusive deadline: it is active for ticks [acquired, expiryTick), and is
// expired when currentTick >= expiryTick.  The system compiles the injected
// typed retention Duration to this tick field at startup.
struct ManualRepairBenefactorLease
{
	ecs::Entity benefactor{};
	std::uint64_t expiryTick{};
};

inline bool IsActive(const ManualRepairBenefactorLease &lease, std::uint64_t tick) noexcept
{
	return lease.benefactor.IsValid() && tick < lease.expiryTick;
}
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::repair::ManualRepairRateBinding>
{
	static constexpr std::string_view StableName="engine.gameplay.rts.repair.manual_rate_binding";
	static constexpr std::uint32_t Version=1;
	static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};

template<> struct ComponentTraits<engine::gameplay::rts::repair::ManualRepairProgress>
{
	static constexpr std::string_view StableName="engine.gameplay.rts.repair.manual_progress";
	static constexpr std::uint32_t Version=1;
	static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};

template<> struct ComponentTraits<engine::gameplay::rts::repair::ManualRepairAssignment>
{
	static constexpr std::string_view StableName="engine.gameplay.rts.repair.manual_assignment";
	static constexpr std::uint32_t Version=1;
	static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};

template<> struct ComponentTraits<engine::gameplay::rts::repair::ManualRepairBenefactorLease>
{
	static constexpr std::string_view StableName="engine.gameplay.rts.repair.manual_benefactor_lease";
	static constexpr std::uint32_t Version=1;
	static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
