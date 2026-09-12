module;

#include <cstdint>
#include <string_view>

export module games.generalszh.gameplay.upgrades.components.upgrade_status;

export import engine.ecs.core.component_registry;

export namespace generalszh::upgrades
{
enum class Status : std::int32_t
{
	Invalid = 0,
	InProduction = 1,
	Complete = 2
};

struct UpgradeInstanceStatus
{
	Status value{Status::Invalid};
};

constexpr bool IsValidStatus(const std::int32_t value) noexcept
{
	return value >= static_cast<std::int32_t>(Status::Invalid)
		&& value <= static_cast<std::int32_t>(Status::Complete);
}
}

export namespace ecs
{
template<>
struct ComponentTraits<generalszh::upgrades::UpgradeInstanceStatus>
{
	static constexpr std::string_view StableName =
		"games.generalszh.upgrades.instance_status";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
