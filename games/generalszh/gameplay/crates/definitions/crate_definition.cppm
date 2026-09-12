module;

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

export module games.generalszh.gameplay.crates.definitions.crate_definition;
export import engine.gameplay.rts.unlocks.definitions.unlock_definition;

export namespace generalszh::crates
{
using KindOfMask = std::uint64_t;
inline constexpr KindOfMask KindOfStructure = UINT64_C(1) << 0;
inline constexpr KindOfMask KindOfParachute = UINT64_C(1) << 1;

enum class RewardKind : std::uint8_t
{
	Money,
	Heal,
	Veterancy,
	Salvage,
	Unit,
	Shroud,
	Sabotage
};

struct MoneyUpgradeBoost final
{
	std::uint32_t definition{};
	std::uint32_t amount{};
};

struct CrateDefinition final
{
	RewardKind reward{RewardKind::Money};
	std::uint64_t requiredKindOf{};
	std::uint64_t forbiddenKindOf{};
	engine::gameplay::rts::unlocks::UnlockId pickupScience{};
	bool forbidOwner{};
	bool buildingPickup{};
	bool humanOnly{};
	bool allowMultiPickup{};
	std::uint32_t moneyProvided{};
	std::vector<MoneyUpgradeBoost> moneyUpgradeBoosts{};
	std::uint32_t effectRange{};
	bool addsOwnerVeterancy{};
	bool isPilot{};
};

class CrateDefinitionCatalog final
{
public:
	explicit CrateDefinitionCatalog(std::span<const CrateDefinition> definitions) :
		definitions_(definitions.begin(), definitions.end())
	{
		if (definitions_.empty())
			throw std::invalid_argument("Crate definition catalog cannot be empty");
		for (const auto &definition : definitions_)
		{
			switch (definition.reward)
			{
			case RewardKind::Money:
			case RewardKind::Heal:
			case RewardKind::Veterancy:
				break;
			default:
				throw std::invalid_argument(
					"Crate reward family is outside the bounded money/heal/veterancy slice");
			}
			for (const auto &boost : definition.moneyUpgradeBoosts)
			{
				const auto total = static_cast<std::uint64_t>(definition.moneyProvided) +
					static_cast<std::uint64_t>(boost.amount);
				if (total > static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()))
					throw std::invalid_argument(
						"Crate money reward plus an upgrade boost exceeds account request range");
			}
		}
		for (const auto &definition : definitions_)
			if (definition.reward == RewardKind::Veterancy && definition.isPilot)
				throw std::invalid_argument(
					"Pilot veterancy crates require an unavailable typed collector locomotor/air-state boundary");
	}

	std::size_t Size() const noexcept { return definitions_.size(); }
	const CrateDefinition &Get(const std::uint32_t index) const
	{
		if (index >= definitions_.size())
			throw std::out_of_range("Crate definition index out of range");
		return definitions_[index];
	}

private:
	std::vector<CrateDefinition> definitions_;
};
} // namespace generalszh::crates
