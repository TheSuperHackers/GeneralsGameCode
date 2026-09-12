module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>

export module engine.gameplay.rts.rank.inputs.rank_batch;
export import engine.ecs.core.entity;

export namespace engine::gameplay::rts::rank
{
// Final account-targeted skill awards. Attribution, authored value lookup and
// any modifier rounding happen before this boundary.
struct AcceptedRankPoints final
{
	ecs::Entity account{};
	std::uint64_t amount{};
};

enum class RankOutcome : std::uint8_t
{
	Applied,
	Clamped,
	InvalidTarget,
	InvalidState
};

struct RankResult final
{
	std::size_t requestIndex{};
	std::uint64_t tick{};
	ecs::Entity account{};
	std::uint64_t requested{};
	std::uint64_t applied{};
	std::uint64_t skillPointsBefore{};
	std::uint64_t skillPointsAfter{};
	std::uint64_t creditsBefore{};
	std::uint64_t creditsAfter{};
	std::uint32_t oldLevel{};
	std::uint32_t newLevel{};
	RankOutcome outcome{RankOutcome::InvalidTarget};

	[[nodiscard]] bool LevelChanged() const noexcept { return oldLevel != newLevel; }
};

class RankBatch final
{
public:
	explicit RankBatch(const std::size_t capacity) :
		inputs_(capacity), results_(capacity), order_(capacity), ranges_(capacity)
	{
	}

	void SetInputs(const std::span<const AcceptedRankPoints> inputs)
	{
		if (pending_ || prepared_)
			throw std::logic_error("Rank inputs are already pending");
		if (inputs.size() > inputs_.size())
			throw std::length_error("Rank input capacity exceeded");
		for (std::size_t index = 0; index < inputs.size(); ++index)
			inputs_[index] = inputs[index];
		count_ = inputs.size();
		published_ = 0;
		rangeCount_ = 0;
		pending_ = true;
	}

	// Ordered producers may append after host input staging and before the rank
	// system's caller-side preparation hook.
	void Append(const std::span<const AcceptedRankPoints> inputs)
	{
		if (!pending_ || prepared_)
			throw std::logic_error("Rank inputs are not appendable at this boundary");
		if (inputs.size() > inputs_.size() - count_)
			throw std::length_error("Rank input capacity exceeded");
		for (std::size_t index = 0; index < inputs.size(); ++index)
			inputs_[count_ + index] = inputs[index];
		count_ += inputs.size();
	}

	void Prepare(const std::uint64_t tick)
	{
		if (prepared_)
			throw std::logic_error("Rank batch is already prepared");
		if (!pending_)
			count_ = 0;
		published_ = 0;
		rangeCount_ = 0;
		for (std::size_t index = 0; index < count_; ++index)
		{
			order_[index] = index;
			results_[index] = RankResult{index, tick, inputs_[index].account,
				inputs_[index].amount};
		}
		std::sort(order_.begin(), order_.begin() + static_cast<std::ptrdiff_t>(count_),
			[this](const std::size_t left, const std::size_t right) {
				const auto &leftInput = inputs_[left];
				const auto &rightInput = inputs_[right];
				return std::tuple{leftInput.account.index, leftInput.account.generation, left} <
					std::tuple{rightInput.account.index, rightInput.account.generation, right};
			});
		for (std::size_t offset = 0; offset < count_;)
		{
			const ecs::Entity account = inputs_[order_[offset]].account;
			std::size_t end = offset + 1;
			while (end < count_ && inputs_[order_[end]].account == account)
				++end;
			ranges_[rangeCount_++] = AccountRange{account, offset, end - offset};
			offset = end;
		}
		prepared_ = true;
	}

	[[nodiscard]] std::size_t Capacity() const noexcept { return inputs_.size(); }
	[[nodiscard]] std::span<const std::size_t> Order() const noexcept
	{
		assert(prepared_);
		return {order_.data(), count_};
	}
	[[nodiscard]] const AcceptedRankPoints &Input(const std::size_t index) const noexcept
	{
		assert(prepared_ && index < count_);
		return inputs_[index];
	}
	[[nodiscard]] RankResult &Result(const std::size_t index) noexcept
	{
		assert(prepared_ && index < count_);
		return results_[index];
	}
	[[nodiscard]] std::span<const RankResult> Results() const noexcept
	{
		return {results_.data(), published_};
	}

	[[nodiscard]] bool FindRange(const ecs::Entity account,
		std::size_t &begin, std::size_t &count) const noexcept
	{
		assert(prepared_);
		std::size_t low = 0;
		std::size_t high = rangeCount_;
		while (low < high)
		{
			const std::size_t middle = low + (high - low) / 2;
			if (LessEntity(ranges_[middle].account, account))
				low = middle + 1;
			else
				high = middle;
		}
		if (low == rangeCount_ || ranges_[low].account != account)
			return false;
		begin = ranges_[low].begin;
		count = ranges_[low].count;
		return true;
	}

	void Publish()
	{
		if (!prepared_)
			throw std::logic_error("Rank batch is not prepared");
		published_ = count_;
		pending_ = false;
		prepared_ = false;
		count_ = 0;
		rangeCount_ = 0;
	}

private:
	static constexpr bool LessEntity(const ecs::Entity left, const ecs::Entity right) noexcept
	{
		return std::tuple{left.index, left.generation} < std::tuple{right.index, right.generation};
	}

	struct AccountRange final
	{
		ecs::Entity account{};
		std::size_t begin{};
		std::size_t count{};
	};

	std::vector<AcceptedRankPoints> inputs_;
	std::vector<RankResult> results_;
	std::vector<std::size_t> order_;
	std::vector<AccountRange> ranges_;
	std::size_t count_{};
	std::size_t published_{};
	std::size_t rangeCount_{};
	bool pending_{};
	bool prepared_{};
};
} // namespace engine::gameplay::rts::rank
