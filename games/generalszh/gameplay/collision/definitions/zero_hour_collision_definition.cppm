module;

#include <cstdint>
#include <limits>
#include <stdexcept>

export module games.generalszh.gameplay.collision.definitions.zero_hour_collision_definition;
export import engine.gameplay.spatial.contacts.algorithms.contact_geometry;
export import engine.gameplay.spatial.contacts.algorithms.contact_participation;

export namespace generalszh::collision
{
using CollisionDefinitionId = std::uint32_t;
inline constexpr CollisionDefinitionId InvalidCollisionDefinition =
	(std::numeric_limits<CollisionDefinitionId>::max)();

enum class ZeroHourCollisionOmission : std::uint8_t
{
	Height = 1u << 0,
	IsSmall = 1u << 1,
	Heading = 1u << 2
};

inline constexpr std::uint8_t operator|(const ZeroHourCollisionOmission left,
	const ZeroHourCollisionOmission right) noexcept
{
	return static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right);
}

struct ZeroHourCollisionDefinition final
{
	CollisionDefinitionId id{InvalidCollisionDefinition};
	engine::gameplay::spatial::contacts::ContactGeometry geometry{};
	engine::gameplay::spatial::contacts::ContactParticipation participation{};
	std::uint8_t omissions{
		static_cast<std::uint8_t>(ZeroHourCollisionOmission::Height) |
		static_cast<std::uint8_t>(ZeroHourCollisionOmission::IsSmall) |
		static_cast<std::uint8_t>(ZeroHourCollisionOmission::Heading)};
};

inline void ValidateZeroHourCollisionDefinition(const ZeroHourCollisionDefinition &definition)
{
	if (definition.id == InvalidCollisionDefinition)
		throw std::invalid_argument("Zero Hour collision definition requires a valid ID");
	engine::gameplay::spatial::contacts::ValidateGeometry(definition.geometry);
	engine::gameplay::spatial::contacts::ValidateParticipation(definition.participation);
	constexpr auto supported = static_cast<std::uint8_t>(ZeroHourCollisionOmission::Height) |
		static_cast<std::uint8_t>(ZeroHourCollisionOmission::IsSmall) |
		static_cast<std::uint8_t>(ZeroHourCollisionOmission::Heading);
	if ((definition.omissions & ~supported) != 0)
		throw std::invalid_argument("Zero Hour collision definition contains an unknown omission");
}
} // namespace generalszh::collision
