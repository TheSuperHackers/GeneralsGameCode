module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

export module games.generalszh.gameplay.ai.squads.systems.squad_command_system;

export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.containment.components.containment_task;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.rts.construction.components.construction_components;
export import engine.gameplay.rts.economy.components.resource_balance;
export import engine.gameplay.rts.harvesting.components.harvest_state;
export import engine.gameplay.rts.repair.components.manual_repair;
export import engine.gameplay.rts.ai.squads.algorithms.squad_goal;
export import engine.gameplay.rts.ai.squads.components.squad_goal_application;
export import engine.gameplay.rts.ai.squads.components.squad_membership;
export import engine.gameplay.rts.ai.squads.inputs.squad_command_batch;
export import games.generalszh.gameplay.capture.components.capture_state;
export import games.generalszh.gameplay.construction.components.structure;
export import games.generalszh.gameplay.production.components.production_state;

export namespace generalszh::ai::squads
{

using SquadCommand = engine::gameplay::rts::ai::squads::SquadCommand;
using SquadCommandBatch = engine::gameplay::rts::ai::squads::SquadCommandBatch;
using SquadCommandKind = engine::gameplay::rts::ai::squads::SquadCommandKind;
using SquadCommandReceipt = engine::gameplay::rts::ai::squads::SquadCommandReceipt;
using SquadCommandStatus = engine::gameplay::rts::ai::squads::SquadCommandStatus;
using SquadGoal = engine::gameplay::rts::ai::squads::SquadGoal;
using SquadGoalApplication = engine::gameplay::rts::ai::squads::SquadGoalApplication;
using SquadMembership = engine::gameplay::rts::ai::squads::SquadMembership;

class SquadCommandSystem final
{
public:
	using Query = ecs::Query<
		ecs::Read<engine::gameplay::rts::economy::ResourceBalance>,
		ecs::Write<SquadGoal>,
		ecs::Read<engine::gameplay::combat::LifeState>,
		ecs::Read<engine::gameplay::combat::Health>,
		ecs::Read<engine::gameplay::navigation::GridPosition>,
		ecs::Read<generalszh::production::ProducedUnit>,
		ecs::OptionalWrite<SquadMembership>,
		ecs::OptionalWrite<SquadGoalApplication>>;

	// Every World.Get used by the joined validation is represented here or in
	// Query.  This is access metadata, not a population scan or a second node.
	using AuxiliaryAccess = ecs::Query<
		ecs::Optional<engine::gameplay::rts::construction::BuilderAssignment>,
		ecs::Optional<engine::gameplay::rts::harvesting::HarvestState>,
		ecs::Optional<engine::gameplay::containment::ContainmentTask>,
		ecs::Optional<generalszh::capture::CaptureActorState>,
		ecs::Optional<engine::gameplay::rts::repair::ManualRepairAssignment>,
		ecs::Optional<engine::gameplay::containment::PassengerMembership>,
		ecs::Optional<generalszh::production::Producer>,
		ecs::Optional<generalszh::construction::Structure>>;

	SquadCommandSystem(ecs::World &world,
		const engine::gameplay::navigation::NavigationGrid &grid,
		SquadCommandBatch &batch) : world_(world), grid_(grid), batch_(batch)
	{
		indices_.reserve(batch_.Capacity());
		duplicate_.reserve(batch_.Capacity());
		pending_.reserve(batch_.Capacity());
	}

	void Execute(ecs::SystemContext &context)
	{
		if (!batch_.IsPublished() || batch_.Tick() != context.Tick())
			throw std::logic_error("Squad command batch tick is not published for this step");

		const auto commands = batch_.Commands();
		indices_.resize(commands.size());
		std::iota(indices_.begin(), indices_.end(), std::size_t{0});
		duplicate_.assign(commands.size(), std::uint8_t{0});
		pending_.clear();
		pending_.resize(commands.size());

		MarkDuplicateSquads(commands);
		MarkDuplicateActors(commands);

		// Complete validation precedes every component write and every deferred
		// structural command.  Deferred Add/Remove is not visible through World.Get.
		for (std::size_t index = 0; index != commands.size(); ++index)
		{
			pending_[index] = Validate(index, commands[index]);
			batch_.RecordReceipt(pending_[index]);
		}

		for (std::size_t index = 0; index != commands.size(); ++index)
			if (pending_[index].status == SquadCommandStatus::Accepted)
				Apply(commands[index], context.Commands());
	}

private:
	static bool EntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
	{
		return left.index < right.index ||
			(left.index == right.index && left.generation < right.generation);
	}

