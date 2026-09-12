module;

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

export module engine.gameplay.rts.production.storage.production_queue_storage;

export import engine.ecs.core.world;
export import engine.gameplay.rts.production.components.production_queue;

export namespace engine::gameplay::rts::production
{

class ProductionQueueStorage
{
public:
	ProductionQueueStorage() = default;
	~ProductionQueueStorage() noexcept;

	ProductionQueueStorage(const ProductionQueueStorage &) = delete;
	ProductionQueueStorage &operator=(const ProductionQueueStorage &) = delete;
	ProductionQueueStorage(ProductionQueueStorage &&) = delete;
	ProductionQueueStorage &operator=(ProductionQueueStorage &&) = delete;

	ecs::Entity Create(ecs::World &world, std::uint32_t initialCapacity = 0);
	bool Destroy(ecs::World &world, ecs::Entity queue);
	void Append(ecs::World &world, ecs::Entity queue, ecs::Entity item);
	bool Remove(ecs::World &world, ecs::Entity queue, ecs::Entity item);

	bool Contains(const ecs::World &world, ecs::Entity queue, ecs::Entity item) const;
	ecs::Entity First(const ecs::World &world, ecs::Entity queue) const;
	ecs::Entity Next(const ecs::World &world, ecs::Entity queue, ecs::Entity item) const;
	std::span<const ecs::Entity> Entries(const ecs::World &world, ecs::Entity queue) const;
	std::span<const ecs::Entity> Entries(const ProductionQueue &queue) const;

	std::size_t AllocatedSlots() const noexcept { return m_arena.size(); }
	std::size_t LiveQueueCount() const noexcept { return m_liveQueueCount; }

private:
	static constexpr std::size_t FreeBinCount = 32;
	static constexpr std::uint32_t EmptyOffset =
		(std::numeric_limits<std::uint32_t>::max)();

	struct Block
	{
		std::uint32_t offset{EmptyOffset};
		std::uint32_t capacity{0};
		std::size_t bin{0};
	};

	struct Range
	{
		std::size_t offset{0};
		std::size_t count{0};
		std::size_t capacity{0};
		std::size_t bin{0};
	};

	static std::size_t RequiredBin(std::uint32_t requested);
	static std::uint32_t CapacityForBin(std::size_t bin) noexcept;
	static std::size_t ExactBin(std::uint32_t capacity);
	static std::uint32_t NextCapacity(std::uint32_t capacity);

	void RequireWorld(const ecs::World &world) const;
	void BeginStructuralMutation(ecs::World &world) const;
	void BindWorld(ecs::World &world);
	void RequireOwnedQueue(ecs::Entity queue) const;

	void ReserveFreeBucket(std::size_t bin);
	Block AcquireBlock(std::uint32_t requested);
	void ReleaseBlock(Block block) noexcept;
	Range ValidateRange(const ProductionQueue &queue) const;

	ProductionQueue *RequireQueue(ecs::World &world, ecs::Entity queue) const;
	const ProductionQueue *FindQueue(const ecs::World &world, ecs::Entity queue) const;

