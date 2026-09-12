module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

export module engine.ecs.storage.archetype;

export import engine.ecs.core.component_registry;
export import engine.ecs.storage.chunk;
export import engine.ecs.storage.signature;

export namespace ecs
{

// This cross-module friend has ordinary C++ ownership. Its definition remains
// exported by core.world; a forward declaration must not attach it here.
extern "C++" { class World; }

class Archetype
{
public:
	struct Slot
	{
		Chunk *chunk{nullptr};
		std::size_t row{0};
	};

	Archetype(Signature signature,
		std::vector<const ComponentInfo *> components,
		std::size_t chunkTargetBytes);

	const Signature &GetSignature() const noexcept { return m_signature; }
	const ChunkLayout &Layout() const noexcept { return m_layout; }
	const std::vector<std::unique_ptr<Chunk>> &Chunks() const noexcept { return m_chunks; }

	bool Has(ComponentId component) const noexcept;
	std::size_t ColumnIndex(ComponentId component) const noexcept;
	std::size_t EntityCount() const noexcept { return m_entityCount; }

	Slot AddEntity(Entity entity);
	Entity RemoveEntity(Chunk *chunk, std::size_t row);

private:
	Slot ReserveEntity();
	void PublishEntity(Slot slot, Entity entity) noexcept;
	void CancelEntity(Slot slot) noexcept;
	void PrepareForRemoval(Chunk *chunk);

	Signature m_signature;
	std::vector<const ComponentInfo *> m_components;
	ChunkLayout m_layout;
	std::vector<std::unique_ptr<Chunk>> m_chunks;
	std::vector<Chunk *> m_availableChunks;
	std::size_t m_entityCount{0};

	friend class World;
};

class ArchetypeRegistry
{
public:
	Archetype &GetOrCreate(const Signature &signature,
		const ComponentRegistry &components,
		std::size_t chunkTargetBytes,
		bool *created = nullptr);

	std::vector<Archetype *> GetArchetypes();
	std::vector<const Archetype *> GetArchetypes() const;
	std::size_t Count() const noexcept { return m_archetypes.size(); }
	std::uint64_t Revision() const noexcept { return m_revision; }

private:
	std::map<Signature, std::unique_ptr<Archetype>, SignatureLess> m_archetypes;
	std::uint64_t m_revision{0};
};

} // namespace ecs