	static bool IsGoal(const SquadCommand &command) noexcept
	{ return command.kind == SquadCommandKind::SetGoal; }

	static bool IsMembership(const SquadCommand &command) noexcept
	{ return command.kind == SquadCommandKind::Assign || command.kind == SquadCommandKind::Remove; }

	void MarkDuplicateSquads(const std::span<const SquadCommand> commands)
	{
		std::sort(indices_.begin(), indices_.end(), [&](const std::size_t left, const std::size_t right) {
			const auto &a = commands[left];
			const auto &b = commands[right];
			if (IsGoal(a) != IsGoal(b)) return IsGoal(a) > IsGoal(b);
			if (!IsGoal(a)) return left < right;
			if (a.squad != b.squad) return EntityLess(a.squad, b.squad);
			return left < right;
		});
		for (std::size_t first = 0; first != indices_.size();)
		{
			const auto index = indices_[first];
			if (!IsGoal(commands[index])) break;
			std::size_t last = first + 1;
			while (last != indices_.size() && IsGoal(commands[indices_[last]]) &&
				commands[indices_[last]].squad == commands[index].squad)
				++last;
			if (last - first > 1)
				for (std::size_t at = first; at != last; ++at)
					duplicate_[indices_[at]] = 1;
			first = last;
		}
	}

	void MarkDuplicateActors(const std::span<const SquadCommand> commands)
	{
		std::sort(indices_.begin(), indices_.end(), [&](const std::size_t left, const std::size_t right) {
			const auto &a = commands[left];
			const auto &b = commands[right];
			if (IsMembership(a) != IsMembership(b)) return IsMembership(a) > IsMembership(b);
			if (!IsMembership(a)) return left < right;
			if (a.actor != b.actor) return EntityLess(a.actor, b.actor);
			return left < right;
		});
		for (std::size_t first = 0; first != indices_.size();)
		{
			const auto index = indices_[first];
			if (!IsMembership(commands[index])) break;
			std::size_t last = first + 1;
			while (last != indices_.size() && IsMembership(commands[indices_[last]]) &&
				commands[indices_[last]].actor == commands[index].actor)
				++last;
			if (last - first > 1)
				for (std::size_t at = first; at != last; ++at)
					duplicate_[indices_[at]] = 2;
			first = last;
		}
	}

