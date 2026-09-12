module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

export module engine.ecs.core.world;

export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export import engine.ecs.storage.archetype;

export namespace ecs
{

extern "C++"
{
class CommandBuffer;
class Scheduler;
struct WorldTestAccess;
}

struct WorldConfig
{
	std::size_t chunkTargetBytes{ChunkLayout::DefaultTargetBytes};
};

// World and its cross-module collaborators use ordinary C++ linkage so that
// forward declarations and World::Commit definitions share one entity across
// named module boundaries. The public interface is still imported from here.
extern "C++"
{
class World
{
public:
	explicit World(WorldConfig config = {});
	~World() = default;

	World(const World &) = delete;
	World &operator=(const World &) = delete;

	template<typename... Components>
	Entity Create()
	{
		RequireComponentsFinalized();
		RequireStructuralMutationAllowed();

		Signature signature;
		signature.reserve(sizeof...(Components));
		(signature.push_back(GetDefaultConstructibleComponent<Components>()), ...);
		CanonicalizeSignature(signature);

		const Entity entity = AllocateEntity();
		try
		{
			Archetype &archetype = GetOrCreateArchetype(signature);
			const Archetype::Slot slot = archetype.AddEntity(entity);
			SetLocation(entity, EntityLocation{&archetype, slot.chunk, slot.row});
		}
		catch (...)
		{
			ReleaseUnconstructedEntity(entity);
			throw;
		}
		return entity;
	}

	template<typename T>
	ComponentId RegisterComponent()
	{
		return m_components.Register<T>();
	}

	void FinalizeComponents()
	{
		m_components.Finalize();
	}

	bool ComponentsFinalized() const noexcept
	{
		return m_components.IsFrozen();
	}

	// Diagnostic/preflight query. The simulation owner still serializes scheduler
	// launch and external lifecycle operations; this is not an exclusion lock.
	bool IsScheduledExecutionActive() const noexcept { return m_scheduledExecutionActive; }

	void Commit(CommandBuffer &commands);
	void Commit(std::span<CommandBuffer *> commandBuffers);

	bool IsAlive(Entity entity) const noexcept;
	bool Destroy(Entity entity);

	template<typename T>
	bool Has(Entity entity) const
	{
		RequireComponentsFinalized();
		if (!IsAlive(entity))
			return false;
		const ComponentId component = GetRegisteredComponent<T>();
		return m_records[entity.index].location.archetype->Has(component);
	}

	template<typename T>
	bool Add(Entity entity)
	{
		RequireComponentsFinalized();
		RequireStructuralMutationAllowed();
		if (!IsAlive(entity))
			return false;
		return AddComponent(entity, GetRegisteredComponent<T>(), nullptr);
	}

	template<typename T>
	bool Remove(Entity entity)
	{
		RequireComponentsFinalized();
		RequireStructuralMutationAllowed();
		if (!IsAlive(entity))
			return false;
		return RemoveComponent(entity, GetRegisteredComponent<T>());
	}

	template<typename T>
	T *Get(Entity entity)
	{
		RequireComponentsFinalized();
		const ComponentId component = GetRegisteredComponent<T>();
		if (!IsAlive(entity))
			return nullptr;

		EntityRecord &record = m_records[entity.index];
		const std::size_t column = record.location.archetype->ColumnIndex(component);
		if (column == std::numeric_limits<std::size_t>::max())
			return nullptr;
		return static_cast<T *>(record.location.chunk->ComponentData(column)) + record.location.row;
	}

	template<typename T>
	const T *Get(Entity entity) const
	{
		RequireComponentsFinalized();
		const ComponentId component = GetRegisteredComponent<T>();
		if (!IsAlive(entity))
			return nullptr;

		const EntityRecord &record = m_records[entity.index];
		const std::size_t column = record.location.archetype->ColumnIndex(component);
		if (column == std::numeric_limits<std::size_t>::max())
			return nullptr;
		return static_cast<const T *>(record.location.chunk->ComponentData(column)) + record.location.row;
	}

	std::size_t EntityCount() const noexcept { return m_entityCount; }
	std::size_t ArchetypeCount() const noexcept { return m_archetypes.Count(); }
	std::uint64_t ArchetypeRevision() const noexcept { return m_archetypes.Revision(); }
	std::vector<Archetype *> GetArchetypes() { return m_archetypes.GetArchetypes(); }
	std::vector<const Archetype *> GetArchetypes() const { return m_archetypes.GetArchetypes(); }
	const ComponentRegistry &Components() const noexcept { return m_components; }

private:
	struct EntityLocation
	{
		Archetype *archetype{nullptr};
		Chunk *chunk{nullptr};
		std::size_t row{0};
	};

	struct EntityRecord
	{
		EntityGeneration generation{1};
		bool alive{false};
		bool retired{false};
		EntityLocation location{};
	};

	Entity AllocateEntity();
	void ReleaseUnconstructedEntity(Entity entity) noexcept;
	void SetLocation(Entity entity, EntityLocation location) noexcept;
	void RequireComponentsFinalized() const;
	void RequireStructuralMutationAllowed() const;
	void BeginScheduledExecution() noexcept;
	void EndScheduledExecution() noexcept;
	void BeginCommandPlayback() noexcept;
	void EndCommandPlayback() noexcept;

	template<typename T>
	ComponentId GetRegisteredComponent() const
	{
		const ComponentId component = m_components.TryGet<T>();
		if (component == InvalidComponentId)
			throw std::logic_error("ECS component was not registered before finalization");
		return component;
	}

	template<typename T>
	ComponentId GetDefaultConstructibleComponent() const
	{
		const ComponentId component = GetRegisteredComponent<T>();
		if (m_components.Get(component).constructDefault == nullptr)
			throw std::logic_error("ECS component does not support default construction");
		return component;
	}

	bool AddComponent(Entity entity, ComponentId component, void *value);
	bool RemoveComponent(Entity entity, ComponentId component);

	Archetype &GetOrCreateArchetype(const Signature &signature);
	void MoveEntity(Entity entity,
		const Signature &targetSignature,
		ComponentId initializedComponent = InvalidComponentId,
		void *initializedValue = nullptr);

	friend class CommandBuffer;
	friend class Scheduler;
	// Test seam for exercising generation exhaustion without billions of cycles.
	friend struct WorldTestAccess;

	WorldConfig m_config;
	ComponentRegistry m_components;
	ArchetypeRegistry m_archetypes;
	std::vector<EntityRecord> m_records;
	std::vector<EntityIndex> m_freeIndices;
	std::size_t m_entityCount{0};
	bool m_scheduledExecutionActive{false};
	bool m_commandPlaybackActive{false};
};
} // extern "C++"

} // namespace ecs

