module;

#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>

export module games.generalszh.gameplay.radar.definitions.radar_provider_definition;

export namespace generalszh::radar
{
struct RadarUpgradeBinding final
{
	std::uint32_t definitionId{};
	std::uint32_t wordOrdinal{};
	std::uint64_t bitMask{};
	std::uint64_t schemaHash{};
};

enum class RadarGrantAuthority : std::uint8_t
{
	CreationGrant,
	CompletionGrant,
	ObjectUpgradeCompletion
};

struct RadarProviderDefinition final
{
	std::uint32_t id{};
	RadarUpgradeBinding upgrade{};
	RadarGrantAuthority authority{RadarGrantAuthority::CompletionGrant};
	bool resistant{};
	bool requiresComplete{true};
};

static_assert(std::is_standard_layout_v<RadarUpgradeBinding>);
static_assert(std::is_trivially_copyable_v<RadarUpgradeBinding>);
static_assert(std::is_standard_layout_v<RadarProviderDefinition>);
static_assert(std::is_trivially_copyable_v<RadarProviderDefinition>);

class RadarProviderDefinitions final
{
public:
	explicit RadarProviderDefinitions(const std::span<const RadarProviderDefinition> definitions) :
		definitions_(definitions)
	{
		for (std::size_t index = 0; index != definitions_.size(); ++index)
		{
			const auto &definition = definitions_[index];
			if (definition.id != index)
				throw std::invalid_argument("Radar provider definition IDs must be dense");
			if (!std::has_single_bit(definition.upgrade.bitMask))
				throw std::invalid_argument("Radar upgrade binding must contain one bit");
			if (definition.upgrade.wordOrdinal != definition.upgrade.definitionId / 64u ||
				definition.upgrade.bitMask !=
					(std::uint64_t{1} << (definition.upgrade.definitionId % 64u)))
				throw std::invalid_argument("Radar upgrade binding is not canonical for its dense ID");
			if (definition.upgrade.schemaHash == 0)
				throw std::invalid_argument("Radar upgrade binding requires a finalized schema hash");
			switch (definition.authority)
			{
			case RadarGrantAuthority::CreationGrant:
			case RadarGrantAuthority::CompletionGrant:
				break;
			case RadarGrantAuthority::ObjectUpgradeCompletion:
				throw std::invalid_argument(
					"Object-upgrade radar authority requires a registered concrete producer");
			default:
				throw std::invalid_argument("Radar provider authority is invalid");
			}
		}
	}

	[[nodiscard]] const RadarProviderDefinition &Get(const std::uint32_t id) const
	{
		if (id >= definitions_.size())
			throw std::out_of_range("Radar provider definition ID is outside the catalog");
		return definitions_[id];
	}

	[[nodiscard]] std::size_t Size() const noexcept { return definitions_.size(); }

private:
	std::span<const RadarProviderDefinition> definitions_;
};
}