	SquadCommandReceipt Validate(const std::size_t sequence, const SquadCommand &command) const
	{
		SquadCommandReceipt receipt{sequence, command.kind, SquadCommandStatus::InvalidCommand,
			command.issuer, command.squad, command.actor, 0, {}};
		if (duplicate_[sequence] == 1)
		{
			receipt.status = SquadCommandStatus::DuplicateSquad;
			return receipt;
		}
		if (duplicate_[sequence] == 2)
		{
			receipt.status = SquadCommandStatus::DuplicateActor;
			return receipt;
		}

		const auto *balance = world_.Get<engine::gameplay::rts::economy::ResourceBalance>(command.issuer);
		if (!world_.IsAlive(command.issuer) || balance == nullptr)
		{
			receipt.status = SquadCommandStatus::InvalidIssuer;
			return receipt;
		}
		const auto *squad = world_.Get<SquadGoal>(command.squad);
		if (!world_.IsAlive(command.squad) || squad == nullptr)
		{
			receipt.status = SquadCommandStatus::InvalidSquad;
			return receipt;
		}
		if (squad->owner != command.issuer)
		{
			receipt.status = SquadCommandStatus::NotOwned;
			return receipt;
		}

		switch (command.kind)
		{
		case SquadCommandKind::SetGoal:
		{
			if (command.expectedRevision != squad->revision)
			{
				receipt.status = SquadCommandStatus::StaleGoalRevision;
				return receipt;
			}
			if (!ValidGoal(*squad, command.goal))
			{
				receipt.status = command.goal.kind == engine::gameplay::rts::ai::squads::SquadGoalKind::AttackTarget
					&& !ValidTarget(command.goal.target)
					? SquadCommandStatus::InvalidTarget : SquadCommandStatus::InvalidGoal;
				return receipt;
			}
			receipt.status = SquadCommandStatus::Accepted;
			receipt.revision = engine::gameplay::rts::ai::squads::NextSquadGoalRevision(squad->revision);
			return receipt;
		}

		case SquadCommandKind::Assign:
		{
			if (command.expectedMembership.IsValid())
			{
				receipt.status = SquadCommandStatus::StaleMembership;
				return receipt;
			}
			const auto actorStatus = ValidateActor(command.actor, command.issuer);
			if (actorStatus != SquadCommandStatus::Accepted)
			{
				receipt.status = actorStatus;
				return receipt;
			}
			if (world_.Get<SquadMembership>(command.actor) != nullptr ||
				world_.Get<SquadGoalApplication>(command.actor) != nullptr)
			{
				receipt.status = SquadCommandStatus::AlreadyMember;
				return receipt;
			}
			if (TaskBusy(command.actor))
			{
				receipt.status = SquadCommandStatus::TaskBusy;
				return receipt;
			}
			receipt.status = SquadCommandStatus::Accepted;
			receipt.membership = command.squad;
			return receipt;
		}

		case SquadCommandKind::Remove:
		{
			const auto actorStatus = ValidateActor(command.actor, command.issuer);
			if (actorStatus != SquadCommandStatus::Accepted)
			{
				receipt.status = actorStatus;
				return receipt;
			}
			{
				const auto *membership = world_.Get<SquadMembership>(command.actor);
				if (membership == nullptr)
				{
					receipt.status = SquadCommandStatus::MissingMembership;
					return receipt;
				}
				if (membership->squad != command.squad ||
					membership->squad != command.expectedMembership)
				{
					receipt.status = SquadCommandStatus::StaleMembership;
					return receipt;
				}
			}
			receipt.status = SquadCommandStatus::Accepted;
			return receipt;
		}
		default:
			return receipt;
		}
	}

	bool ValidGoal(const SquadGoal &current, const engine::gameplay::rts::ai::squads::SquadGoalPayload &payload) const
	{
		const SquadGoal candidate{payload.kind, current.owner, payload.target, payload.destination,
			payload.searchRadiusCells, current.revision};
		try
		{
			engine::gameplay::rts::ai::squads::ValidateSquadGoal(candidate, grid_);
		}
		catch (const std::invalid_argument &)
		{
			return false;
		}
		return payload.kind != engine::gameplay::rts::ai::squads::SquadGoalKind::AttackTarget ||
			ValidTarget(payload.target);
	}

	bool ValidTarget(const ecs::Entity target) const
	{
		if (!world_.IsAlive(target)) return false;
		const auto *life = world_.Get<engine::gameplay::combat::LifeState>(target);
		const auto *health = world_.Get<engine::gameplay::combat::Health>(target);
		const auto *position = world_.Get<engine::gameplay::navigation::GridPosition>(target);
		// LifeState and positive Health are both required at this boundary; an
		// alive row with zero health is not a valid actor/target.
		if (life == nullptr || !life->alive || health == nullptr || health->current == 0 || position == nullptr ||
			position->cell >= grid_.Count()) return false;
		const auto owner = TargetOwner(target);
		return owner.IsValid() && world_.IsAlive(owner);
	}

