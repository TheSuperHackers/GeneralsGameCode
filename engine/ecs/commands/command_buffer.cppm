module;

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

export module engine.ecs.commands.command_buffer;

export import engine.ecs.core.world;

export namespace ecs
{

struct CommandBufferOrder
{
	std::uint32_t phase{0};
	std::uint32_t systemOrder{0};
	std::uint32_t jobOrder{0};
	std::uint32_t chunkOrder{0};

	friend constexpr bool operator==(CommandBufferOrder left, CommandBufferOrder right) noexcept
	{
		return left.phase == right.phase && left.systemOrder == right.systemOrder &&
			left.jobOrder == right.jobOrder && left.chunkOrder == right.chunkOrder;
	}

	friend constexpr bool operator<(CommandBufferOrder left, CommandBufferOrder right) noexcept
	{
		if (left.phase != right.phase)
			return left.phase < right.phase;
		if (left.systemOrder != right.systemOrder)
			return left.systemOrder < right.systemOrder;
		if (left.jobOrder != right.jobOrder)
			return left.jobOrder < right.jobOrder;
		return left.chunkOrder < right.chunkOrder;
	}
};

enum class StructuralCommandType : std::uint8_t
{
	CreateEntity,
	DestroyEntity,
	AddComponent,
	RemoveComponent
};

enum class CommandBufferState : std::uint8_t
{
	Recording,
	Playing,
	Consumed,
	Failed
};

class DeferredEntity
{
public:
	DeferredEntity() noexcept = default;
	[[nodiscard]] bool IsValid() const noexcept { return m_index != InvalidIndex; }

private:
	static constexpr std::uint32_t InvalidIndex = (std::numeric_limits<std::uint32_t>::max)();

	DeferredEntity(std::uint64_t ownerToken, std::uint64_t epoch, std::uint32_t index) noexcept :
		m_ownerToken(ownerToken),
		m_epoch(epoch),
		m_index(index)
	{
	}

	std::uint64_t m_ownerToken{0};
	std::uint64_t m_epoch{0};
	std::uint32_t m_index{InvalidIndex};

