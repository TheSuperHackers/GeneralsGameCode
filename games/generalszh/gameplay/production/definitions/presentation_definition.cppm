module;

#include <cstddef>
#include <cstdint>
#include <string_view>

export module games.generalszh.gameplay.production.definitions.presentation_definition;

export import games.generalszh.gameplay.production.doors.production_door;
export import games.generalszh.gameplay.production.presentation.construction_marker;
export import engine.time.simulation_time;

namespace generalszh::production_presentation_definition_detail
{
constexpr std::string_view FingerprintDomain =
	"games.generalszh.production.presentation_definition.recipe";
constexpr std::uint64_t FnvOffsetBasis = UINT64_C(14695981039346656037);
constexpr std::uint64_t FnvPrime = UINT64_C(1099511628211);

void HashByte(std::uint64_t &hash, const std::uint8_t byte) noexcept
{
	hash ^= byte;
	hash *= FnvPrime;
}

void HashLittleEndian(std::uint64_t &hash, const std::uint64_t value,
	const std::size_t byteCount) noexcept
{
	for (std::size_t byte = 0; byte < byteCount; ++byte)
		HashByte(hash, static_cast<std::uint8_t>(value >> (byte * 8)));
}

void HashBytes(std::uint64_t &hash, const std::string_view bytes) noexcept
{
	for (const char byte : bytes)
		HashByte(hash, static_cast<std::uint8_t>(static_cast<unsigned char>(byte)));
}

} // namespace generalszh::production_presentation_definition_detail

export namespace generalszh::production
{

class ProductionPresentationDefinition
{
public:
	static constexpr std::uint32_t Version = 1;

	explicit ProductionPresentationDefinition(std::int32_t animations,
		engine::time::Duration opening,
		engine::time::Duration waiting,
		engine::time::Duration closing,
		engine::time::Duration complete,
		engine::time::FixedStep step);

	ProductionPresentationDefinition(const ProductionPresentationDefinition &) = default;
	ProductionPresentationDefinition &operator=(
		const ProductionPresentationDefinition &) = delete;
	ProductionPresentationDefinition &operator=(ProductionPresentationDefinition &&) = delete;

	std::int32_t NumDoorAnimations() const noexcept
	{
		return animations_;
	}

	const DoorTiming &Doors() const noexcept
	{
		return doors_;
	}

	const ConstructionMarkerTiming &Marker() const noexcept
	{
		return marker_;
	}

	engine::time::FixedStep Step() const noexcept
	{
		return step_;
	}

	std::uint64_t Fingerprint() const noexcept
	{
		return fingerprint_;
	}

private:
	std::uint64_t CalculateFingerprint() const noexcept
	{
		std::uint64_t hash = production_presentation_definition_detail::FnvOffsetBasis;
		production_presentation_definition_detail::HashLittleEndian(hash,
			static_cast<std::uint64_t>(
				production_presentation_definition_detail::FingerprintDomain.size()),
			sizeof(std::uint64_t));
		production_presentation_definition_detail::HashBytes(hash,
			production_presentation_definition_detail::FingerprintDomain);
		production_presentation_definition_detail::HashLittleEndian(hash,
			Version, sizeof(Version));
		production_presentation_definition_detail::HashLittleEndian(hash,
			static_cast<std::uint32_t>(animations_), sizeof(std::uint32_t));
		production_presentation_definition_detail::HashLittleEndian(hash,
			step_.TicksPerSecond(), sizeof(std::uint32_t));
		production_presentation_definition_detail::HashLittleEndian(hash,
			doors_.opening, sizeof(std::uint64_t));
		production_presentation_definition_detail::HashLittleEndian(hash,
			doors_.waiting, sizeof(std::uint64_t));
		production_presentation_definition_detail::HashLittleEndian(hash,
			doors_.closing, sizeof(std::uint64_t));
		production_presentation_definition_detail::HashLittleEndian(hash,
			marker_.duration, sizeof(std::uint64_t));
		return hash;
	}

	const engine::time::FixedStep step_;
	const DoorTiming doors_;
	const ConstructionMarkerTiming marker_;
	const std::int32_t animations_;
	const std::uint64_t fingerprint_;
};

} // namespace generalszh::production

generalszh::production::ProductionPresentationDefinition::ProductionPresentationDefinition(
	const std::int32_t animations,
	const engine::time::Duration opening,
	const engine::time::Duration waiting,
	const engine::time::Duration closing,
	const engine::time::Duration complete,
	const engine::time::FixedStep step)
	: step_(step),
	  doors_(generalszh::production::AuthorDoorTiming(opening, waiting, closing, step_)),
	  marker_(generalszh::production::AuthorConstructionMarkerTiming(complete, step_)),
	  animations_(animations),
	  fingerprint_(CalculateFingerprint())
{
}
