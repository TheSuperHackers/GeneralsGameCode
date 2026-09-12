module;

#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>

export module games.generalszh.gameplay.crates.inputs.crate_reward_batch;
export import games.generalszh.gameplay.economy.transactions.account_batch;
export import engine.ecs.core.entity;

export namespace generalszh::crates
{
// AccountSystem remains the sole account-balance writer. Economy only merges
// this narrow publication seam into its account input batch.
class CrateRewardBatch final
{
public:
	explicit CrateRewardBatch(const std::size_t capacity) : requests_(capacity), crates_(capacity)
	{
		if (capacity == 0)
			throw std::invalid_argument("Crate reward batch capacity must be positive");
	}

	std::size_t Capacity() const noexcept { return requests_.size(); }

	void Publish(std::span<const economy::AccountRequest> requests,
		std::span<const ecs::Entity> consumedCrates)
	{
		if (published_)
			throw std::logic_error("Crate reward batch is already published");
		if (requests.size() > requests_.size() || consumedCrates.size() > crates_.size())
			throw std::length_error("Crate reward batch capacity exhausted");
		for (std::size_t index = 0; index != requests.size(); ++index)
			requests_[index] = requests[index];
		for (std::size_t index = 0; index != consumedCrates.size(); ++index)
			crates_[index] = consumedCrates[index];
		requestCount_ = requests.size(); crateCount_ = consumedCrates.size(); published_ = true;
	}

	bool IsPublished() const noexcept { return published_; }
	std::span<const economy::AccountRequest> Requests() const
	{
		if (!published_)
			throw std::logic_error("Crate reward requests are not published");
		return {requests_.data(), requestCount_};
	}
	std::span<const ecs::Entity> ConsumedCrates() const
	{
		if (!published_)
			throw std::logic_error("Crate reward crates are not published");
		return {crates_.data(), crateCount_};
	}

	void Release()
	{
		if (!published_)
			throw std::logic_error("Crate reward batch is not published");
		requestCount_ = crateCount_ = 0; published_ = false;
	}

private:
	std::vector<economy::AccountRequest> requests_;
	std::vector<ecs::Entity> crates_;
	std::size_t requestCount_{}, crateCount_{};
	bool published_{};
};
} // namespace generalszh::crates
