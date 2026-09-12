module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

export module engine.gameplay.rts.upgrades.object_scope.systems.object_upgrade_system;
export import engine.ecs.system.system;
export import engine.gameplay.rts.upgrades.components.upgrade_words;
export import engine.gameplay.rts.upgrades.object_scope.components.object_upgrade_scope;
export import engine.gameplay.rts.upgrades.object_scope.inputs.object_upgrade_batch;

export namespace engine::gameplay::rts::upgrades::object_scope
{
class ObjectUpgradeSystem final
{
public:
	using Query = ecs::Query<
		ecs::Read<ObjectUpgradeWordMarker>,
		ecs::Read<engine::gameplay::rts::upgrades::UpgradeWordOwner>,
		ecs::Read<engine::gameplay::rts::upgrades::UpgradeWordOrdinal>,
		ecs::Write<engine::gameplay::rts::upgrades::CompletedUpgradeWord>>;
	using TargetQuery = ecs::Query<ecs::Read<ObjectUpgradeTarget>>;
	using AuxiliaryAccess = TargetQuery;

	ObjectUpgradeSystem(ecs::World &world,
		ObjectUpgradeBatch &batch,
		const std::size_t wordRowCapacity) :
		batch_(batch),
		targets_(world),
		entityIndexCapacity_(batch.EntityIndexCapacity()),
		wordRowCapacity_(wordRowCapacity)
	{
		if (wordRowCapacity_ == 0)
			throw std::invalid_argument("Object upgrade word-row capacity must be positive");
		targetRows_.reserve(entityIndexCapacity_);
		wordRows_.reserve(wordRowCapacity_);
		orphanRows_.reserve(wordRowCapacity_);
		requestOrder_.reserve(batch.Capacity());
		requestAccepted_.reserve(batch.Capacity());
		requestRanges_.reserve(batch.Capacity());
		resultSlots_.reserve(batch.Capacity());
		orderedReceipts_.reserve(batch.Capacity());
	}

