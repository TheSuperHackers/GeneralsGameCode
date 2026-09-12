module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>

export module engine.gameplay.rts.unlocks.inputs.unlock_batch;
export import engine.ecs.core.entity;
export import engine.gameplay.rts.unlocks.definitions.unlock_definition;

export namespace engine::gameplay::rts::unlocks
{
enum class UnlockOperation : std::uint8_t
{
	Purchase,
	Grant,
	Availability
};

enum class UnlockResultCode : std::uint8_t
{
	Purchased,
	Granted,
	Available,
	InvalidTarget,
	InvalidOperation,
	UnknownUnlock,
	AlreadyOwned,
	Hidden,
	Disabled,
	MissingPrerequisite,
	ZeroCost,
	InsufficientCredits,
	NotGrantable
};

struct UnlockRequest final
{
	ecs::Entity account{};
	UnlockKey key{InvalidUnlockKey};
	UnlockOperation operation{UnlockOperation::Purchase};
};

struct UnlockReceipt final
{
	std::size_t requestIndex{};
	std::uint64_t tick{};
	ecs::Entity account{};
	UnlockKey key{InvalidUnlockKey};
	UnlockId id{InvalidUnlockId};
	UnlockOperation operation{UnlockOperation::Purchase};
	UnlockResultCode result{UnlockResultCode::InvalidTarget};
	std::uint64_t creditsBefore{};
	std::uint64_t creditsAfter{};
	bool owned{};
};

class UnlockBatch final
{
public:
	explicit UnlockBatch(const std::size_t capacity) :
		inputs_(capacity),
		receipts_(capacity),
		order_(capacity),
		ranges_(capacity)
	{
	}

	void SetInputs(std::span<const UnlockRequest> inputs)
	{
		if (pending_ || prepared_)
			throw std::logic_error("Unlock inputs are already pending");
		if (inputs.size() > inputs_.size())
			throw std::length_error("Unlock input capacity exceeded");
		for (std::size_t index = 0; index < inputs.size(); ++index)
			inputs_[index] = inputs[index];
		count_ = inputs.size();
		published_ = 0;
		rangeCount_ = 0;
		pending_ = true;
	}

	void Prepare(const std::uint64_t tick)
	{
		if (prepared_)
			throw std::logic_error("Unlock batch is already prepared");
		if (!pending_)
			count_ = 0;
		published_ = 0;
		rangeCount_ = 0;
		for (std::size_t index = 0; index < count_; ++index)
		{
			order_[index] = index;
			receipts_[index] = UnlockReceipt{
				index,
				tick,
				inputs_[index].account,
				inputs_[index].key,
				InvalidUnlockId,
				inputs_[index].operation,
				UnlockResultCode::InvalidTarget,
				0,
				0,
				false};
		}
		std::sort(order_.begin(), order_.begin() + count_, [this](const std::size_t left, const std::size_t right) {
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
	[[nodiscard]] const UnlockRequest &Input(const std::size_t index) const noexcept
	{
		assert(prepared_ && index < count_);
		return inputs_[index];
	}
	[[nodiscard]] UnlockReceipt &Receipt(const std::size_t index) noexcept
	{
		assert(prepared_ && index < count_);
		return receipts_[index];
	}
	[[nodiscard]] std::span<const UnlockReceipt> Results() const noexcept
	{
		return {receipts_.data(), published_};
	}

	// The query assigns these ranges directly to matching chunk rows. A false
	// result leaves the preinitialized InvalidTarget receipt intact, making stale
	// generation handles and missing account components observable to callers.
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
			throw std::logic_error("Unlock batch is not prepared");
		published_ = count_;
		pending_ = false;
		prepared_ = false;
		count_ = 0;
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

	std::vector<UnlockRequest> inputs_;
	std::vector<UnlockReceipt> receipts_;
	std::vector<std::size_t> order_;
	std::vector<AccountRange> ranges_;
	std::size_t count_{};
	std::size_t published_{};
	std::size_t rangeCount_{};
	bool pending_{};
	bool prepared_{};
};
} // namespace engine::gameplay::rts::unlocks