	std::vector<ecs::Entity> m_arena;
	std::array<std::vector<std::uint32_t>, FreeBinCount> m_freeBins;
	std::array<std::size_t, FreeBinCount> m_allocatedBlockCounts{};
	std::vector<ecs::EntityGeneration> m_queueGenerations;
	ecs::World *m_world{nullptr};
	std::size_t m_liveQueueCount{0};
};

ProductionQueueStorage::~ProductionQueueStorage() noexcept
{
	if (m_liveQueueCount != 0)
		std::terminate();
}

std::size_t ProductionQueueStorage::RequiredBin(const std::uint32_t requested)
{
	if (requested == 0)
		return 0;

	std::uint32_t capacity = 1;
	for (std::size_t bin = 0; bin < FreeBinCount; ++bin)
	{
		if (capacity >= requested)
			return bin;
		if (bin + 1 == FreeBinCount)
			break;
		capacity <<= 1;
	}
	throw std::length_error("Production queue capacity exceeds the arena power-of-two limit");
}

std::uint32_t ProductionQueueStorage::CapacityForBin(const std::size_t bin) noexcept
{
	assert(bin < FreeBinCount);
	return std::uint32_t{1} << bin;
}

std::size_t ProductionQueueStorage::ExactBin(const std::uint32_t capacity)
{
	if (capacity == 0)
		throw std::logic_error("Production queue range has zero capacity with a live block");

	std::uint32_t power = 1;
	for (std::size_t bin = 0; bin < FreeBinCount; ++bin)
	{
		if (power == capacity)
			return bin;
		if (bin + 1 == FreeBinCount)
			break;
		power <<= 1;
	}
	throw std::logic_error("Production queue range capacity is not a power of two");
}

std::uint32_t ProductionQueueStorage::NextCapacity(const std::uint32_t capacity)
{
	if (capacity == 0)
		return 1;
	if (capacity > (std::numeric_limits<std::uint32_t>::max)() / 2)
		throw std::length_error("Production queue cannot grow beyond the arena capacity limit");
	return capacity * 2;
}

void ProductionQueueStorage::RequireWorld(const ecs::World &world) const
{
	if (m_liveQueueCount != 0 && m_world != &world)
		throw std::logic_error("Production queue storage is bound to another live ECS World");
}

void ProductionQueueStorage::BeginStructuralMutation(ecs::World &world) const
{
	if (world.IsScheduledExecutionActive())
		throw std::logic_error("Production queue structural mutation is forbidden during scheduled execution");
	RequireWorld(world);
}

void ProductionQueueStorage::BindWorld(ecs::World &world)
{
	if (m_liveQueueCount != 0)
	{
		RequireWorld(world);
		return;
	}
	m_world = &world;
}

void ProductionQueueStorage::ReserveFreeBucket(const std::size_t bin)
{
	if (bin >= FreeBinCount)
		throw std::logic_error("Production queue free-bin index is out of range");
	auto &bucket = m_freeBins[bin];
	const std::size_t allocated = m_allocatedBlockCounts[bin];
	if (allocated >= bucket.max_size())
		throw std::length_error("Production queue free-bin capacity is exhausted");
	const std::size_t required = allocated + 1;
	if (required > bucket.capacity())
	{
		// Reserve for every allocated block, not just currently free blocks.
		// Grow geometrically so provisioning queues does not allocate per queue.
		const std::size_t capacity = bucket.capacity();
		const std::size_t grown = capacity > bucket.max_size() / 2
			? bucket.max_size() : capacity * 2;
		bucket.reserve(grown < required ? required : grown);
	}
}

ProductionQueueStorage::Block ProductionQueueStorage::AcquireBlock(
	const std::uint32_t requested)
{
	if (requested == 0)
		return {};

	const std::size_t requestedBin = RequiredBin(requested);
	auto &bucket = m_freeBins[requestedBin];
	if (!bucket.empty())
	{
		const std::uint32_t capacity = CapacityForBin(requestedBin);
		const std::uint32_t offset = bucket.back();
		const std::size_t offsetSize = static_cast<std::size_t>(offset);
		if (offsetSize > m_arena.size() ||
			static_cast<std::size_t>(capacity) > m_arena.size() - offsetSize)
			throw std::logic_error("Production queue free-bin range is outside the arena");
		bucket.pop_back();
		return Block{offset, capacity, requestedBin};
	}

	const std::uint32_t capacity = CapacityForBin(requestedBin);
	ReserveFreeBucket(requestedBin);
	const std::size_t offset = m_arena.size();
	const std::size_t maximumSlots =
		static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)());
	if (offset > maximumSlots - static_cast<std::size_t>(capacity))
		throw std::length_error("Production queue arena offset or capacity exceeds uint32 range");
	const std::size_t end = offset + static_cast<std::size_t>(capacity);
	if (end > m_arena.max_size())
		throw std::length_error("Production queue arena exceeds vector capacity");
	m_arena.resize(end);
	++m_allocatedBlockCounts[requestedBin];
	return Block{static_cast<std::uint32_t>(offset), capacity, requestedBin};
}

