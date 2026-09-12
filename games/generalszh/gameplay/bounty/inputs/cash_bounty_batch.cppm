module;

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>

export module games.generalszh.gameplay.bounty.inputs.cash_bounty_batch;
export import games.generalszh.gameplay.economy.transactions.account_batch;

export namespace generalszh::bounty
{
struct CashBountyAward final
{
	ecs::Entity victim{};
	ecs::Entity source{};
	ecs::Entity account{};
	std::uint32_t amount{};
};

// Tick-local bounded output. The account balance and income history remain
// exclusively owned by AccountSystem; this object only carries settlement input
// across the explicit EconomySystem dependency.
class CashBountyBatch final
{
public:
	explicit CashBountyBatch(const std::size_t capacity) : capacity_(capacity)
	{
		if (capacity == 0)
			throw std::invalid_argument("Cash bounty batch capacity must be positive");
		awards_.reserve(capacity);
		requests_.reserve(capacity);
	}

	CashBountyBatch(const CashBountyBatch &) = delete;
	CashBountyBatch &operator=(const CashBountyBatch &) = delete;

	std::size_t Capacity() const noexcept { return capacity_; }

	void BeginTick()
	{
		if (stage_ == Stage::Published)
			throw std::logic_error("Cash bounty settlement was not consumed");
		awards_.clear();
		requests_.clear();
		stage_ = Stage::Empty;
	}

	void Publish(std::span<const CashBountyAward> awards)
	{
		if (stage_ != Stage::Empty)
			throw std::logic_error("Cash bounty batch is not ready for publication");
		if (awards.size() > capacity_)
			throw std::length_error("Cash bounty award capacity exhausted");
		for (const CashBountyAward &award : awards)
		{
			if (!award.victim.IsValid() || !award.source.IsValid() || !award.account.IsValid())
				throw std::invalid_argument("Cash bounty award has an invalid entity");
			if (award.amount == 0)
				throw std::invalid_argument("Cash bounty award must be positive");
		}
		awards_.assign(awards.begin(), awards.end());
		for (const CashBountyAward &award : awards_)
			requests_.push_back({award.account,
				{economy::AccountOperation::Deposit, award.amount, false, true}});
		stage_ = Stage::Published;
	}

	std::span<const CashBountyAward> Awards() const noexcept
	{
		return stage_ == Stage::Published ? std::span<const CashBountyAward>{awards_} : std::span<const CashBountyAward>{};
	}

	// Economy consumes the publication before the root returns from the tick.
	// Retain that bounded tick snapshot for diagnostics and deterministic host
	// reports without creating another balance or history owner.
	std::span<const CashBountyAward> SettledAwards() const noexcept
	{
		return stage_ == Stage::Published || stage_ == Stage::Consumed
			? std::span<const CashBountyAward>{awards_} : std::span<const CashBountyAward>{};
	}

	std::span<const economy::AccountRequest> Requests() const noexcept
	{
		return stage_ == Stage::Published ? std::span<const economy::AccountRequest>{requests_}
			: std::span<const economy::AccountRequest>{};
	}

	void Consume()
	{
		if (stage_ != Stage::Published)
			throw std::logic_error("Cash bounty batch has no published settlement");
		stage_ = Stage::Consumed;
	}

private:
	enum class Stage : std::uint8_t { Empty, Published, Consumed };
	const std::size_t capacity_;
	Stage stage_{Stage::Empty};
	std::vector<CashBountyAward> awards_;
	std::vector<economy::AccountRequest> requests_;
};
}