namespace ecs
{

extern "C++"
{
World::World(WorldConfig config) :
	m_config(config)
{
}

Entity World::AllocateEntity()
{
	if (!m_freeIndices.empty())
	{
		const EntityIndex index = m_freeIndices.back();
		m_freeIndices.pop_back();
		EntityRecord &record = m_records[index];
		assert(!record.retired);
		record.alive = true;
		record.location = EntityLocation{};
		++m_entityCount;
		return Entity{index, record.generation};
	}

	if (m_records.size() >= static_cast<std::size_t>(Entity::InvalidIndex))
		throw std::length_error("ECS entity registry exhausted");

	const EntityIndex index = static_cast<EntityIndex>(m_records.size());
	m_records.push_back(EntityRecord{});
	EntityRecord &record = m_records.back();
	record.alive = true;
	++m_entityCount;
	return Entity{index, record.generation};
}

void World::ReleaseUnconstructedEntity(Entity entity) noexcept
{
	assert(entity.index < m_records.size());
	EntityRecord &record = m_records[entity.index];
	record.alive = false;
	record.location = EntityLocation{};
	assert(m_entityCount > 0);
	--m_entityCount;
	m_freeIndices.push_back(entity.index);
}

void World::SetLocation(Entity entity, EntityLocation location) noexcept
{
	assert(entity.index < m_records.size());
	EntityRecord &record = m_records[entity.index];
	assert(record.alive && record.generation == entity.generation);
	record.location = location;
}

void World::RequireComponentsFinalized() const
{
	if (!m_components.IsFrozen())
		throw std::logic_error("ECS component registry must be finalized before ECS state operations");
}

void World::RequireStructuralMutationAllowed() const
{
	if (m_scheduledExecutionActive && !m_commandPlaybackActive)
		throw std::logic_error("Direct ECS structural mutation is forbidden during scheduled execution; use a command buffer");
}

void World::BeginScheduledExecution() noexcept
{
	assert(!m_scheduledExecutionActive);
	assert(!m_commandPlaybackActive);
	m_scheduledExecutionActive = true;
}

void World::EndScheduledExecution() noexcept
{
	assert(m_scheduledExecutionActive);
	assert(!m_commandPlaybackActive);
	m_scheduledExecutionActive = false;
}

void World::BeginCommandPlayback() noexcept
{
	assert(!m_commandPlaybackActive);
	m_commandPlaybackActive = true;
}

void World::EndCommandPlayback() noexcept
{
	assert(m_commandPlaybackActive);
	m_commandPlaybackActive = false;
}

Archetype &World::GetOrCreateArchetype(const Signature &signature)
{
	RequireComponentsFinalized();
	return m_archetypes.GetOrCreate(signature, m_components, m_config.chunkTargetBytes);
}

bool World::IsAlive(Entity entity) const noexcept
{
	return entity.IsValid() && entity.index < m_records.size() &&
		m_records[entity.index].alive && m_records[entity.index].generation == entity.generation;
}

bool World::Destroy(Entity entity)
{
	RequireStructuralMutationAllowed();
	if (!IsAlive(entity))
		return false;

	EntityRecord &record = m_records[entity.index];
	const EntityLocation location = record.location;
	const Entity moved = location.archetype->RemoveEntity(location.chunk, location.row);
	if (moved.IsValid())
	{
		EntityRecord &movedRecord = m_records[moved.index];
		movedRecord.location = EntityLocation{location.archetype, location.chunk, location.row};
	}

	record.alive = false;
	record.location = EntityLocation{};
	if (record.generation == std::numeric_limits<EntityGeneration>::max())
	{
		// The maximum generation has already been issued. Retiring the index
		// prevents an old handle from becoming valid again after wraparound.
		record.retired = true;
	}
	else
	{
		++record.generation;
		m_freeIndices.push_back(entity.index);
	}
	assert(m_entityCount > 0);
	--m_entityCount;
	return true;
}

bool World::AddComponent(Entity entity, const ComponentId component, void *value)
{
	RequireComponentsFinalized();
	const ComponentInfo *info = m_components.TryGet(component);
	if (component == InvalidComponentId || info == nullptr)
		throw std::logic_error("ECS component ID is not registered");
	if (value == nullptr && info->constructDefault == nullptr)
		throw std::logic_error("ECS component does not support default construction");
	if (!IsAlive(entity))
		return false;

	EntityRecord &record = m_records[entity.index];
	if (record.location.archetype->Has(component))
		return false;

	Signature signature = record.location.archetype->GetSignature();
	signature.push_back(component);
	CanonicalizeSignature(signature);
	if (value == nullptr)
		MoveEntity(entity, signature);
	else
		MoveEntity(entity, signature, component, value);
	return true;
}

bool World::RemoveComponent(const Entity entity, const ComponentId component)
{
	RequireComponentsFinalized();
	if (component == InvalidComponentId || m_components.TryGet(component) == nullptr)
		throw std::logic_error("ECS component ID is not registered");
	if (!IsAlive(entity))
		return false;

	EntityRecord &record = m_records[entity.index];
	if (!record.location.archetype->Has(component))
		return false;

	Signature signature = record.location.archetype->GetSignature();
	signature.erase(std::remove(signature.begin(), signature.end(), component), signature.end());
	MoveEntity(entity, signature);
	return true;
}

void World::MoveEntity(Entity entity,
	const Signature &targetSignature,
	const ComponentId initializedComponent,
	void *initializedValue)
{
	assert(IsAlive(entity));
	assert(initializedValue == nullptr || initializedComponent != InvalidComponentId);
	EntityRecord &record = m_records[entity.index];
	const EntityLocation sourceLocation = record.location;
	Archetype *source = sourceLocation.archetype;
	Archetype &target = GetOrCreateArchetype(targetSignature);
	if (source == &target)
		return;

	// Ensure the source availability list cannot allocate after source values
	// have been moved. This is a cold structural-path preflight.
	source->PrepareForRemoval(sourceLocation.chunk);
	const Archetype::Slot destination = target.ReserveEntity();
	std::size_t constructedNew = 0;
	try
	{
		// New components are constructed before any source component is moved.
		// Their default constructors may throw, while registered component moves
		// are noexcept by contract.
		for (ComponentId component : target.GetSignature())
		{
			if (source->Has(component))
				continue;

			const std::size_t destinationColumn = target.ColumnIndex(component);
			const ComponentInfo &info = m_components.Get(component);
			void *destinationData = destination.chunk->ComponentData(destinationColumn);
			const ChunkLayout::Column &destinationLayout = target.Layout().Columns()[destinationColumn];
			void *destinationElement = static_cast<std::byte *>(destinationData) +
				destination.row * destinationLayout.size;
			if (component == initializedComponent && initializedValue != nullptr)
			{
				info.constructMove(destinationElement, initializedValue);
			}
			else
			{
				if (info.constructDefault == nullptr)
					throw std::logic_error("ECS component does not support default construction");
				info.constructDefault(destinationElement);
			}
			++constructedNew;
		}
	}
	catch (...)
	{
		std::size_t remaining = constructedNew;
		for (ComponentId component : target.GetSignature())
		{
			if (source->Has(component) || remaining == 0)
				continue;

			const std::size_t destinationColumn = target.ColumnIndex(component);
			const ComponentInfo &info = m_components.Get(component);
			const ChunkLayout::Column &destinationLayout = target.Layout().Columns()[destinationColumn];
			void *destinationElement = static_cast<std::byte *>(destination.chunk->ComponentData(destinationColumn)) +
				destination.row * destinationLayout.size;
			info.destroy(destinationElement);
			--remaining;
		}
		target.CancelEntity(destination);
		throw;
	}

	for (ComponentId component : source->GetSignature())
	{
		if (!target.Has(component))
			continue;

		const std::size_t destinationColumn = target.ColumnIndex(component);
		const std::size_t sourceColumn = source->ColumnIndex(component);
		const ComponentInfo &info = m_components.Get(component);
		void *destinationData = destination.chunk->ComponentData(destinationColumn);
		void *sourceData = sourceLocation.chunk->ComponentData(sourceColumn);
		const ChunkLayout::Column &destinationLayout = target.Layout().Columns()[destinationColumn];
		const ChunkLayout::Column &sourceLayout = source->Layout().Columns()[sourceColumn];
		void *destinationElement = static_cast<std::byte *>(destinationData) + destination.row * destinationLayout.size;
		void *sourceElement = static_cast<std::byte *>(sourceData) + sourceLocation.row * sourceLayout.size;
		info.constructMove(destinationElement, sourceElement);
	}

	target.PublishEntity(destination, entity);
	const Entity moved = source->RemoveEntity(sourceLocation.chunk, sourceLocation.row);
	if (moved.IsValid())
	{
		EntityRecord &movedRecord = m_records[moved.index];
		movedRecord.location = EntityLocation{source, sourceLocation.chunk, sourceLocation.row};
	}
	record.location = EntityLocation{&target, destination.chunk, destination.row};
}

} // extern "C++"
} // namespace ecs