	void BeforeChunks(Query &query, ecs::SystemContext &context)
	{
		(void)context;
		if (batch_.IsPublished())
			throw std::logic_error("Object upgrade input was not reset before the next tick");

		targetRows_.clear();
		wordRows_.clear();
		orphanRows_.clear();
		requestOrder_.clear();
		requestAccepted_.clear();
		requestRanges_.clear();
		resultSlots_.clear();
		orderedReceipts_.clear();

		targets_.ForEachChunk([&](TargetQuery::Chunk chunk) {
			const auto scopes = chunk.Get<ObjectUpgradeTarget>();
			for (std::size_t row = 0; row < chunk.Count(); ++row)
			{
				const ecs::Entity target = chunk.Entities()[row];
				if (static_cast<std::size_t>(target.index) >= entityIndexCapacity_)
					throw std::length_error("Object upgrade target entity-index capacity exhausted");
				if (scopes[row].schemaHash == 0)
					throw std::logic_error("Object upgrade target has an unfrozen schema hash");
				PushBounded(targetRows_, entityIndexCapacity_,
					TargetObservation{target, scopes[row]},
					"Object upgrade target capacity exhausted");
			}
		});

		query.ForEachPreparedChunk([&](Query::Chunk chunk) {
			const auto owners = chunk.Get<engine::gameplay::rts::upgrades::UpgradeWordOwner>();
			const auto ordinals = chunk.Get<engine::gameplay::rts::upgrades::UpgradeWordOrdinal>();
			for (std::size_t row = 0; row < chunk.Count(); ++row)
			{
				PushBounded(wordRows_, wordRowCapacity_,
					WordObservation{chunk.Entities()[row], owners[row].value, ordinals[row].value},
					"Object upgrade word-row capacity exhausted");
			}
		});

		std::sort(targetRows_.begin(), targetRows_.end(),
			[](const TargetObservation &left, const TargetObservation &right) {
				return EntityLess(left.entity, right.entity);
			});
		std::sort(wordRows_.begin(), wordRows_.end(),
			[](const WordObservation &left, const WordObservation &right) {
				if (left.owner != right.owner)
					return EntityLess(left.owner, right.owner);
				if (left.ordinal != right.ordinal)
					return left.ordinal < right.ordinal;
				return EntityLess(left.word, right.word);
			});

		ValidateLiveScopes();
		for (const WordObservation &row : wordRows_)
		{
			if (FindTarget(row.owner) == nullptr)
				PushBounded(orphanRows_, wordRowCapacity_, row.word,
					"Object upgrade orphan-row capacity exhausted");
		}
		std::sort(orphanRows_.begin(), orphanRows_.end(), EntityLess);

		const auto requests = batch_.Requests();
		resultSlots_.resize(requests.size());
		requestAccepted_.resize(requests.size(), 0);
		for (std::size_t index = 0; index < requests.size(); ++index)
			requestOrder_.push_back(index);
		std::sort(requestOrder_.begin(), requestOrder_.end(), [&](const std::size_t left,
			const std::size_t right) {
			const auto &lhs = requests[left];
			const auto &rhs = requests[right];
			if (lhs.target != rhs.target)
				return EntityLess(lhs.target, rhs.target);
			if (lhs.binding.wordOrdinal != rhs.binding.wordOrdinal)
				return lhs.binding.wordOrdinal < rhs.binding.wordOrdinal;
			if (lhs.producerSequence != rhs.producerSequence)
				return lhs.producerSequence < rhs.producerSequence;
			return left < right;
		});

		for (const std::size_t requestIndex : requestOrder_)
		{
			const ObjectUpgradeRequest &request = requests[requestIndex];
			// ObjectUpgradeBatch validates external requests before they enter the
			// scheduler.  This is only a local invariant check; it must disappear
			// with diagnostics in Release builds.
			assert(IsCanonicalBinding(request.binding));
			ResultSlot &slot = resultSlots_[requestIndex];
			slot.written = false;
			const TargetObservation *target = FindTarget(request.target);
			if (target == nullptr)
			{
				slot.receipt = MakeReceipt(request, ecs::Entity{},
					ObjectUpgradeTransition::RejectedStaleTarget);
				slot.written = true;
				continue;
			}
			if (request.binding.schemaHash != target->scope.schemaHash)
				throw std::invalid_argument("Object upgrade request schema is stale for its target");
			if (request.binding.wordOrdinal >= target->scope.wordCount)
				throw std::invalid_argument("Object upgrade request ordinal exceeds target word scope");
			const WordObservation *word = FindWord(request.target, request.binding.wordOrdinal);
			if (word == nullptr)
				throw std::logic_error("Object upgrade live scope is missing a requested word row");
			requestAccepted_[requestIndex] = 1;
		}

		std::size_t orderedPosition = 0;
		while (orderedPosition < requestOrder_.size())
		{
			const std::size_t firstPosition = orderedPosition;
			const std::size_t firstIndex = requestOrder_[orderedPosition];
			if (requestAccepted_[firstIndex] == 0)
			{
				++orderedPosition;
				continue;
			}
			const ObjectUpgradeRequest &first = requests[firstIndex];
			++orderedPosition;
			while (orderedPosition < requestOrder_.size())
			{
				const std::size_t nextIndex = requestOrder_[orderedPosition];
				if (requestAccepted_[nextIndex] == 0)
					break;
				const ObjectUpgradeRequest &next = requests[nextIndex];
				if (next.target != first.target ||
					next.binding.wordOrdinal != first.binding.wordOrdinal)
					break;
				++orderedPosition;
			}
			PushBounded(requestRanges_, batch_.Capacity(),
				RequestRange{first.target, first.binding.wordOrdinal,
					firstPosition, orderedPosition},
				"Object upgrade request-range capacity exhausted");
		}
	}