	friend class CommandBuffer;
};

extern "C++"
{
class CommandBuffer
{
public:
	explicit CommandBuffer(CommandBufferOrder order = {}) :
		m_order(order),
		m_ownerToken(NextOwnerToken())
	{
	}

	~CommandBuffer() noexcept;

	CommandBuffer(const CommandBuffer &) = delete;
	CommandBuffer &operator=(const CommandBuffer &) = delete;
	CommandBuffer(CommandBuffer &&other) noexcept;
	CommandBuffer &operator=(CommandBuffer &&other) noexcept;

	[[nodiscard]] CommandBufferOrder Order() const noexcept { return m_order; }
	void SetOrder(CommandBufferOrder order);
	[[nodiscard]] CommandBufferState State() const noexcept { return m_state; }

	DeferredEntity Create();
	void Destroy(Entity entity);
	void Destroy(DeferredEntity entity);

	template<typename T, typename Value>
	void Add(Entity entity, Value &&value)
	{
		RecordAdd<T>(MakeTarget(entity), std::forward<Value>(value));
	}

	template<typename T, typename Value>
	void Add(DeferredEntity entity, Value &&value)
	{
		RecordAdd<T>(MakeTarget(entity), std::forward<Value>(value));
	}

	template<typename T>
	void Add(Entity entity)
	{
		RecordComponentCommand(StructuralCommandType::AddComponent,
			MakeTarget(entity), ComponentKeyFor<T>(), &typeid(T));
	}

	template<typename T>
	void Add(DeferredEntity entity)
	{
		RecordComponentCommand(StructuralCommandType::AddComponent,
			MakeTarget(entity), ComponentKeyFor<T>(), &typeid(T));
	}

	template<typename T>
	void Remove(Entity entity)
	{
		RecordComponentCommand(StructuralCommandType::RemoveComponent,
			MakeTarget(entity), ComponentKeyFor<T>(), &typeid(T));
	}

	template<typename T>
	void Remove(DeferredEntity entity)
	{
		RecordComponentCommand(StructuralCommandType::RemoveComponent,
			MakeTarget(entity), ComponentKeyFor<T>(), &typeid(T));
	}

	[[nodiscard]] Entity Resolve(DeferredEntity entity) const;
	[[nodiscard]] std::size_t Size() const noexcept { return m_commands.size(); }
	[[nodiscard]] bool Empty() const noexcept { return m_commands.empty(); }

private:
	struct PayloadAllocation
	{
		void *data{nullptr};
		std::size_t blockIndex{0};
		std::size_t previousUsed{0};
	};

	class PayloadArena
	{
		struct Block
		{
			void *data{nullptr};
			std::size_t capacity{0};
			std::size_t used{0};
			std::size_t alignment{alignof(std::max_align_t)};

			Block() = default;
			Block(void *blockData, std::size_t blockCapacity, std::size_t blockAlignment) noexcept :
				data(blockData),
				capacity(blockCapacity),
				alignment(blockAlignment)
			{
			}

			~Block() noexcept
			{
				if (data != nullptr)
					::operator delete(data, std::align_val_t(alignment));
			}

			Block(const Block &) = delete;
			Block &operator=(const Block &) = delete;

			Block(Block &&other) noexcept :
				data(other.data),
				capacity(other.capacity),
				used(other.used),
				alignment(other.alignment)
			{
				other.data = nullptr;
				other.capacity = 0;
				other.used = 0;
			}

			Block &operator=(Block &&other) noexcept
			{
				if (this == &other)
					return *this;
				if (data != nullptr)
					::operator delete(data, std::align_val_t(alignment));
				data = other.data;
				capacity = other.capacity;
				used = other.used;
				alignment = other.alignment;
				other.data = nullptr;
				other.capacity = 0;
				other.used = 0;
				return *this;
			}
		};

	public:
		PayloadAllocation Allocate(std::size_t size, std::size_t alignment);
		void Rewind(PayloadAllocation allocation) noexcept;
		void Clear() noexcept { m_blocks.clear(); }

	private:
		static std::size_t AlignUp(std::size_t value, std::size_t alignment);
		void AddBlock(std::size_t minimumCapacity, std::size_t alignment);

		std::vector<Block> m_blocks;
		std::size_t m_nextBlockCapacity{4096};
	};

	enum class TargetKind : std::uint8_t
	{
		Existing,
		Deferred
	};

	struct CommandTarget
	{
		TargetKind kind{TargetKind::Existing};
		Entity existing{};
		DeferredEntity deferred{};
	};

	struct StructuralCommand
	{
		StructuralCommandType type{StructuralCommandType::CreateEntity};
		std::uint32_t sequence{0};
		CommandTarget target{};
		ComponentKey componentKey{0};
		const std::type_info *componentType{&typeid(void)};
		void *payload{nullptr};
		void (*destroyPayload)(void *) noexcept{nullptr};
	};

	static constexpr ComponentKey InvalidComponentKey = 0;
	static std::uint64_t NextOwnerToken() noexcept;

	template<typename T>
	static constexpr ComponentKey ComponentKeyFor()
	{
		static_assert(detail::HasComponentTraits<T>,
			"Command components must specialize ecs::ComponentTraits");
		static_assert(ComponentTraits<T>::StableName.size() != 0,
			"Command components must have a non-empty stable name");
		return HashComponentKey(ComponentTraits<T>::StableName);
	}

	static CommandTarget MakeTarget(Entity entity) noexcept
	{
		return CommandTarget{TargetKind::Existing, entity, DeferredEntity{}};
	}

	CommandTarget MakeTarget(DeferredEntity entity) const;
	void ValidateDeferredTarget(const DeferredEntity &entity) const;
	void Validate(const World &world) const;
	ComponentId ResolveComponent(const StructuralCommand &command, const World &world) const;
	void Playback(World &world);
	Entity ResolveTarget(const CommandTarget &target, const std::vector<Entity> &resolved) const;

	void EnsureRecording() const;
	void Record(StructuralCommand command);
	void RecordComponentCommand(StructuralCommandType type,
		CommandTarget target,
		ComponentKey componentKey,
		const std::type_info *componentType);
	void ClearPendingCommands() noexcept;
	void FailPlayback() noexcept;

	template<typename T>
	static void DestroyPayload(void *payload) noexcept
	{
		std::destroy_at(static_cast<T *>(payload));
	}

	template<typename T, typename Value>
	void RecordAdd(CommandTarget target, Value &&value)
	{
		EnsureRecording();
		static_assert(detail::HasComponentTraits<T>,
			"Command components must specialize ecs::ComponentTraits");
		static_assert(ComponentTraits<T>::StableName.size() != 0,
			"Command components must have a non-empty stable name");
		static_assert(std::is_object_v<T> && !std::is_const_v<T> && !std::is_volatile_v<T>,
			"Command component types must be unqualified object types");
		static_assert(std::is_constructible_v<T, Value &&>,
			"Command component value must construct the requested component type");

		const PayloadAllocation allocation = m_payload.Allocate(sizeof(T), alignof(T));
		try
		{
			std::construct_at(static_cast<T *>(allocation.data), std::forward<Value>(value));
		}
		catch (...)
		{
			m_payload.Rewind(allocation);
			throw;
		}

		StructuralCommand command;
		command.type = StructuralCommandType::AddComponent;
		command.target = target;
		command.componentKey = ComponentKeyFor<T>();
		command.componentType = &typeid(T);
		command.payload = allocation.data;
		command.destroyPayload = &DestroyPayload<T>;
		try
		{
			Record(command);
		}
		catch (...)
		{
			command.destroyPayload(command.payload);
			m_payload.Rewind(allocation);
			throw;
		}
	}

	CommandBufferOrder m_order{};
	CommandBufferState m_state{CommandBufferState::Recording};
	std::uint64_t m_ownerToken{0};
	std::uint64_t m_epoch{1};
	std::uint32_t m_nextSequence{0};
	std::uint32_t m_nextTemporaryIndex{0};
	std::uint64_t m_lastEpoch{0};
	std::vector<StructuralCommand> m_commands;
	std::vector<Entity> m_lastResolved;
	PayloadArena m_payload;

	friend class World;
};
} // extern "C++"

} // namespace ecs

