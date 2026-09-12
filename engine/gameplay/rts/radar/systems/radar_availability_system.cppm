module;

#include <algorithm>
#include <array>
#include <cassert>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

export module engine.gameplay.rts.radar.systems.radar_availability_system;
export import engine.ecs.system.system;
export import engine.events.storage.event_batch;
export import engine.gameplay.rts.radar.algorithms.radar_availability;
export import engine.gameplay.rts.radar.components.radar_provider_contribution;
export import engine.gameplay.rts.radar.events.radar_availability_transition;

export namespace engine::gameplay::rts::radar
{
struct RadarAvailabilityLimits final
{
	std::size_t accountCount{};
	std::size_t providerCount{};
	std::size_t wordCount{};
	std::size_t transitionCount{};
};

struct RadarWordRecord final
{
	ecs::Entity owner{};
	std::uint32_t ordinal{};
	std::uint64_t completed{};
};

struct RadarProviderProjection final
{
	ecs::Entity account{};
	bool rawUpgradeCompleted{};
	bool lifecycleEligible{};
	bool resistant{};
};

inline bool RadarEntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
{
	return std::tie(left.index, left.generation) < std::tie(right.index, right.generation);
}

inline bool RadarEntityEqual(const ecs::Entity left, const ecs::Entity right) noexcept
{
	return left.index == right.index && left.generation == right.generation;
}

class RadarWordIndex final
{
public:
	explicit RadarWordIndex(const std::size_t logicalCapacity) : m_limit(logicalCapacity)
	{
		m_rows.reserve(logicalCapacity);
	}

	void Clear() noexcept
	{
		m_rows.clear();
		m_finalized = false;
	}

	void Add(const RadarWordRecord record)
	{
		if (!record.owner.IsValid())
			throw std::invalid_argument("Radar object-word row has an invalid owner");
		if (m_rows.size() >= m_limit)
			throw std::length_error("Radar object-word capacity exhausted");
		m_rows.push_back(record);
	}

	void Finalize()
	{
		std::sort(m_rows.begin(), m_rows.end(), [](const RadarWordRecord &left,
			const RadarWordRecord &right) noexcept {
			if (RadarEntityLess(left.owner, right.owner)) return true;
			if (RadarEntityLess(right.owner, left.owner)) return false;
			return left.ordinal < right.ordinal;
		});
		for (std::size_t index = 1; index < m_rows.size(); ++index)
			if (RadarEntityEqual(m_rows[index - 1].owner, m_rows[index].owner) &&
				m_rows[index - 1].ordinal == m_rows[index].ordinal)
				throw std::logic_error("Radar object-word index contains a duplicate owner ordinal");
		m_finalized = true;
	}

	[[nodiscard]] const RadarWordRecord *Find(const ecs::Entity owner,
		const std::uint32_t ordinal) const noexcept
	{
		if (!m_finalized) return nullptr;
		const auto found = std::lower_bound(m_rows.begin(), m_rows.end(),
			std::pair{owner, ordinal}, [](const RadarWordRecord &row,
			const std::pair<ecs::Entity, std::uint32_t> key) noexcept {
			if (RadarEntityLess(row.owner, key.first)) return true;
			if (RadarEntityLess(key.first, row.owner)) return false;
			return row.ordinal < key.second;
		});
		return found != m_rows.end() && RadarEntityEqual(found->owner, owner) &&
			found->ordinal == ordinal ? &*found : nullptr;
	}

	[[nodiscard]] std::size_t Size() const noexcept { return m_rows.size(); }

private:
	std::size_t m_limit{};
	std::vector<RadarWordRecord> m_rows;
	bool m_finalized{};
};

// ProviderPolicy is deliberately a typed data adapter, not an execution owner.
// It supplies ProviderQuery, AccountQuery, WordQuery, AuxiliaryAccess and Grant;
// ExtractWord, Project and AccountFor are pure row projections. The system owns
// all query iteration, grant/contribution writes, reduction and publication.
template<typename ProviderPolicy>
class RadarAvailabilitySystemT final
{
public:
	using Query = typename ProviderPolicy::ProviderQuery;
	using AccountQuery = typename ProviderPolicy::AccountQuery;
	using WordQuery = typename ProviderPolicy::WordQuery;
	using AuxiliaryAccess = typename ProviderPolicy::AuxiliaryAccess;
	using Grant = typename ProviderPolicy::Grant;

