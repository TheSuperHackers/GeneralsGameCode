module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

export module engine.gameplay.rts.ai.squads.inputs.squad_order_request_batch;

export import engine.ecs.core.entity;
export import engine.gameplay.rts.orders.inputs.order_input;

export namespace engine::gameplay::rts::ai::squads
{

struct SquadOrderRequest
{
	ecs::Entity squad{};
	std::uint64_t revision{};
	engine::gameplay::rts::orders::OrderInput input{};
};

using SquadOrderInput = SquadOrderRequest;

inline bool SquadOrderRequestLess(const SquadOrderRequest &left,
	const SquadOrderRequest &right) noexcept
{
	if (left.squad != right.squad)
		return left.squad.index < right.squad.index ||
			(left.squad.index == right.squad.index && left.squad.generation < right.squad.generation);
	if (left.revision != right.revision)
		return left.revision < right.revision;
	return left.input.actor.index < right.input.actor.index ||
		(left.input.actor.index == right.input.actor.index &&
			left.input.actor.generation < right.input.actor.generation);
}

class SquadOrderRequestBatch final
{
public:
	explicit SquadOrderRequestBatch(const std::size_t capacity) : capacity_(capacity)
	{
		if (capacity_ == 0)
			throw std::invalid_argument("Squad order request capacity must be positive");
		requests_.reserve(capacity_);
	}

	SquadOrderRequestBatch(const SquadOrderRequestBatch &) = delete;
	SquadOrderRequestBatch &operator=(const SquadOrderRequestBatch &) = delete;

	void Begin(const std::uint64_t tick)
	{
		if (stage_ != Stage::Empty)
			throw std::logic_error("Squad order request batch must be released before reuse");
		tick_ = tick;
		stage_ = Stage::Collecting;
	}

	void Append(const SquadOrderRequest &request)
	{
		if (stage_ != Stage::Collecting)
			throw std::logic_error("Squad order request batch is not collecting");
		if (!request.squad.IsValid() || !request.input.actor.IsValid())
			throw std::invalid_argument("Squad order request identities must be valid");
		if (request.input.order.kind == engine::gameplay::rts::orders::OrderKind::None)
			throw std::invalid_argument("Squad order request cannot carry a None order");
		if (requests_.size() == capacity_)
			throw std::length_error("Squad order request batch capacity exhausted");
		requests_.push_back(request);
	}

	void Publish()
	{
		if (stage_ != Stage::Collecting)
			throw std::logic_error("Squad order request batch is not collecting");
		if (!std::is_sorted(requests_.begin(), requests_.end(), SquadOrderRequestLess))
			throw std::logic_error("Squad order requests are not in canonical order");
		for (std::size_t index = 1; index < requests_.size(); ++index)
			if (!SquadOrderRequestLess(requests_[index - 1], requests_[index]) &&
				!SquadOrderRequestLess(requests_[index], requests_[index - 1]))
				throw std::logic_error("Duplicate squad order request identity");
		stage_ = Stage::Published;
	}

	void Release()
	{
		if (stage_ != Stage::Published)
			throw std::logic_error("Only a published squad order request batch can be released");
		requests_.clear();
		stage_ = Stage::Empty;
	}

	[[nodiscard]] std::size_t Capacity() const noexcept { return capacity_; }
	[[nodiscard]] std::uint64_t Tick() const
	{
		if (stage_ != Stage::Published)
			throw std::logic_error("Squad order request batch has no published tick");
		return tick_;
	}
	[[nodiscard]] bool IsPublished() const noexcept { return stage_ == Stage::Published; }
	[[nodiscard]] std::span<const SquadOrderRequest> Requests() const
	{
		if (stage_ != Stage::Published)
			throw std::logic_error("Squad order requests are not published");
		return {requests_.data(), requests_.size()};
	}

private:
	enum class Stage : std::uint8_t { Empty, Collecting, Published };
	std::size_t capacity_{};
	std::uint64_t tick_{};
	Stage stage_{Stage::Empty};
	std::vector<SquadOrderRequest> requests_;
};

} // namespace engine::gameplay::rts::ai::squads
