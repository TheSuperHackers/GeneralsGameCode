module;

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

export module games.generalszh.adapters.content.collision.collision_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import games.generalszh.gameplay.collision.definitions.zero_hour_collision_definition;

export namespace generalszh::content
{
struct CollisionContentScale final
{
	std::uint32_t worldUnitsPerCell{};
};

namespace collision_detail
{
inline bool HasField(const IniBlock &object, const std::string_view key) noexcept
{
	for (const auto &field : object.fields)
		if (EqualToken(field.key, key))
			return true;
	return false;
}

inline bool IsGeometryField(const std::string_view key) noexcept
{
	return key.size() >= 8 && EqualToken(key.substr(0, 8), "Geometry");
}

inline bool Boolean(const std::string_view value, const std::string_view field)
{
	if (EqualToken(Trim(value), "Yes")) return true;
	if (EqualToken(Trim(value), "No")) return false;
	throw std::invalid_argument("Zero Hour collision boolean requires Yes or No: " + std::string(field));
}

inline std::uint32_t MicroCellsFromWorld(const std::string_view authored,
	const std::uint32_t worldUnitsPerCell, const std::string_view field)
{
	if (worldUnitsPerCell == 0)
		throw std::invalid_argument("Collision content scale requires worldUnitsPerCell");
	const auto microWorldUnits = ScaledDecimal(authored,
		engine::gameplay::spatial::contacts::FixedScale);
	// Content is authored in world units while ContactGeometry stores microcells.
	// Quantization is conservative: ceil(micro-world-units / worldUnitsPerCell),
	// so a non-zero remainder keeps the shape from becoming smaller.
	const auto quotient = microWorldUnits / worldUnitsPerCell;
	const auto remainder = microWorldUnits % worldUnitsPerCell;
	const auto rounded = quotient + (remainder == 0 ? 0u : 1u);
	if (rounded == 0 || rounded > engine::gameplay::spatial::contacts::MaxExtentMicrocells)
		throw std::out_of_range("Zero Hour collision extent exceeds the fixed contract: " + std::string(field));
	return static_cast<std::uint32_t>(rounded);
}

inline std::string_view ValueOr(const IniBlock &object, const std::string_view key,
	const std::string_view fallback) noexcept
{
	const auto value = object.Value(key);
	return value.empty() ? fallback : value;
}

inline void ValidateGeometryFields(const IniBlock &object)
{
	for (const auto &field : object.fields)
	{
		const auto key = std::string_view{field.key};
		if (!IsGeometryField(key) && EqualToken(key, "Angle"))
			throw std::invalid_argument("Zero Hour collision heading is outside the bounded 2D slice");
		if (!IsGeometryField(key))
			continue;
		if (EqualToken(key, "Geometry") || EqualToken(key, "GeometryMajorRadius") ||
			EqualToken(key, "GeometryMinorRadius") || EqualToken(key, "GeometryHeight") ||
			EqualToken(key, "GeometryIsSmall"))
			continue;
		throw std::invalid_argument("Unsupported Zero Hour collision geometry field: " + field.key);
	}
	if (HasField(object, "GeometryAngle"))
		throw std::invalid_argument("Zero Hour collision heading is outside the bounded 2D slice");
}
} // namespace collision_detail

inline generalszh::collision::ZeroHourCollisionDefinition DecodeZeroHourCollisionDefinition(
	const IniBlock &object, const generalszh::collision::CollisionDefinitionId id,
	const CollisionContentScale scale)
{
	if (!EqualToken(object.kind, "Object") || object.argument.empty())
		throw std::invalid_argument("Zero Hour collision decoder requires a named Object block");
	collision_detail::ValidateGeometryFields(object);

	const auto authoredShape = collision_detail::ValueOr(object, "Geometry", "SPHERE");
	const auto major = collision_detail::MicroCellsFromWorld(
		collision_detail::ValueOr(object, "GeometryMajorRadius", "1"),
		scale.worldUnitsPerCell, "GeometryMajorRadius");
	const auto minorText = collision_detail::ValueOr(object, "GeometryMinorRadius", "1");
	const auto minor = collision_detail::MicroCellsFromWorld(minorText,
		scale.worldUnitsPerCell, "GeometryMinorRadius");

	if (collision_detail::HasField(object, "GeometryHeight"))
	{
		const auto height = ScaledDecimal(object.Value("GeometryHeight"),
			engine::gameplay::spatial::contacts::FixedScale);
		if (height == 0)
			throw std::invalid_argument("Zero Hour collision GeometryHeight must be positive");
	}
	if (collision_detail::HasField(object, "GeometryIsSmall"))
		(void)collision_detail::Boolean(object.Value("GeometryIsSmall"), "GeometryIsSmall");

	engine::gameplay::spatial::contacts::ContactGeometry geometry{};
	if (EqualToken(authoredShape, "SPHERE") || EqualToken(authoredShape, "CYLINDER"))
		geometry = engine::gameplay::spatial::contacts::Circle(major);
	else if (EqualToken(authoredShape, "BOX"))
		geometry = engine::gameplay::spatial::contacts::AxisAlignedBox(major, minor);
	else
		throw std::invalid_argument("Unsupported Zero Hour collision Geometry: " + std::string(authoredShape));

	generalszh::collision::ZeroHourCollisionDefinition result{
		id, geometry, {},
		static_cast<std::uint8_t>(generalszh::collision::ZeroHourCollisionOmission::Height) |
			static_cast<std::uint8_t>(generalszh::collision::ZeroHourCollisionOmission::IsSmall) |
			static_cast<std::uint8_t>(generalszh::collision::ZeroHourCollisionOmission::Heading)};
	generalszh::collision::ValidateZeroHourCollisionDefinition(result);
	return result;
}

inline generalszh::collision::ZeroHourCollisionDefinition DecodeZeroHourCollisionDefinition(
	const std::string_view text, const std::string_view name,
	const generalszh::collision::CollisionDefinitionId id, const CollisionContentScale scale)
{
	return DecodeZeroHourCollisionDefinition(ReadNamedBlock(text, "Object", name), id, scale);
}
} // namespace generalszh::content