	RadarAvailabilitySystemT(ecs::World &world, const ProviderPolicy &policy,
		const RadarAvailabilityLimits limits,
		engine::events::RecordedBatch<RadarAvailabilityTransition> &recorded,
		engine::events::PublishedBatch<RadarAvailabilityTransition> &published) :
		policy_(policy), limits_(limits), accounts_(world), words_(world),
		wordIndex_(limits.wordCount), recorded_(recorded), published_(published)
	{
		if (limits_.providerCount > static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)()))
			throw std::length_error("Radar provider capacity exceeds signed count range");
		if (recorded.Capacity() < limits_.transitionCount ||
			published.Capacity() < limits_.transitionCount)
			throw std::length_error("Radar event storage is smaller than its logical transition capacity");
		accountRows_.reserve(limits_.accountCount);
		providerRows_.reserve(limits_.providerCount);
		transitions_.reserve(limits_.transitionCount);
	}

	RadarAvailabilitySystemT(const RadarAvailabilitySystemT &) = delete;
	RadarAvailabilitySystemT &operator=(const RadarAvailabilitySystemT &) = delete;

	void BeforeChunks(Query &query, ecs::SystemContext &context)
	{
		if (published_.IsPublished())
			throw std::logic_error("Radar transition publication must be released before the next tick");
		if (recorded_.State() == engine::events::RecordingState::Recording ||
			recorded_.State() == engine::events::RecordingState::Sealed ||
			recorded_.State() == engine::events::RecordingState::Failed)
			throw std::logic_error("Radar transition recording was not reclaimed");

		accountRows_.clear();
		providerRows_.clear();
		transitions_.clear();
		wordIndex_.Clear();

		std::size_t providerCount{};
		query.ForEachPreparedChunk([&](Query::Chunk chunk) {
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				if (providerCount >= limits_.providerCount)
					throw std::length_error("Radar provider capacity exhausted");
				++providerCount;
			}
		});

		words_.PrepareChunks();
		words_.ForEachPreparedChunk([this](WordQuery::Chunk chunk) {
			for (std::size_t row = 0; row != chunk.Count(); ++row)
				wordIndex_.Add(policy_.ExtractWord(chunk, row));
		});
		wordIndex_.Finalize();

		accounts_.PrepareChunks();
		accounts_.ForEachPreparedChunk([this](AccountQuery::Chunk chunk) {
			auto states = chunk.template Get<RadarAvailability>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				if (accountRows_.size() >= limits_.accountCount)
					throw std::length_error("Radar account capacity exhausted");
				accountRows_.push_back(AccountRow{
					chunk.Entities()[row], &states[row], policy_.AccountSuppressed(chunk, row), 0, 0});
			}
		});
		std::sort(accountRows_.begin(), accountRows_.end(), [](const AccountRow &left,
			const AccountRow &right) noexcept { return RadarEntityLess(left.account, right.account); });
		for (std::size_t index = 1; index < accountRows_.size(); ++index)
			if (accountRows_[index - 1].account.index == accountRows_[index].account.index)
				throw std::logic_error("Radar account index has multiple live generations");

		const auto boundary = engine::events::BatchBoundary{
			context.Tick(), static_cast<std::uint32_t>(context.Phase())};
		recorded_.Begin({boundary, context.Id(), 0, 0});
	}

	void Execute(Query::Chunk chunk, ecs::SystemContext &) const
	{
		auto grants = chunk.template Get<Grant>();
		auto contributions = chunk.template Get<RadarProviderContribution>();
		for (std::size_t row = 0; row != chunk.Count(); ++row)
		{
			const RadarProviderProjection projection = policy_.Project(chunk, row, wordIndex_);
			if (!grants[row].granted && projection.rawUpgradeCompleted)
				grants[row].granted = true;

			const bool ownerExists = FindAccount(projection.account).has_value();
			const bool active = grants[row].granted && projection.lifecycleEligible && ownerExists;
			contributions[row] = RadarProviderContribution{
				active ? std::int32_t{1} : std::int32_t{0},
				active && projection.resistant ? std::int32_t{1} : std::int32_t{0}};
		}
	}

	void AfterChunks(Query &query, ecs::SystemContext &)
	{
		providerRows_.clear();
		for (AccountRow &account : accountRows_)
		{
			account.producers = 0;
			account.resistantProducers = 0;
		}

		query.ForEachPreparedChunk([this](Query::Chunk chunk) {
			const auto contributions = chunk.template Get<RadarProviderContribution>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				if (providerRows_.size() >= limits_.providerCount)
					throw std::length_error("Radar provider reduction capacity exhausted");
				providerRows_.push_back(ProviderRow{
					chunk.Entities()[row], policy_.AccountFor(chunk, row), contributions[row]});
			}
		});
		std::sort(providerRows_.begin(), providerRows_.end(), [](const ProviderRow &left,
			const ProviderRow &right) noexcept {
			if (RadarEntityLess(left.account, right.account)) return true;
			if (RadarEntityLess(right.account, left.account)) return false;
			return RadarEntityLess(left.provider, right.provider);
		});

		for (const ProviderRow &provider : providerRows_)
		{
			const auto accountIndex = FindAccount(provider.account);
			if (!accountIndex.has_value()) continue; // stale owner: no contribution
			const auto producerBits = std::bit_cast<std::uint32_t>(provider.contribution.producers);
			const auto resistantBits = std::bit_cast<std::uint32_t>(provider.contribution.resistantProducers);
			assert(provider.contribution.producers == 0 || provider.contribution.producers == 1);
			assert(provider.contribution.resistantProducers == 0 || provider.contribution.resistantProducers == 1);
			const auto producers = static_cast<std::uint64_t>(producerBits & 1u);
			const auto resistant = static_cast<std::uint64_t>((resistantBits & producerBits) & 1u);
			assert(resistant <= producers);
			accountRows_[*accountIndex].producers += producers;
			accountRows_[*accountIndex].resistantProducers += resistant;
		}

		std::size_t transitionCount{};
		for (const AccountRow &account : accountRows_)
		{
			const bool before = HasRadar(*account.state);
			RadarAvailability next = *account.state;
			next.producers = ToCount(account.producers);
			next.resistantProducers = ToCount(account.resistantProducers);
			next.suppressed = account.suppressed;
			const bool after = HasRadar(next);
			if (before != after)
			{
				if (transitionCount >= limits_.transitionCount)
					throw std::length_error("Radar transition capacity exhausted");
				++transitionCount;
			}
		}
		for (const AccountRow &account : accountRows_)
		{
			const bool before = HasRadar(*account.state);
			RadarAvailability next = *account.state;
			next.producers = ToCount(account.producers);
			next.resistantProducers = ToCount(account.resistantProducers);
			next.suppressed = account.suppressed;
			*account.state = next;
			const bool after = HasRadar(next);
			if (before != after)
				transitions_.push_back({account.account, before, after});
		}

		for (const RadarAvailabilityTransition transition : transitions_)
			recorded_.Emplace(transition);
		recorded_.Seal();
		std::array<engine::events::RecordedBatch<RadarAvailabilityTransition> *, 1> batches{&recorded_};
		published_.Publish(recorded_.Order().boundary, batches);
	}

	[[nodiscard]] const RadarAvailabilityLimits &Limits() const noexcept { return limits_; }

