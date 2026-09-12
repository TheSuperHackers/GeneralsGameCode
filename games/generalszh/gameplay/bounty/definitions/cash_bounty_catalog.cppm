module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

export module games.generalszh.gameplay.bounty.definitions.cash_bounty_catalog;
export import engine.gameplay.rts.bounty.algorithms.bounty_amount;
export import engine.gameplay.rts.unlocks.components.unlock_state;
export import engine.gameplay.rts.unlocks.definitions.unlock_catalog;
export import games.generalszh.gameplay.bounty.definitions.cash_bounty_definition;

export namespace generalszh::bounty
{
class CashBountyCatalog final
{
public:
	struct Entry final
	{
		engine::gameplay::rts::unlocks::UnlockId science{};
		engine::gameplay::rts::bounty::BountyRate rate{};
	};

	CashBountyCatalog(std::span<const CashBountyDefinition> definitions,
		const engine::gameplay::rts::unlocks::UnlockCatalog &unlocks)
	{
		if (!unlocks.IsFinalized())
			throw std::logic_error("Cash bounty catalog requires a finalized unlock catalog");
		entries_.reserve(definitions.size());
		for (const auto &definition : definitions)
		{
			if (definition.science == engine::gameplay::rts::unlocks::InvalidUnlockKey)
				throw std::invalid_argument("Cash bounty definition has no science key");
			if (!engine::gameplay::rts::bounty::IsValidBountyRate(definition.rate))
				throw std::invalid_argument("Cash bounty definition has an invalid rate");
			const auto id = unlocks.Find(definition.science);
			if (id == engine::gameplay::rts::unlocks::InvalidUnlockId)
				throw std::invalid_argument("Cash bounty definition names an unknown science");
			entries_.push_back({id, definition.rate});
		}
		std::sort(entries_.begin(), entries_.end(), [](const Entry &left, const Entry &right) {
			return left.science.value < right.science.value;
		});
		for (std::size_t index = 1; index < entries_.size(); ++index)
			if (entries_[index - 1].science == entries_[index].science)
				throw std::invalid_argument("Duplicate science in cash bounty catalog");
		schemaHash_ = unlocks.SchemaHash();
	}

	std::uint64_t SchemaHash() const noexcept { return schemaHash_; }
	std::span<const Entry> Entries() const noexcept { return entries_; }

	engine::gameplay::rts::bounty::BountyRate MaximumOwned(
		const engine::gameplay::rts::unlocks::UnlockState &state) const noexcept
	{
		auto maximum = engine::gameplay::rts::bounty::BountyRate{};
		for (const Entry &entry : entries_)
			if (state.IsOwned(entry.science) &&
				engine::gameplay::rts::bounty::GreaterBountyRate(entry.rate, maximum))
				maximum = entry.rate;
		return maximum;
	}

private:
	std::vector<Entry> entries_;
	std::uint64_t schemaHash_{};
};
}
