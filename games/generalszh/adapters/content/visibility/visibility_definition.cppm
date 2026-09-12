module;

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

export module games.generalszh.adapters.content.visibility.visibility_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import engine.gameplay.rts.visibility.definitions.visibility_definition;

export namespace generalszh::content::visibility
{
struct VisibilityContentScale final
{
	std::uint32_t worldUnitsPerCell{10};
};

struct DecodedVisibilityDefinition final
{
	engine::gameplay::rts::visibility::VisibilityDefinition definition{};
};

inline bool IsMinusOne(std::string_view value) noexcept
{
	value = generalszh::content::Trim(value);
	if (value.empty() || value.front() != '-') return false;
	value.remove_prefix(1);
	if (value.empty()) return false;
	std::uint64_t whole = 0;
	bool digit = false;
	bool dot = false;
	for (const char character : value)
	{
		if (character == '.' && !dot)
		{
			dot = true;
			continue;
		}
		if (character < '0' || character > '9') return false;
		if (dot) { if (character != '0') return false; continue; }
		if (whole > 1) return false;
		whole = whole * 10 + static_cast<std::uint64_t>(character - '0');
		digit = true;
	}
	return digit && whole == 1;
}

inline std::uint32_t RadiusCells(const std::string_view value, const VisibilityContentScale scale)
{
	if (!scale.worldUnitsPerCell)
		throw std::invalid_argument("Visibility world-units-per-cell scale must be positive");
	if (generalszh::content::Trim(value).starts_with('-'))
		throw std::invalid_argument("Visibility radius cannot be negative");
	const auto microWorldUnits = generalszh::content::ScaledDecimal(value, 1'000'000);
	const auto microWorldUnitsPerCell = std::uint64_t{scale.worldUnitsPerCell} * 1'000'000;
	const auto cells = microWorldUnits / microWorldUnitsPerCell +
		(microWorldUnits % microWorldUnitsPerCell ? 1u : 0u);
	if (cells > (std::numeric_limits<std::uint32_t>::max)())
		throw std::out_of_range("Visibility radius exceeds supported cell capacity");
	return static_cast<std::uint32_t>(cells);
}

inline engine::time::Duration DecodeMilliseconds(const std::string_view value)
{
	const auto nanos = generalszh::content::ScaledDecimal(value, 1'000'000);
	if (!nanos || nanos > static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)()))
		throw std::out_of_range("UnlookPersistDuration exceeds duration range");
	return engine::time::Duration{static_cast<std::int64_t>(nanos)};
}

inline DecodedVisibilityDefinition DecodeVisibilityDefinition(
	const generalszh::content::IniBlock &object,
	const engine::gameplay::rts::visibility::VisibilityDefinitionId id,
	const VisibilityContentScale scale,
	const std::uint32_t constructionRadiusCells,
	const std::string_view unlookPersistMilliseconds,
	const engine::time::FixedStep step)
{
	const auto visionText = object.Require("VisionRange");
	const auto shroudText = object.Value("ShroudClearingRange");
	const auto shroudRadius = shroudText.empty() || IsMinusOne(shroudText) ?
		RadiusCells(visionText, scale) : RadiusCells(shroudText, scale);
	if (!object.Value("ShroudRevealToAllRange").empty())
		throw std::invalid_argument("ShroudRevealToAllRange is outside ordinary participant visibility");
	return DecodedVisibilityDefinition{engine::gameplay::rts::visibility::CompileVisibilityDefinition(
		id, shroudRadius, constructionRadiusCells, DecodeMilliseconds(unlookPersistMilliseconds), step)};
}
} // namespace generalszh::content::visibility