private:
	struct AccountRow final
	{
		ecs::Entity account{};
		RadarAvailability *state{};
		bool suppressed{};
		std::uint64_t producers{};
		std::uint64_t resistantProducers{};
	};

	struct ProviderRow final
	{
		ecs::Entity provider{};
		ecs::Entity account{};
		RadarProviderContribution contribution{};
	};

	[[nodiscard]] std::optional<std::size_t> FindAccount(const ecs::Entity account) const noexcept
	{
		if (!account.IsValid()) return std::nullopt;
		const auto found = std::lower_bound(accountRows_.begin(), accountRows_.end(), account,
			[](const AccountRow &row, const ecs::Entity value) noexcept {
				return RadarEntityLess(row.account, value);
			});
		if (found == accountRows_.end() || !RadarEntityEqual(found->account, account)) return std::nullopt;
		return static_cast<std::size_t>(found - accountRows_.begin());
	}

	static std::int32_t ToCount(const std::uint64_t value) noexcept
	{
		assert(value <= static_cast<std::uint64_t>((std::numeric_limits<std::int32_t>::max)()));
		return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(value));
	}

	const ProviderPolicy &policy_;
	RadarAvailabilityLimits limits_;
	AccountQuery accounts_;
	WordQuery words_;
	RadarWordIndex wordIndex_;
	std::vector<AccountRow> accountRows_;
	mutable std::vector<ProviderRow> providerRows_;
	mutable std::vector<RadarAvailabilityTransition> transitions_;
	engine::events::RecordedBatch<RadarAvailabilityTransition> &recorded_;
	engine::events::PublishedBatch<RadarAvailabilityTransition> &published_;
};
} // namespace engine::gameplay::rts::radar

export namespace ecs
{
template<typename ProviderPolicy>
struct SystemTraits<engine::gameplay::rts::radar::RadarAvailabilitySystemT<ProviderPolicy>>
{
	static constexpr std::string_view StableName = ProviderPolicy::StableName;
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = typename ProviderPolicy::Before;
	using After = typename ProviderPolicy::After;
};
}