	void Execute(Query::Chunk chunk, ecs::SystemContext &)
	{
		const auto markers = chunk.Get<ObjectUpgradeWordMarker>();
		const auto owners = chunk.Get<engine::gameplay::rts::upgrades::UpgradeWordOwner>();
		const auto ordinals = chunk.Get<engine::gameplay::rts::upgrades::UpgradeWordOrdinal>();
		auto completed = chunk.Get<engine::gameplay::rts::upgrades::CompletedUpgradeWord>();
		(void)markers;
		const auto requests = batch_.Requests();
		for (std::size_t row = 0; row < chunk.Count(); ++row)
		{
			const RequestRange *range = FindRequestRange(owners[row].value, ordinals[row].value);
			if (range == nullptr)
				continue;
			for (std::size_t position = range->first; position < range->last; ++position)
			{
				const std::size_t requestIndex = requestOrder_[position];
				const ObjectUpgradeRequest &request = requests[requestIndex];
				ResultSlot &slot = resultSlots_[requestIndex];
				assert(!slot.written);
				ObjectUpgradeTransition transition{};
				if (request.operation == ObjectUpgradeOperation::Grant)
				{
					if ((completed[row].value & request.binding.bitMask) != 0)
						transition = ObjectUpgradeTransition::AlreadyGranted;
					else
					{
						completed[row].value |= request.binding.bitMask;
						transition = ObjectUpgradeTransition::Granted;
					}
				}
				else if (request.operation == ObjectUpgradeOperation::Remove)
				{
					if ((completed[row].value & request.binding.bitMask) == 0)
						transition = ObjectUpgradeTransition::AlreadyAbsent;
					else
					{
						completed[row].value &= ~request.binding.bitMask;
						transition = ObjectUpgradeTransition::Removed;
					}
				}
				else
				{
					// Append is the external validation boundary.  No invalid operation
					// can reach this branch through the injected batch.
					assert(request.operation == ObjectUpgradeOperation::Remove);
				}
				slot.receipt = MakeReceipt(request, chunk.Entities()[row], transition);
				slot.written = true;
			}
		}
	}

	void AfterChunks(Query &, ecs::SystemContext &context)
	{
		for (const ResultSlot &slot : resultSlots_)
			if (!slot.written)
				throw std::logic_error("Object upgrade request did not receive a deterministic result");

		for (const std::size_t requestIndex : requestOrder_)
			PushBounded(orderedReceipts_, batch_.Capacity(), resultSlots_[requestIndex].receipt,
				"Object upgrade receipt capacity exhausted");
		batch_.Publish(orderedReceipts_);

		for (const ecs::Entity orphan : orphanRows_)
			context.Commands().Destroy(orphan);
	}

private:
	struct TargetObservation
	{
		ecs::Entity entity{};
		ObjectUpgradeTarget scope{};
	};

	struct WordObservation
	{
		ecs::Entity word{};
		ecs::Entity owner{};
		std::uint32_t ordinal{};
	};

	struct RequestRange
	{
		ecs::Entity owner{};
		std::uint32_t ordinal{};
		std::size_t first{};
		std::size_t last{};
	};

	struct ResultSlot
	{
		ObjectUpgradeReceipt receipt{};
		bool written{false};
	};

	static bool EntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
	{
		return left.index < right.index ||
			(left.index == right.index && left.generation < right.generation);
	}

	static ObjectUpgradeReceipt MakeReceipt(const ObjectUpgradeRequest &request,
		const ecs::Entity word,
		const ObjectUpgradeTransition transition) noexcept
	{
		return {request.target, word, request.binding, request.operation,
			request.producerSequence, transition};
	}

	template<typename T>
	static void PushBounded(std::vector<T> &values,
		const std::size_t limit,
		T value,
		const char *message)
	{
		if (values.size() >= limit)
			throw std::length_error(message);
		values.push_back(std::move(value));
	}

