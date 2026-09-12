module;

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

export module engine.gameplay.rts.ai.squads.inputs.squad_command_batch;

export import engine.ecs.core.entity;
export import engine.gameplay.navigation.grid.navigation_grid;
export import engine.gameplay.rts.ai.squads.components.squad_goal;

export namespace engine::gameplay::rts::ai::squads
{

struct SquadGoalPayload
{
	SquadGoalKind kind{SquadGoalKind::None};
	ecs::Entity target{};
	navigation::Cell destination{navigation::InvalidCell};
	std::uint32_t searchRadiusCells{};
};

enum class SquadCommandKind : std::uint8_t
{
	SetGoal,
	Assign,
	Remove
};

// A command is an explicit request at the simulation boundary.  Ownership and
// revisions are supplied by the authoritative ECS state, never by the caller.
struct SquadCommand
{
	SquadCommandKind kind{SquadCommandKind::SetGoal};
	ecs::Entity issuer{};
	ecs::Entity squad{};
	ecs::Entity actor{};
	ecs::Entity expectedMembership{};
	std::uint64_t expectedRevision{};
	SquadGoalPayload goal{};
};

enum class SquadCommandStatus : std::uint8_t
{
	Accepted,
	InvalidCommand,
	InvalidIssuer,
	InvalidSquad,
	StaleGoalRevision,
	InvalidGoal,
	InvalidTarget,
	InvalidActor,
	NotOwned,
	AlreadyMember,
	MissingMembership,
	StaleMembership,
	TaskBusy,
	DuplicateSquad,
	DuplicateActor
};

struct SquadCommandReceipt
{
	// This is the ordinal in the canonical host-provided command span for this
	// tick. It is local receipt association only, not network-arrival order or a
	// persistent command identity.
	std::size_t sequence{};
	SquadCommandKind kind{SquadCommandKind::SetGoal};
	SquadCommandStatus status{SquadCommandStatus::InvalidCommand};
	ecs::Entity issuer{};
	ecs::Entity squad{};
	ecs::Entity actor{};
	std::uint64_t revision{};
	ecs::Entity membership{};
};

class SquadCommandBatch final
{
public:
	// The composition/input boundary appends commands in its already-canonical
	// span order. The batch preserves that order; it does not sort by arrival,
	// worker, address, or a transport/persistence key.
	explicit SquadCommandBatch(const std::size_t capacity) : capacity_(capacity)
	{
		if (capacity_ == 0)
			throw std::invalid_argument("Squad command capacity must be positive");
		commands_.reserve(capacity_);
		receipts_.reserve(capacity_);
	}

	SquadCommandBatch(const SquadCommandBatch &) = delete;
	SquadCommandBatch &operator=(const SquadCommandBatch &) = delete;

	void Begin(const std::uint64_t tick)
	{
		if (stage_ != Stage::Empty)
			throw std::logic_error("Squad command batch must be released before reuse");
		tick_ = tick;
		commands_.clear();
		receipts_.clear();
		stage_ = Stage::Collecting;
	}

	void Append(const SquadCommand &command)
	{
		if (stage_ != Stage::Collecting)
			throw std::logic_error("Squad command batch is not collecting");
		if (commands_.size() == capacity_)
			throw std::length_error("Squad command batch capacity exhausted");
		ValidateInput(command);
		commands_.push_back(command);
	}

	void Append(const std::span<const SquadCommand> commands)
	{
		if (stage_ != Stage::Collecting)
			throw std::logic_error("Squad command batch is not collecting");
		if (commands.size() > capacity_ - commands_.size())
			throw std::length_error("Squad command batch capacity exhausted");
		for (const auto &command : commands)
			ValidateInput(command);
		commands_.insert(commands_.end(), commands.begin(), commands.end());
	}

	void Publish()
	{
		if (stage_ != Stage::Collecting)
			throw std::logic_error("Squad command batch is not collecting");
		stage_ = Stage::Published;
	}

	void RecordReceipt(SquadCommandReceipt receipt)
	{
		if (stage_ != Stage::Published)
			throw std::logic_error("Squad command batch is not published");
		if (receipt.sequence != receipts_.size() || receipts_.size() == capacity_)
			throw std::logic_error("Squad command receipts must be recorded in input order");
		receipts_.push_back(receipt);
	}

	void Release()
	{
		if (stage_ != Stage::Published)
			throw std::logic_error("Only a published squad command batch can be released");
		if (receipts_.size() != commands_.size())
			throw std::logic_error("Squad command batch cannot release before receipts are complete");
		commands_.clear();
		receipts_.clear();
		stage_ = Stage::Empty;
	}

	[[nodiscard]] std::size_t Capacity() const noexcept { return capacity_; }
	[[nodiscard]] std::uint64_t Tick() const
	{
		if (stage_ != Stage::Published)
			throw std::logic_error("Squad command batch has no published tick");
		return tick_;
	}
	[[nodiscard]] bool IsPublished() const noexcept { return stage_ == Stage::Published; }
	[[nodiscard]] std::span<const SquadCommand> Commands() const
	{
		if (stage_ != Stage::Published)
			throw std::logic_error("Squad command batch is not published");
		return {commands_.data(), commands_.size()};
	}
	[[nodiscard]] std::span<const SquadCommandReceipt> Receipts() const
	{
		if (stage_ != Stage::Published)
			throw std::logic_error("Squad command receipts are not published");
		return {receipts_.data(), receipts_.size()};
	}

private:
	static void ValidateInput(const SquadCommand &command)
	{
		if (!command.issuer.IsValid() || !command.squad.IsValid())
			throw std::invalid_argument("Squad command issuer and squad must be valid");
		if ((command.kind == SquadCommandKind::Assign || command.kind == SquadCommandKind::Remove) &&
			!command.actor.IsValid())
			throw std::invalid_argument("Squad membership command actor must be valid");
		if (command.kind == SquadCommandKind::SetGoal && command.actor.IsValid())
			throw std::invalid_argument("Squad goal command cannot carry an actor");
	}

	enum class Stage : std::uint8_t { Empty, Collecting, Published };
	std::size_t capacity_{};
	std::uint64_t tick_{};
	Stage stage_{Stage::Empty};
	std::vector<SquadCommand> commands_;
	std::vector<SquadCommandReceipt> receipts_;
};

} // namespace engine::gameplay::rts::ai::squads
