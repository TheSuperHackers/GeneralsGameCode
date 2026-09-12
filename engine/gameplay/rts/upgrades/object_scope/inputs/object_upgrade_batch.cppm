module;

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

export module engine.gameplay.rts.upgrades.object_scope.inputs.object_upgrade_batch;
export import engine.ecs.core.entity;

export namespace engine::gameplay::rts::upgrades::object_scope
{
enum class ObjectUpgradeOperation : std::uint8_t
{
	Grant,
	Remove
};

enum class ObjectUpgradeTransition : std::uint8_t
{
	Granted,
	Removed,
	AlreadyGranted,
	AlreadyAbsent,
	RejectedStaleTarget
};

struct ObjectUpgradeBinding
{
	std::uint32_t definitionId{};
	std::uint32_t wordOrdinal{};
	std::uint64_t bitMask{};
	std::uint64_t schemaHash{};

	friend constexpr bool operator==(const ObjectUpgradeBinding &,
		const ObjectUpgradeBinding &) noexcept = default;
};

// The dense definition ID is canonical: one definition owns one bit in the
// frozen catalog layout.  This helper deliberately checks the ordinal and bit
// relationship as well as the nonzero/single-bit shape.
constexpr bool IsCanonicalBinding(const ObjectUpgradeBinding &binding) noexcept
{
	if (binding.schemaHash == 0 || binding.bitMask == 0)
		return false;
	const std::uint32_t expectedOrdinal = binding.definitionId / 64u;
	const std::uint64_t expectedBit =
		std::uint64_t{1} << (binding.definitionId % 64u);
	return binding.wordOrdinal == expectedOrdinal && binding.bitMask == expectedBit;
}

inline void ValidateCanonicalBinding(const ObjectUpgradeBinding &binding)
{
	if (!IsCanonicalBinding(binding))
		throw std::invalid_argument(
			"Object upgrade binding does not match its canonical definition word, bit, or schema");
}

struct ObjectUpgradeRequest
{
	ecs::Entity target{};
	ObjectUpgradeBinding binding{};
	ObjectUpgradeOperation operation{ObjectUpgradeOperation::Grant};
	std::uint64_t producerSequence{};
};

struct ObjectUpgradeReceipt
{
	ecs::Entity target{};
	ecs::Entity word{};
	ObjectUpgradeBinding binding{};
	ObjectUpgradeOperation operation{ObjectUpgradeOperation::Grant};
	std::uint64_t producerSequence{};
	ObjectUpgradeTransition transition{ObjectUpgradeTransition::RejectedStaleTarget};

	friend constexpr bool operator==(const ObjectUpgradeReceipt &,
		const ObjectUpgradeReceipt &) noexcept = default;
};

class ObjectUpgradeBatch final
{
public:
	ObjectUpgradeBatch(const std::size_t entityIndexCapacity,
		const std::size_t requestCapacity) :
		m_entityIndexCapacity(entityIndexCapacity),
		m_requestCapacity(requestCapacity)
	{
		if (m_entityIndexCapacity == 0)
			throw std::invalid_argument("Object upgrade entity capacity must be positive");
		if (m_requestCapacity == 0)
			throw std::invalid_argument("Object upgrade request capacity must be positive");
		m_requests.reserve(m_requestCapacity);
		m_receipts.reserve(m_requestCapacity);
	}

	ObjectUpgradeBatch(const ObjectUpgradeBatch &) = delete;
	ObjectUpgradeBatch &operator=(const ObjectUpgradeBatch &) = delete;

	[[nodiscard]] std::size_t EntityIndexCapacity() const noexcept
	{
		return m_entityIndexCapacity;
	}

	[[nodiscard]] std::size_t Capacity() const noexcept { return m_requestCapacity; }

	// The composition root starts a new fixed-step input boundary explicitly.
	// This does not allocate and does not publish an implicit result.
	void Reset() noexcept
	{
		m_requests.clear();
		m_receipts.clear();
		m_published = false;
	}

	void Append(const ObjectUpgradeRequest &request)
	{
		if (m_published)
			throw std::logic_error("Cannot append object upgrade input after publication");
		if (!request.target.IsValid() ||
			static_cast<std::size_t>(request.target.index) >= m_entityIndexCapacity)
			throw std::invalid_argument("Object upgrade request target is outside its entity capacity");
		ValidateCanonicalBinding(request.binding);
		if (request.operation != ObjectUpgradeOperation::Grant &&
			request.operation != ObjectUpgradeOperation::Remove)
			throw std::invalid_argument("Object upgrade request operation is invalid");
		if (m_requests.size() >= m_requestCapacity)
			throw std::length_error("Object upgrade input capacity exhausted");
		m_requests.push_back(request);
	}

	[[nodiscard]] std::span<const ObjectUpgradeRequest> Requests() const noexcept
	{
		return m_requests;
	}

	[[nodiscard]] bool IsPublished() const noexcept { return m_published; }

	void Publish(std::span<const ObjectUpgradeReceipt> receipts)
	{
		if (m_published)
			throw std::logic_error("Object upgrade results were already published");
		if (receipts.size() > m_requestCapacity)
			throw std::length_error("Object upgrade receipt capacity exhausted");
		m_receipts.assign(receipts.begin(), receipts.end());
		m_published = true;
	}

	[[nodiscard]] std::span<const ObjectUpgradeReceipt> Receipts() const
	{
		if (!m_published)
			throw std::logic_error("Object upgrade results are not published");
		return m_receipts;
	}

private:
	std::size_t m_entityIndexCapacity;
	std::size_t m_requestCapacity;
	std::vector<ObjectUpgradeRequest> m_requests;
	std::vector<ObjectUpgradeReceipt> m_receipts;
	bool m_published{false};
};
}