void ProductionQueueStorage::ReleaseBlock(const Block block) noexcept
{
	if (block.capacity == 0)
		return;
	assert(block.bin < FreeBinCount);
	auto &bucket = m_freeBins[block.bin];
	assert(bucket.size() < bucket.capacity());
	bucket.push_back(block.offset);
}

ProductionQueueStorage::Range ProductionQueueStorage::ValidateRange(
	const ProductionQueue &queue) const
{
	if (queue.count > queue.capacity)
		throw std::logic_error("Production queue count exceeds its capacity");
	if (queue.capacity == 0)
	{
		if (queue.offset != EmptyOffset || queue.count != 0)
			throw std::logic_error("Empty production queue has an invalid arena range");
		return {};
	}
	if (queue.offset == EmptyOffset)
		throw std::logic_error("Production queue has capacity without an arena offset");

	const std::size_t offset = static_cast<std::size_t>(queue.offset);
	const std::size_t capacity = static_cast<std::size_t>(queue.capacity);
	if (offset > m_arena.size() || capacity > m_arena.size() - offset)
		throw std::logic_error("Production queue arena range is outside the storage arena");
	return Range{offset, static_cast<std::size_t>(queue.count), capacity,
		ExactBin(queue.capacity)};
}

ProductionQueue *ProductionQueueStorage::RequireQueue(ecs::World &world,
	const ecs::Entity queue) const
{
	if (!world.IsAlive(queue))
		throw std::invalid_argument("Production queue entity is stale or not alive");
	RequireOwnedQueue(queue);
	ProductionQueue *component = world.Get<ProductionQueue>(queue);
	if (component == nullptr)
		throw std::logic_error("Entity does not have a ProductionQueue component");
	return component;
}

const ProductionQueue *ProductionQueueStorage::FindQueue(const ecs::World &world,
	const ecs::Entity queue) const
{
	if (!world.IsAlive(queue))
		return nullptr;
	RequireOwnedQueue(queue);
	const ProductionQueue *component = world.Get<ProductionQueue>(queue);
	if (component == nullptr)
		throw std::logic_error("Entity does not have a ProductionQueue component");
	return component;
}

void ProductionQueueStorage::RequireOwnedQueue(const ecs::Entity queue) const
{
	if (!queue.IsValid() || queue.index >= m_queueGenerations.size() ||
		m_queueGenerations[queue.index] != queue.generation)
		throw std::logic_error("Live ECS entity is not owned by this production queue storage");
}

ecs::Entity ProductionQueueStorage::Create(ecs::World &world,
	const std::uint32_t initialCapacity)
{
	const std::uint32_t requested = initialCapacity == 0 ? 0 : CapacityForBin(RequiredBin(initialCapacity));
	if (m_liveQueueCount == (std::numeric_limits<std::size_t>::max)())
		throw std::length_error("Production queue live-count limit is exhausted");
	BeginStructuralMutation(world);
	BindWorld(world);

	Block block{};
	ecs::Entity queue{};
	bool created = false;
	try
	{
		block = AcquireBlock(requested);
		queue = world.Create<ProductionQueue>();
		created = true;
		ProductionQueue *component = world.Get<ProductionQueue>(queue);
		if (component == nullptr)
			throw std::logic_error("Created production queue is missing its component");
		*component = ProductionQueue{block.offset, 0, block.capacity};
		if (static_cast<std::size_t>(queue.index) >= m_queueGenerations.size())
			m_queueGenerations.resize(static_cast<std::size_t>(queue.index) + 1,
				ecs::Entity::InvalidGeneration);
		m_queueGenerations[queue.index] = queue.generation;
		++m_liveQueueCount;
		return queue;
	}
	catch (...)
	{
		if (created)
		{
			try
			{
				(void)world.Destroy(queue);
			}
			catch (...)
			{
				std::terminate();
			}
		}
		ReleaseBlock(block);
		if (m_liveQueueCount == 0)
			m_world = nullptr;
		throw;
	}
}