	const TargetObservation *FindTarget(const ecs::Entity entity) const noexcept
	{
		const auto found = std::lower_bound(targetRows_.begin(), targetRows_.end(), entity,
			[](const TargetObservation &row, const ecs::Entity value) {
				return EntityLess(row.entity, value);
			});
		return found != targetRows_.end() && found->entity == entity ? &*found : nullptr;
	}

	const WordObservation *FindWord(const ecs::Entity owner,
		const std::uint32_t ordinal) const noexcept
	{
		const auto found = std::lower_bound(wordRows_.begin(), wordRows_.end(),
			std::pair{owner, ordinal}, [](const WordObservation &row,
			const std::pair<ecs::Entity, std::uint32_t> value) {
				if (row.owner != value.first)
					return EntityLess(row.owner, value.first);
				return row.ordinal < value.second;
			});
		return found != wordRows_.end() && found->owner == owner &&
			found->ordinal == ordinal ? &*found : nullptr;
	}

	const WordObservation *FindFirstWord(const ecs::Entity owner) const noexcept
	{
		const auto found = std::lower_bound(wordRows_.begin(), wordRows_.end(),
			std::pair{owner, std::uint32_t{0}}, [](const WordObservation &row,
			const std::pair<ecs::Entity, std::uint32_t> value) {
			if (row.owner != value.first)
				return EntityLess(row.owner, value.first);
			return row.ordinal < value.second;
		});
		return found != wordRows_.end() && found->owner == owner ? &*found : nullptr;
	}

	const RequestRange *FindRequestRange(const ecs::Entity owner,
		const std::uint32_t ordinal) const noexcept
	{
		const auto found = std::lower_bound(requestRanges_.begin(), requestRanges_.end(),
			std::pair{owner, ordinal}, [](const RequestRange &range,
			const std::pair<ecs::Entity, std::uint32_t> value) {
				if (range.owner != value.first)
					return EntityLess(range.owner, value.first);
				return range.ordinal < value.second;
			});
		return found != requestRanges_.end() && found->owner == owner &&
			found->ordinal == ordinal ? &*found : nullptr;
	}

	void ValidateLiveScopes() const
	{
		for (const TargetObservation &target : targetRows_)
		{
			const WordObservation *first = FindFirstWord(target.entity);
			const std::size_t firstIndex = first == nullptr
				? wordRows_.size()
				: static_cast<std::size_t>(first - wordRows_.data());
			std::size_t endIndex = firstIndex;
			while (endIndex < wordRows_.size() && wordRows_[endIndex].owner == target.entity)
				++endIndex;
			const std::size_t count = endIndex - firstIndex;
			if (count != target.scope.wordCount)
				throw std::logic_error("Object upgrade live scope has an incomplete word-row set");
			if (count != 0)
			{
				for (std::size_t offset = 0; offset < count; ++offset)
					if (wordRows_[firstIndex + offset].ordinal != offset)
						throw std::logic_error("Object upgrade live scope has duplicate or malformed word ordinals");
			}
		}
	}

	ObjectUpgradeBatch &batch_;
	TargetQuery targets_;
	std::size_t entityIndexCapacity_;
	std::size_t wordRowCapacity_;
	std::vector<TargetObservation> targetRows_;
	std::vector<WordObservation> wordRows_;
	std::vector<ecs::Entity> orphanRows_;
	std::vector<std::size_t> requestOrder_;
	std::vector<std::uint8_t> requestAccepted_;
	std::vector<RequestRange> requestRanges_;
	std::vector<ResultSlot> resultSlots_;
	std::vector<ObjectUpgradeReceipt> orderedReceipts_;
};
}

export namespace ecs
{
template<>
struct SystemTraits<engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeSystem>
{
	static constexpr std::string_view StableName =
		"engine.gameplay.rts.upgrades.object_scope.system";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};
}