namespace ecs
{

Archetype::Archetype(Signature signature,
	std::vector<const ComponentInfo *> components,
	std::size_t chunkTargetBytes) :
	m_signature(std::move(signature)),
	m_components(std::move(components)),
	m_layout(ChunkLayout::Build(m_components, chunkTargetBytes))
{
}

bool Archetype::Has(ComponentId component) const noexcept
{
	return std::binary_search(m_signature.begin(), m_signature.end(), component);
}

std::size_t Archetype::ColumnIndex(ComponentId component) const noexcept
{
	const auto found = std::lower_bound(m_signature.begin(), m_signature.end(), component);
	if (found == m_signature.end() || *found != component)
		return std::numeric_limits<std::size_t>::max();
	return static_cast<std::size_t>(found - m_signature.begin());
}

Archetype::Slot Archetype::ReserveEntity()
{
	Chunk *target = nullptr;
	if (!m_availableChunks.empty())
	{
		target = m_availableChunks.back();
	}
	else
	{
		m_chunks.push_back(std::make_unique<Chunk>(ChunkLayout(m_layout), m_components));
		target = m_chunks.back().get();
		try
		{
			m_availableChunks.push_back(target);
		}
		catch (...)
		{
			m_chunks.pop_back();
			throw;
		}
	}

	assert(target != nullptr && !target->IsFull());
	return Slot{target, target->ReserveRow()};
}

void Archetype::PublishEntity(const Slot slot, const Entity entity) noexcept
{
	assert(slot.chunk != nullptr);
	slot.chunk->PublishRow(entity, slot.row);
	++m_entityCount;
	if (slot.chunk->IsFull())
	{
		assert(!m_availableChunks.empty() && m_availableChunks.back() == slot.chunk);
		m_availableChunks.pop_back();
	}
}

void Archetype::CancelEntity(const Slot slot) noexcept
{
	assert(slot.chunk != nullptr);
	slot.chunk->CancelRow(slot.row);
}

void Archetype::PrepareForRemoval(Chunk *chunk)
{
	assert(chunk != nullptr);
	assert(!chunk->IsFull() || std::find(m_availableChunks.begin(), m_availableChunks.end(), chunk) == m_availableChunks.end());
	assert(std::find_if(m_chunks.begin(), m_chunks.end(), [chunk](const std::unique_ptr<Chunk> &candidate) {
		return candidate.get() == chunk;
	}) != m_chunks.end());

	if (chunk->IsFull())
		m_availableChunks.reserve(m_availableChunks.size() + 1);
}

Archetype::Slot Archetype::AddEntity(const Entity entity)
{
	const Slot slot = ReserveEntity();
	std::size_t constructed = 0;
	try
	{
		for (const ComponentInfo *info : m_components)
		{
			if (info->constructDefault == nullptr)
				throw std::logic_error("ECS component does not support default construction");
			const std::size_t columnIndex = ColumnIndex(info->id);
			const ChunkLayout::Column &column = m_layout.Columns()[columnIndex];
			info->constructDefault(static_cast<std::byte *>(slot.chunk->ComponentData(columnIndex)) +
				slot.row * column.size);
			++constructed;
		}
	}
	catch (...)
	{
		for (std::size_t index = 0; index < constructed; ++index)
		{
			const ComponentInfo *info = m_components[index];
			const std::size_t columnIndex = ColumnIndex(info->id);
			const ChunkLayout::Column &column = m_layout.Columns()[columnIndex];
			info->destroy(static_cast<std::byte *>(slot.chunk->ComponentData(columnIndex)) +
				slot.row * column.size);
		}
		CancelEntity(slot);
		throw;
	}

	PublishEntity(slot, entity);
	return slot;
}

Entity Archetype::RemoveEntity(Chunk *chunk, std::size_t row)
{
	assert(chunk != nullptr);
	assert(!chunk->IsFull() || std::find(m_availableChunks.begin(), m_availableChunks.end(), chunk) == m_availableChunks.end());
	assert(std::find_if(m_chunks.begin(), m_chunks.end(), [chunk](const std::unique_ptr<Chunk> &candidate) {
		return candidate.get() == chunk;
	}) != m_chunks.end());

	const bool wasFull = chunk->IsFull();
	PrepareForRemoval(chunk);
	const Entity moved = chunk->RemoveSwap(row);
	assert(m_entityCount > 0);
	--m_entityCount;
	if (wasFull)
		m_availableChunks.push_back(chunk);
	return moved;
}

Archetype &ArchetypeRegistry::GetOrCreate(const Signature &signature,
	const ComponentRegistry &components,
	std::size_t chunkTargetBytes,
	bool *created)
{
	if (!components.IsFrozen())
		throw std::logic_error("ECS component registry must be finalized before creating archetypes");

	Signature canonicalSignature = signature;
	CanonicalizeSignature(canonicalSignature);

	const auto found = m_archetypes.find(canonicalSignature);
	if (found != m_archetypes.end())
	{
		if (created != nullptr)
			*created = false;
		return *found->second;
	}

	std::vector<const ComponentInfo *> infos;
	infos.reserve(canonicalSignature.size());
	for (ComponentId component : canonicalSignature)
		infos.push_back(&components.Get(component));

	auto archetype = std::make_unique<Archetype>(std::move(canonicalSignature), std::move(infos), chunkTargetBytes);
	Archetype *result = archetype.get();
	m_archetypes.emplace(result->GetSignature(), std::move(archetype));
	++m_revision;
	if (created != nullptr)
		*created = true;
	return *result;
}

std::vector<Archetype *> ArchetypeRegistry::GetArchetypes()
{
	std::vector<Archetype *> result;
	result.reserve(m_archetypes.size());
	for (const auto &entry : m_archetypes)
		result.push_back(entry.second.get());
	return result;
}

std::vector<const Archetype *> ArchetypeRegistry::GetArchetypes() const
{
	std::vector<const Archetype *> result;
	result.reserve(m_archetypes.size());
	for (const auto &entry : m_archetypes)
		result.push_back(entry.second.get());
	return result;
}

} // namespace ecs