bool ProductionQueueStorage::Destroy(ecs::World &world, const ecs::Entity queue)
{
	BeginStructuralMutation(world);
	if (!world.IsAlive(queue))
		return false;

	RequireOwnedQueue(queue);
	ProductionQueue *component = world.Get<ProductionQueue>(queue);
	if (component == nullptr)
		throw std::logic_error("Entity does not have a ProductionQueue component");
	const Range range = ValidateRange(*component);
	if (range.count != 0)
		throw std::logic_error("Cannot destroy a non-empty production queue");
	if (world.Components().TryGet<ProductionQueueMember>() != ecs::InvalidComponentId)
	{
		const auto *membership = world.Get<ProductionQueueMember>(queue);
		if (membership != nullptr && membership->queue.IsValid())
			throw std::logic_error("Detach a queue entity from its parent queue before destroying it");
	}

	if (!world.Destroy(queue))
		return false;
	if (range.capacity != 0)
		ReleaseBlock(Block{static_cast<std::uint32_t>(range.offset),
			static_cast<std::uint32_t>(range.capacity), range.bin});
	m_queueGenerations[queue.index] = ecs::Entity::InvalidGeneration;
	assert(m_liveQueueCount != 0);
	--m_liveQueueCount;
	if (m_liveQueueCount == 0)
	{
		m_world = nullptr;
		m_queueGenerations.clear();
	}
	return true;
}

void ProductionQueueStorage::Append(ecs::World &world,
	const ecs::Entity queue,
	const ecs::Entity item)
{
	BeginStructuralMutation(world);
	ProductionQueue *component = RequireQueue(world, queue);
	if (queue == item)
		throw std::logic_error("Cannot append a production queue to itself");
	if (!world.IsAlive(item))
		throw std::invalid_argument("Cannot append a stale production queue entry");

	ProductionQueueMember *member = world.Get<ProductionQueueMember>(item);
	if (member == nullptr)
		throw std::logic_error("Production queue entry must already have a ProductionQueueMember component");
	if (member->queue.IsValid())
	{
		if (member->queue == queue)
			throw std::logic_error("Production queue entry is already a member of this queue");
		throw std::logic_error("Production queue entry belongs to another queue");
	}
	if (member->position != 0)
		throw std::logic_error("Unqueued production queue entry has a nonzero position");

	const Range range = ValidateRange(*component);
	if (range.count == range.capacity)
	{
		const Block replacement = AcquireBlock(NextCapacity(component->capacity));
		for (std::size_t index = 0; index < range.count; ++index)
			m_arena[static_cast<std::size_t>(replacement.offset) + index] =
				m_arena[range.offset + index];
		m_arena[static_cast<std::size_t>(replacement.offset) + range.count] = item;
		*component = ProductionQueue{replacement.offset,
			static_cast<std::uint32_t>(range.count + 1), replacement.capacity};
		member->queue = queue;
		member->position = static_cast<std::uint32_t>(range.count);
		ReleaseBlock(Block{static_cast<std::uint32_t>(range.offset),
			static_cast<std::uint32_t>(range.capacity), range.bin});
		return;
	}

	const std::size_t itemOffset = range.offset + range.count;
	m_arena[itemOffset] = item;
	component->count = static_cast<std::uint32_t>(range.count + 1);
	member->queue = queue;
	member->position = static_cast<std::uint32_t>(range.count);
}