namespace ecs
{

extern "C++"
{
std::size_t CommandBuffer::PayloadArena::AlignUp(const std::size_t value, const std::size_t alignment)
{
	if (alignment == 0 || (alignment & (alignment - 1)) != 0)
		throw std::invalid_argument("ECS command payload alignment must be a power of two");
	const std::size_t remainder = value % alignment;
	if (remainder == 0)
		return value;
	const std::size_t padding = alignment - remainder;
	if (value > (std::numeric_limits<std::size_t>::max)() - padding)
		throw std::length_error("ECS command payload arena exhausted");
	return value + padding;
}

void CommandBuffer::PayloadArena::AddBlock(const std::size_t minimumCapacity, const std::size_t alignment)
{
	if (minimumCapacity > (std::numeric_limits<std::size_t>::max)() - (alignment - 1))
		throw std::length_error("ECS command payload is too large");

	const std::size_t capacityWithAlignment = minimumCapacity + alignment - 1;
	const std::size_t capacity = (std::max)(m_nextBlockCapacity, capacityWithAlignment);
	const std::size_t blockAlignment = (std::max)(alignment, alignof(std::max_align_t));
	void *data = ::operator new(capacity, std::align_val_t(blockAlignment));
	try
	{
		m_blocks.emplace_back(data, capacity, blockAlignment);
	}
	catch (...)
	{
		::operator delete(data, std::align_val_t(blockAlignment));
		throw;
	}

	if (m_nextBlockCapacity < 65536)
		m_nextBlockCapacity = (std::min)(std::size_t{65536}, m_nextBlockCapacity * 2);
}

CommandBuffer::PayloadAllocation CommandBuffer::PayloadArena::Allocate(
	const std::size_t size,
	const std::size_t alignment)
{
	if (size == 0)
		throw std::invalid_argument("ECS command payload cannot be empty");

	if (!m_blocks.empty())
	{
		Block &block = m_blocks.back();
		if (block.alignment >= alignment)
		{
			const std::size_t offset = AlignUp(block.used, alignment);
			if (offset <= block.capacity && size <= block.capacity - offset)
			{
				const std::size_t previousUsed = block.used;
				block.used = offset + size;
				return PayloadAllocation{
					static_cast<std::byte *>(block.data) + offset,
					static_cast<std::size_t>(m_blocks.size() - 1),
					previousUsed};
			}
		}
	}

	AddBlock(size, alignment);
	Block &block = m_blocks.back();
	const std::size_t offset = AlignUp(block.used, alignment);
	assert(offset <= block.capacity && size <= block.capacity - offset);
	block.used = offset + size;
	return PayloadAllocation{
		static_cast<std::byte *>(block.data) + offset,
		m_blocks.size() - 1,
		0};
}

void CommandBuffer::PayloadArena::Rewind(const PayloadAllocation allocation) noexcept
{
	assert(allocation.blockIndex < m_blocks.size());
	Block &block = m_blocks[allocation.blockIndex];
	block.used = allocation.previousUsed;
}

CommandBuffer::~CommandBuffer() noexcept
{
	ClearPendingCommands();
}

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept :
	m_order(other.m_order),
	m_state(other.m_state),
	m_ownerToken(other.m_ownerToken),
	m_epoch(other.m_epoch),
	m_nextSequence(other.m_nextSequence),
	m_nextTemporaryIndex(other.m_nextTemporaryIndex),
	m_lastEpoch(other.m_lastEpoch),
	m_commands(std::move(other.m_commands)),
	m_lastResolved(std::move(other.m_lastResolved)),
	m_payload(std::move(other.m_payload))
{
	other.m_nextSequence = 0;
	other.m_nextTemporaryIndex = 0;
	other.m_lastEpoch = 0;
	other.m_state = CommandBufferState::Recording;
	other.m_commands.clear();
	other.m_lastResolved.clear();
	other.m_payload.Clear();
	other.m_ownerToken = NextOwnerToken();
}

CommandBuffer &CommandBuffer::operator=(CommandBuffer &&other) noexcept
{
	if (this == &other)
		return *this;

	for (StructuralCommand &command : m_commands)
	{
		if (command.destroyPayload != nullptr)
			command.destroyPayload(command.payload);
	}

	m_order = other.m_order;
	m_state = other.m_state;
	m_ownerToken = other.m_ownerToken;
	m_epoch = other.m_epoch;
	m_nextSequence = other.m_nextSequence;
	m_nextTemporaryIndex = other.m_nextTemporaryIndex;
	m_lastEpoch = other.m_lastEpoch;
	m_commands = std::move(other.m_commands);
	m_lastResolved = std::move(other.m_lastResolved);
	m_payload = std::move(other.m_payload);

	other.m_nextSequence = 0;
	other.m_nextTemporaryIndex = 0;
	other.m_lastEpoch = 0;
	other.m_state = CommandBufferState::Recording;
	other.m_commands.clear();
	other.m_lastResolved.clear();
	other.m_payload.Clear();
	other.m_ownerToken = NextOwnerToken();
	return *this;
}

void CommandBuffer::SetOrder(const CommandBufferOrder order)
{
	EnsureRecording();
	if (!m_commands.empty() || m_nextTemporaryIndex != 0 || !m_lastResolved.empty())
		throw std::logic_error("ECS command-buffer ordering must be set before recording commands");
	m_order = order;
}

std::uint64_t CommandBuffer::NextOwnerToken() noexcept
{
	static std::atomic<std::uint64_t> nextToken{1};
	std::uint64_t token = nextToken.fetch_add(1, std::memory_order_relaxed);
	if (token == 0)
		token = nextToken.fetch_add(1, std::memory_order_relaxed);
	return token;
}

CommandBuffer::CommandTarget CommandBuffer::MakeTarget(const DeferredEntity entity) const
{
	ValidateDeferredTarget(entity);
	return CommandTarget{TargetKind::Deferred, Entity{}, entity};
}

void CommandBuffer::ValidateDeferredTarget(const DeferredEntity &entity) const
{
	if (!entity.IsValid() || entity.m_ownerToken != m_ownerToken || entity.m_epoch != m_epoch ||
		entity.m_index >= m_nextTemporaryIndex)
		throw std::logic_error("Invalid or expired ECS deferred entity handle");
}

DeferredEntity CommandBuffer::Create()
{
	EnsureRecording();
	if (m_nextTemporaryIndex == (std::numeric_limits<std::uint32_t>::max)())
		throw std::length_error("ECS command buffer temporary entity limit reached");

	const DeferredEntity entity{m_ownerToken, m_epoch, m_nextTemporaryIndex};
	++m_nextTemporaryIndex;
	StructuralCommand command;
	command.type = StructuralCommandType::CreateEntity;
	command.target = MakeTarget(entity);
	try
	{
		Record(command);
	}
	catch (...)
	{
		--m_nextTemporaryIndex;
		throw;
	}
	return entity;
}

void CommandBuffer::Destroy(const Entity entity)
{
	RecordComponentCommand(StructuralCommandType::DestroyEntity,
		MakeTarget(entity), InvalidComponentKey, &typeid(void));
}

void CommandBuffer::Destroy(const DeferredEntity entity)
{
	RecordComponentCommand(StructuralCommandType::DestroyEntity,
		MakeTarget(entity), InvalidComponentKey, &typeid(void));
}

void CommandBuffer::RecordComponentCommand(const StructuralCommandType type,
	const CommandTarget target,
	const ComponentKey componentKey,
	const std::type_info *componentType)
{
	StructuralCommand command;
	command.type = type;
	command.target = target;
	command.componentKey = componentKey;
	command.componentType = componentType;
	Record(command);
}

void CommandBuffer::ClearPendingCommands() noexcept
{
	for (StructuralCommand &command : m_commands)
	{
		if (command.destroyPayload != nullptr)
			command.destroyPayload(command.payload);
	}
	m_commands.clear();
	m_payload.Clear();
}

void CommandBuffer::FailPlayback() noexcept
{
	ClearPendingCommands();
	m_lastResolved.clear();
	m_lastEpoch = 0;
	m_nextSequence = 0;
	m_nextTemporaryIndex = 0;
	++m_epoch;
	m_state = CommandBufferState::Failed;
}

void CommandBuffer::EnsureRecording() const
{
	if (m_state != CommandBufferState::Recording)
		throw std::logic_error("ECS command buffer is no longer recordable");
}

void CommandBuffer::Record(StructuralCommand command)
{
	EnsureRecording();
	if (m_nextSequence == (std::numeric_limits<std::uint32_t>::max)())
		throw std::length_error("ECS command buffer command limit reached");
	command.sequence = m_nextSequence;
	m_commands.push_back(command);
	++m_nextSequence;
}

Entity CommandBuffer::Resolve(const DeferredEntity entity) const
{
	if (m_state != CommandBufferState::Consumed || !entity.IsValid() ||
		entity.m_ownerToken != m_ownerToken || entity.m_epoch != m_lastEpoch ||
		entity.m_index >= m_lastResolved.size())
		throw std::logic_error("ECS deferred entity has not been committed or has expired");
	return m_lastResolved[entity.m_index];
}

void CommandBuffer::Validate(const World &world) const
{
	if (!world.ComponentsFinalized())
		throw std::logic_error("ECS component registry must be finalized before command playback");

	for (const StructuralCommand &command : m_commands)
	{
		if (command.target.kind == TargetKind::Deferred)
			ValidateDeferredTarget(command.target.deferred);

		if (command.type == StructuralCommandType::AddComponent ||
			command.type == StructuralCommandType::RemoveComponent)
		{
			const ComponentId component = ResolveComponent(command, world);
			const ComponentInfo &info = world.Components().Get(component);
			if (command.type == StructuralCommandType::AddComponent)
			{
				if (command.payload == nullptr && info.constructDefault == nullptr)
					throw std::logic_error("ECS command requests default construction for a component without that operation");
				if (command.payload != nullptr && command.destroyPayload == nullptr)
					throw std::logic_error("ECS value command is missing its payload destructor");
			}
		}
	}
}

ComponentId CommandBuffer::ResolveComponent(const StructuralCommand &command, const World &world) const
{
	if (command.componentType == nullptr || *command.componentType == typeid(void))
		throw std::logic_error("ECS structural command is missing its exact component type");

	const ComponentId component = world.Components().TryGet(*command.componentType);
	if (component == InvalidComponentId)
		throw std::logic_error("ECS structural command component type was not registered");

	const ComponentInfo *info = world.Components().TryGet(component);
	if (info == nullptr || info->stableKey != command.componentKey)
		throw std::logic_error("ECS structural command component type and stable key disagree");
	return component;
}

Entity CommandBuffer::ResolveTarget(const CommandTarget &target, const std::vector<Entity> &resolved) const
{
	if (target.kind == TargetKind::Existing)
		return target.existing;

	ValidateDeferredTarget(target.deferred);
	if (target.deferred.m_index >= resolved.size() || !resolved[target.deferred.m_index].IsValid())
		throw std::logic_error("ECS deferred entity was used before its create command");
	return resolved[target.deferred.m_index];
}

void CommandBuffer::Playback(World &world)
{
	EnsureRecording();
	m_state = CommandBufferState::Playing;
	bool commandPlaybackActive = false;
	try
	{
		Validate(world);
		world.BeginCommandPlayback();
		commandPlaybackActive = true;
		std::vector<Entity> resolved(m_nextTemporaryIndex);
		for (const StructuralCommand &command : m_commands)
		{
			switch (command.type)
			{
			case StructuralCommandType::CreateEntity:
				{
					ValidateDeferredTarget(command.target.deferred);
					if (command.target.deferred.m_index >= resolved.size() || resolved[command.target.deferred.m_index].IsValid())
						throw std::logic_error("Duplicate ECS deferred entity create command");
					resolved[command.target.deferred.m_index] = world.Create<>();
					break;
				}
			case StructuralCommandType::DestroyEntity:
				world.Destroy(ResolveTarget(command.target, resolved));
				break;
			case StructuralCommandType::AddComponent:
				{
					const Entity entity = ResolveTarget(command.target, resolved);
					world.AddComponent(entity, ResolveComponent(command, world), command.payload);
					break;
				}
			case StructuralCommandType::RemoveComponent:
				world.RemoveComponent(ResolveTarget(command.target, resolved), ResolveComponent(command, world));
				break;
			}
		}

		world.EndCommandPlayback();
		commandPlaybackActive = false;
		ClearPendingCommands();
		m_lastResolved = std::move(resolved);
		m_lastEpoch = m_epoch;
		++m_epoch;
		m_nextSequence = 0;
		m_nextTemporaryIndex = 0;
		m_state = CommandBufferState::Consumed;
	}
	catch (...)
	{
		if (commandPlaybackActive)
			world.EndCommandPlayback();
		FailPlayback();
		throw;
	}
}

void World::Commit(CommandBuffer &commands)
{
	RequireComponentsFinalized();
	RequireStructuralMutationAllowed();
	commands.Playback(*this);
}

void World::Commit(std::span<CommandBuffer *> commandBuffers)
{
	RequireComponentsFinalized();
	RequireStructuralMutationAllowed();

	std::vector<CommandBuffer *> ordered(commandBuffers.begin(), commandBuffers.end());
	for (CommandBuffer *buffer : ordered)
	{
		if (buffer == nullptr)
			throw std::invalid_argument("ECS command-buffer list cannot contain null buffers");
	}

	std::sort(ordered.begin(), ordered.end(), [](const CommandBuffer *left, const CommandBuffer *right)
	{
		return left->Order() < right->Order();
	});
	for (std::size_t index = 1; index < ordered.size(); ++index)
	{
		if (ordered[index - 1]->Order() == ordered[index]->Order())
			throw std::logic_error("ECS command-buffer ordering metadata must be unique");
	}
	for (CommandBuffer *buffer : ordered)
	{
		buffer->EnsureRecording();
		try
		{
			buffer->Validate(*this);
		}
		catch (...)
		{
			buffer->FailPlayback();
			throw;
		}
	}
	for (CommandBuffer *buffer : ordered)
		buffer->Playback(*this);
}

} // extern "C++"
} // namespace ecs