	ecs::Entity TargetOwner(const ecs::Entity target) const
	{
		// This precedence mirrors TargetIndex::Rebuild: produced unit, then
		// producer, then structure. The first present typed owner is authoritative.
		if (const auto *unit = world_.Get<generalszh::production::ProducedUnit>(target)) return unit->account;
		if (const auto *producer = world_.Get<generalszh::production::Producer>(target)) return producer->account;
		if (const auto *structure = world_.Get<generalszh::construction::Structure>(target)) return structure->account;
		return {};
	}

	SquadCommandStatus ValidateActor(const ecs::Entity actor, const ecs::Entity issuer) const
	{
		if (!world_.IsAlive(actor)) return SquadCommandStatus::InvalidActor;
		const auto *unit = world_.Get<generalszh::production::ProducedUnit>(actor);
		const auto *life = world_.Get<engine::gameplay::combat::LifeState>(actor);
		const auto *health = world_.Get<engine::gameplay::combat::Health>(actor);
		const auto *position = world_.Get<engine::gameplay::navigation::GridPosition>(actor);
		if (unit == nullptr || life == nullptr || !life->alive || health == nullptr || health->current == 0 ||
			position == nullptr || position->cell >= grid_.Count())
			return SquadCommandStatus::InvalidActor;
		return unit->account == issuer ? SquadCommandStatus::Accepted : SquadCommandStatus::NotOwned;
	}

	bool TaskBusy(const ecs::Entity actor) const
	{
		if (const auto *membership = world_.Get<engine::gameplay::containment::PassengerMembership>(actor))
			if (engine::gameplay::containment::IsContained(*membership)) return true;
		if (const auto *task = world_.Get<engine::gameplay::containment::ContainmentTask>(actor))
			if (task->carrier.IsValid()) return true;
		if (const auto *assignment = world_.Get<engine::gameplay::rts::construction::BuilderAssignment>(actor))
			if (assignment->site.IsValid()) return true;
		if (const auto *harvest = world_.Get<engine::gameplay::rts::harvesting::HarvestState>(actor))
			if (harvest->phase != engine::gameplay::rts::harvesting::HarvestPhase::Idle &&
				harvest->phase != engine::gameplay::rts::harvesting::HarvestPhase::NeedsInput) return true;
		if (const auto *capture = world_.Get<generalszh::capture::CaptureActorState>(actor))
			if (generalszh::capture::IsCaptureBusy(*capture)) return true;
		if (const auto *repair = world_.Get<engine::gameplay::rts::repair::ManualRepairAssignment>(actor))
			if (repair->target.IsValid()) return true;
		return false;
	}

	void Apply(const SquadCommand &command, ecs::CommandBuffer &commands)
	{
		if (command.kind == SquadCommandKind::SetGoal)
		{
			auto *goal = world_.Get<SquadGoal>(command.squad);
			const auto next = engine::gameplay::rts::ai::squads::NextSquadGoalRevision(goal->revision);
			*goal = {command.goal.kind, goal->owner, command.goal.target, command.goal.destination,
				command.goal.searchRadiusCells, next};
			return;
		}
		if (command.kind == SquadCommandKind::Assign)
		{
			commands.Add<SquadMembership>(command.actor, SquadMembership{command.squad});
			commands.Add<SquadGoalApplication>(command.actor, SquadGoalApplication{});
			return;
		}
		commands.Remove<SquadMembership>(command.actor);
		if (world_.Get<SquadGoalApplication>(command.actor) != nullptr)
			commands.Remove<SquadGoalApplication>(command.actor);
	}

	ecs::World &world_;
	const engine::gameplay::navigation::NavigationGrid &grid_;
	SquadCommandBatch &batch_;
	std::vector<std::size_t> indices_;
	std::vector<std::uint8_t> duplicate_;
	std::vector<SquadCommandReceipt> pending_;
};

} // namespace generalszh::ai::squads

export namespace ecs
{

template<>
struct SystemTraits<generalszh::ai::squads::SquadCommandSystem>
{
	static constexpr std::string_view StableName = "games.generalszh.ai.squads.command";
	static constexpr SystemPhase Phase = SystemPhase::PreSimulation;
	static constexpr bool Batch = true;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};

} // namespace ecs