bool ProductionQueueStorage::Remove(ecs::World &world,
	const ecs::Entity queue,
	const ecs::Entity item)
{
	BeginStructuralMutation(world);
	if (!world.IsAlive(queue))
		return false;
	RequireOwnedQueue(queue);
	if (!world.IsAlive(item))
		return false;

	ProductionQueue *component = world.Get<ProductionQueue>(queue);
	if (component == nullptr)
		throw std::logic_error("Entity does not have a ProductionQueue component");
	ProductionQueueMember *member = world.Get<ProductionQueueMember>(item);
	if (member == nullptr || member->queue != queue)
		return false;

	const Range range = ValidateRange(*component);
	const std::size_t position = static_cast<std::size_t>(member->position);
	if (position >= range.count || m_arena[range.offset + position] != item)
		throw std::logic_error("Production queue member position does not match its arena entry");

	for (std::size_t index = position + 1; index < range.count; ++index)
	{
		const ecs::Entity shifted = m_arena[range.offset + index];
		if (!world.IsAlive(shifted))
			throw std::logic_error("Production queue contains a stale shifted entry");
		const ProductionQueueMember *shiftedMember = world.Get<ProductionQueueMember>(shifted);
		if (shiftedMember == nullptr || shiftedMember->queue != queue ||
			shiftedMember->position != index)
			throw std::logic_error("Production queue shifted member state is inconsistent");
	}

	for (std::size_t index = position + 1; index < range.count; ++index)
	{
		const ecs::Entity shifted = m_arena[range.offset + index];
		m_arena[range.offset + index - 1] = shifted;
		ProductionQueueMember *shiftedMember = world.Get<ProductionQueueMember>(shifted);
		shiftedMember->position = static_cast<std::uint32_t>(index - 1);
	}
	m_arena[range.offset + range.count - 1] = ecs::Entity{};
	*member = ProductionQueueMember{};
	component->count = static_cast<std::uint32_t>(range.count - 1);
	return true;
}

bool ProductionQueueStorage::Contains(const ecs::World &world,
	const ecs::Entity queue,
	const ecs::Entity item) const
{
	RequireWorld(world);
	const ProductionQueue *component = FindQueue(world, queue);
	if (component == nullptr || !world.IsAlive(item))
		return false;
	const ProductionQueueMember *member = world.Get<ProductionQueueMember>(item);
	if (member == nullptr || member->queue != queue)
		return false;
	const Range range = ValidateRange(*component);
	const std::size_t position = static_cast<std::size_t>(member->position);
	if (position >= range.count)
		throw std::logic_error("Production queue member position is outside its queue");
	if (m_arena[range.offset + position] != item)
		throw std::logic_error("Production queue member claim does not match its arena entry");
	return true;
}

ecs::Entity ProductionQueueStorage::First(const ecs::World &world,
	const ecs::Entity queue) const
{
	RequireWorld(world);
	const ProductionQueue *component = FindQueue(world, queue);
	if (component == nullptr)
		return {};
	(void)ValidateRange(*component);
	const auto entries = Entries(*component);
	return entries.empty() ? ecs::Entity{} : entries.front();
}

ecs::Entity ProductionQueueStorage::Next(const ecs::World &world,
	const ecs::Entity queue,
	const ecs::Entity item) const
{
	RequireWorld(world);
	const ProductionQueue *component = FindQueue(world, queue);
	if (component == nullptr || !world.IsAlive(item))
		return {};
	const ProductionQueueMember *member = world.Get<ProductionQueueMember>(item);
	if (member == nullptr || member->queue != queue)
		return {};
	const Range range = ValidateRange(*component);
	const std::size_t position = static_cast<std::size_t>(member->position);
	if (position >= range.count || m_arena[range.offset + position] != item)
		throw std::logic_error("Production queue member position does not match its arena entry");
	return position + 1 < range.count ? m_arena[range.offset + position + 1] : ecs::Entity{};
}

std::span<const ecs::Entity> ProductionQueueStorage::Entries(const ecs::World &world,
	const ecs::Entity queue) const
{
	RequireWorld(world);
	const ProductionQueue *component = FindQueue(world, queue);
	if (component == nullptr)
		return {};
	(void)ValidateRange(*component);
	return Entries(*component);
}

std::span<const ecs::Entity> ProductionQueueStorage::Entries(
	const ProductionQueue &queue) const
{
	if (queue.count == 0)
		return {};

	const std::size_t offset = static_cast<std::size_t>(queue.offset);
	assert(queue.capacity != 0);
	assert(queue.count <= queue.capacity);
	assert(queue.offset != EmptyOffset);
	assert(offset <= m_arena.size() &&
		static_cast<std::size_t>(queue.capacity) <= m_arena.size() - offset);
	return std::span<const ecs::Entity>(m_arena.data() + offset, queue.count);
}

} // namespace engine::gameplay::rts::production
